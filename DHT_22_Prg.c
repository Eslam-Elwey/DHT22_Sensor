
#include "StdTypes.h"
#include "DIO_Int.h"
#include "DIO_Priv.h"
#include "Timers_Int.h"
#include "LCD_Int.h"
#include "DHT_22_Int.h"

#define F_CPU		8000000UL

#include <util/delay.h>

/****************************************************************************/

static volatile u8 data_handle_arr[5] ; 
static u16 temp_val ;
static u16 humdity_val ;
static volatile u8 calc_crc ;
static u8 rec_crc ;
static volatile  u8 bit_val = 0 ;
static u8 complete_flag = 0 ;
static u8 flag = 0 ;

/****************************************************************************/

static void ICU_Func (void) ;

static void DHT22_Receive_data (void) ;

static void DHT22_Send_Request (void) ;

/****************************************************************************/


static void DHT22_Send_Request (void)
{
	DIO_InitPin(DHT22_PIN,OUTPUT);
	DIO_WritePin(DHT22_PIN,LOW);
	
	_delay_ms(TIME_SELECT_LOW_MS);
	DIO_InitPin(DHT22_PIN,INFREE);
	while (!DIO_ReadPin(DHT22_PIN));
	_delay_us(TIME_WAIT_FOR_RESPONSE_US);
	
	TIMER1_WRITE_VALUE(0) ;
	
	Timer1_ICU_Edge_Mode(ICES1_FALLING) ;
	
	TIMER1_ICU_INTERRUPT_ENABLE() ;
}

/****************************************************************************/

void DHT22_Init (void) 
{
	DHT22_Send_Request() ;
	
	Timer1_Init(TIMER1_NORMAL_MODE,TIMER1_PRESCALER_CLK_8);
	
	Timer1_Set_ICU_Call_Back(&ICU_Func) ;
}

/****************************************************************************/

void DHT22_send_Runnable (void)
{
	if (complete_flag==1)
	{
		complete_flag = 0 ;
		LCD_Write_String_Cursor(1,1,(u8*)"Temp:");
		LCD_Write_Number(temp_val/10);
		LCD_Write_Character('.');
		LCD_Write_Number(temp_val%10);
		LCD_Write_String((u8*)"    ");
		
		LCD_Write_String_Cursor(2,1,(u8*)"HUM:");
		LCD_Write_Number(humdity_val/10);
		LCD_Write_Character('.');
		LCD_Write_Number(humdity_val%10);
		LCD_Write_String((u8*)"    ");
	}
	

}

/****************************************************************************/



static void DHT22_Receive_data (void) 
{
	static u8 current_elemrnt = 0 ;
	static s8 current_bit = 7 ;
	
	if (0==bit_val)
	{
		CLR_BIT(data_handle_arr[current_elemrnt],current_bit);
	}
	else if (1==bit_val)
	{
		SET_BIT(data_handle_arr[current_elemrnt],current_bit);
	}
	
	current_bit-- ;
	
	if (current_bit<0)
	{
		current_elemrnt++ ;
		current_bit = 7 ;
	}
	
	if ( 5 == current_elemrnt)
	{
		complete_flag = 1 ;
		TIMER1_ICU_INTERRUPT_DISABLE() ;
	
		flag = 0 ;
		
		rec_crc = data_handle_arr[4] ;
		complete_flag = 1 ;
		current_elemrnt = 0 ;
		current_bit = 7 ;
		
		humdity_val = data_handle_arr[1] + (u16)data_handle_arr[0] * 10 + (u16)data_handle_arr[0] *256- (u16)data_handle_arr[0] *10 ;
		temp_val = data_handle_arr[3]  + (u16)data_handle_arr[2] * 10 + (u16)data_handle_arr[2] * 256- (u16)data_handle_arr[2] *10 ;
		
	
		calc_crc = humdity_val + temp_val ; 
		
	
	

		calc_crc = 0 ;
		DHT22_Send_Request() ;
	}
}


/****************************************************************************/

static void ICU_Func (void) 
{
	static u16 t1 ;
	static u16 t2;
	static u16 t3;
	u8 duration1 ;
	
	u8 duration2 ;
	
	if (0==flag)
	{
		flag = 1 ;
		
		TIMER1_ICU_READ_VALUE(&t1) ;
		
		Timer1_ICU_Edge_Mode(ICES1_RISING) ;
	}
	
	else if (1==flag)
	{
		TIMER1_ICU_READ_VALUE(&t2) ;
		
		Timer1_ICU_Edge_Mode(ICES1_FALLING) ;
		
		/* 50 us */
		duration1 = t2 - t1 ;
		flag = 2 ;
		
	}
	else if (2==flag)
	{
		TIMER1_ICU_READ_VALUE(&t3) ;	
		
		Timer1_ICU_Edge_Mode(ICES1_RISING) ;
		
		/* 70 - 30 us */
		duration2 = t3 -t2 ;
		/* 70 us */
		if (duration2>=(TIME_DELAY_HIGH_US-3))
		{
			bit_val = 1 ;
		}
		/* 30 us */
		else if (duration2>=(TIME_DELAY_LOW_US-3))
		{
			bit_val = 0 ;
		}
	
		flag = 1 ;
		t1 = t3 ;
		DHT22_Receive_data() ;
		
	}

}

/****************************************************************************/