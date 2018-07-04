/**
 * Project: USBflashTrigger
 * Author: Christopher Hofmann, christopherushofmann@googlemail.com
 * Based on V-USB example code by Christian Starkjohann
 * Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
 * License: GNU GPL v3 (see License.txt)
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#ifdef STORE_FLASHTIME_EEPROM
	#include <avr/eeprom.h>
#endif
#include <util/delay.h>

#include "usbdrv.h"

#include "../common/defines.h"







#define PORTOF(name)           CONCAT2(PORT, name)
#define PINOF(port, name)      CONCAT3(P, port, name)
#define DDROF(name)            CONCAT2(DDR, name)


#define CONCAT3(p1, p2, p3)     p1 ## p2 ## p3
#define CONCAT2(p1, p2)         p1 ## p2

#define TRIGGERPORT PORTOF(PORT_TRIGGER)
#define TRIGGERDDR	DDROF(PORT_TRIGGER)
#define TRIGGERPIN 	PINOF(PORT_TRIGGER, PIN_TRIGGER)
#define FLASHPORT 	PORTOF(PORT_FLASH)
#define FLASHDDR 	DDROF(PORT_FLASH)
#define FLASHPIN 	PINOF(PORT_FLASH, PIN_FLASH)


#ifdef TRIGGER_ACTIVE_IS_LOW
	#define SET_TRIGGER TRIGGERPORT &= ~(1 << TRIGGERPIN);
	#define STOP_TRIGGER  TRIGGERPORT |= (1 << TRIGGERPIN);
	#define TRIGGER_STATE ((TRIGGERPORT & (1 << TRIGGERPIN)) ^ 0x01)
#else
	#define SET_TRIGGER  TRIGGERPORT |= (1 << TRIGGERPIN);
	#define STOP_TRIGGER TRIGGERPORT &= ~(1 << TRIGGERPIN);
	#define TRIGGER_STATE TRIGGERPORT & (1 << TRIGGERPIN)
#endif

#ifdef FLASH_ACTIVE_IS_LOW
	#define STOP_FLASH 	 FLASHPORT	 |= (1 << FLASHPIN);
	#define SET_FLASH 	 FLASHPORT 	 &= ~(1 << FLASHPIN);
	#define FLASH_STATE (FLASHPORT & (1 << FLASHPIN)) ^ 0x01
#else
	#define SET_FLASH 	 FLASHPORT	 |= (1 << FLASHPIN);
	#define STOP_FLASH 	 FLASHPORT 	 &= ~(1 << FLASHPIN);
	#define FLASH_STATE  FLASHPORT & (1 << FLASHPIN)
#endif




// Timer is enable by setting the prescaler to 8, giving it 1.5 MHz
#define ENABLE_TIMER TCCR1B |=(0 << CS12) | (1 << CS11) | (0 << CS10);
// likewise it is disabled by setting the prescaler to 0
#define STOP_TIMER	TCCR1B &= ~(0x07);





uint8_t lightIsOn = 0;
uint16_t flashTime = 500;
uint16_t flashTimeLeft;


usbMsgLen_t usbFunctionSetup(uint8_t data[8]) {
	usbRequest_t *rq = (void *)data;
	static uchar buffer[2];
	
	
	switch(rq->bRequest) {

		case FT_CMD_TRIGGER:
			SET_TRIGGER
			return 0; 

		case FT_CMD_FLASH_AND_TRIGGER:
			flashTimeLeft = flashTime;
			SET_FLASH
			SET_TRIGGER
			ENABLE_TIMER;
			return 0;

		case FT_CMD_LIGHT_ON:
			SET_FLASH;
			// ledRedOn();
			return 0;

		case FT_CMD_LIGHT_OFF:
			STOP_FLASH;
			// ledRedOff();
			return 0;

		case FT_CMD_LIGHT_STATE:
			
			buffer[0] = (FLASHPORT & (1 << FLASHPIN)) >> FLASHPIN;
			#ifdef FLASH_ACTIVE_IS_LOW
				// flip last bit if low is active
				buffer[0] ^= 0x01;
			#endif
    		usbMsgPtr = buffer;
    		return 1;

    	case FT_CMD_FLASH_TIME_SET:
    		flashTime = (rq->wValue.bytes[1] << 8);
    		flashTime |= rq->wValue.bytes[0];
    		return 0;

    	case FT_CMD_FLASH_TIME_GET:
    		
    		buffer[0] = (uchar)(flashTime >> 8);
    		buffer[1] = (uchar)(flashTime & 0xFF);

    		usbMsgPtr = buffer;
    		return 2;


	}

	
	return 0; // by default don't return any data
}




int main() {
	uint8_t i, j;

	/* no pullups on USB pins */
	PORTD = 0;
	PORTB = 0;

	/* PD2 = INT0  must be input: for usb interrupts */
	DDRD = ~(1 << PD2);

	/* output SE0 for USB reset */
	DDRB = ~0;
	j = 0;
	/* USB Reset by device only required on Watchdog Reset */
	while (--j) {
		i = 0;
		/* delay >10ms for USB reset */
		while (--i)
			;
	}
	/* all USB and ISP pins inputs */
	// /* PB3 and PB5 are outputs */
	DDRB = 0;
	TRIGGERDDR |= (1 << TRIGGERPIN); 
	FLASHDDR |= (1 << FLASHPIN);

	STOP_FLASH;
	STOP_TRIGGER;

	/* all inputs except PC0, PC1, PC2*/
	DDRC = 0x07;
	PORTC = 0;


	/* init usb  */
	usbInit();

	/* force reenumeration */
	usbDeviceDisconnect();
	for(i=0; i<250; i++){
		// wdt_reset();	// keep watchdog happy
		_delay_ms(2);
	}
	usbDeviceConnect();

	/* allow interrupts */
	sei();




	/* Init timer */
	TCCR1A  = 0; // no pwm and no output pin
	TIMSK  |= (1 << OCIE1A); // compare enable enable
	TCCR1B |= (1 << WGM12); // CTC Mode
	OCR1A = 1499; //compare value for 1 ms resolution with 1.5 MHz clock

	
	for (;;) {
		//  check for new usb events
		usbPoll();

		/* if trigger is set, then clear it after 30ms */
		if (TRIGGER_STATE)
		{
			_delay_ms(30);
			STOP_TRIGGER;
		}



	}
	return 0;
}



ISR (TIMER1_COMPA_vect)
{
	/* Interrupt happens every 1ms */

	flashTimeLeft--;
	if (flashTimeLeft == 0)
	{
		STOP_FLASH;
		STOP_TIMER;
	}

}
