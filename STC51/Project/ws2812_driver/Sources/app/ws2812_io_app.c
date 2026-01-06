#include "ws2812_io_app.h"
#include "ws2812_io_hal.h"
#include "ai8051u.h"
#include "config.h"

//turn off all leds
void clear_leds() {
    unsigned char i = 0;
			
    for(i = 0;i < 64;i ++) {
			
        show_led(0, 0, 0);

		delay_ms(5);
    };
	
	send_reset();
}

//led lights one by one
void led_onebyone() {
	
    unsigned char i = 0;
    unsigned char j = 0;
	
	for(i = 0;i < 64;i ++){
		
		for(j = 0;j < i;j ++) {
			
			show_led(0, 0, 32);
		}
		
		send_reset();
		
	}
}


