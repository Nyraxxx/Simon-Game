#include "timer.h"
#include "spi.h"
#include "uart.h"

extern uint8_t digit1, digit2;
extern uint8_t segs [];


// Compare values for tone (pre calculated)
#define cmpp1 = 5482
#define cmpp2 = 6511
#define cmpp3 = 4105
#define cmpp4 = 109645

void pb_debounce(void);

void timer_init(void) {
    cli();

    TCB0.CTRLB = TCB_CNTMODE_INT_gc;    // Configure TCB0 in periodic interrupt mode
    TCB0.CCMP = 33333;                   // Set interval for 10ms (33333 clocks @ 3.3 MHz)
    TCB0.INTCTRL = TCB_CAPT_bm;         // CAPT interrupt enable
    TCB0.CTRLA = TCB_ENABLE_bm;         // Enable
    sei(); 
}

void get_time(void){
   uint16_t time = TCB0.CNT;
   char *string = "hello";
    
}

//ISR for debounce and writing to spi
ISR(TCB0_INT_vect) {
    pb_debounce();

    static uint8_t digit = 0;    

    if (digit) {
       spi_write(segs[0] | (0x01 << 7));
    } else {
        spi_write(segs[1]);
    }
    digit = !digit;

    TCB0.INTFLAGS = TCB_CAPT_bm;
}

