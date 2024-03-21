
#ifndef DHT_22_INT_H_
#define DHT_22_INT_H_


#define TIME_SELECT_LOW_MS													18
#define TIME_WAIT_FOR_RESPONSE_US											80
#define TIME_SLOT_US														50
#define TIME_DELAY_HIGH_US													70						
#define TIME_DELAY_LOW_US													30	

#define DHT22_PIN										PIND6



void DHT22_Init (void) ;


void DHT22_send_Runnable (void) ;

#endif /* DHT_22_INT_H_ */