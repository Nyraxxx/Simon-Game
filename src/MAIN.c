#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include "spi.h"
#include "buzzer.h"
#include "uart(1).h"
#include "serial.h"

// Definitions for segments to light up
#define SEGS_EF 0b00111110
#define SEGS_BC 0b01101011
#define SEGS_OFF 0b01111111

// button pin bitmaps
#define PB1 PIN4_bm
#define PB2 PIN5_bm
#define PB3 PIN6_bm
#define PB4 PIN7_bm

// Compare values for tone (pre calculated)
#define cmpp1 = 5482
#define cmpp2 = 6511
#define cmpp3 = 4105
#define cmpp4 = 109645

// array for triggering the correct segments
volatile uint8_t segs[] = {SEGS_OFF, SEGS_OFF}; // segs initially off

// debouncing for what???
// inital debouncing variables, initially 0 as no debouncing has occured
volatile uint8_t pb_debounced = 0xFF;
uint8_t pb_previous_state = 0xFF;
uint8_t pb_new_state = 0xFF;

int sequenceSTEP()
{
    static uint8_t MASK = 0xE2023CAB;
    volatile uint8_t STATE_LFSR = 0x11079606;
    // get LSB
    volatile uint8_t BIT = STATE_LFSR | 0x00000001;
    STATE_LFSR = STATE_LFSR >> 1;
    if (BIT == 1)
    {
        STATE_LFSR = STATE_LFSR ^ MASK;
    }
    volatile uint8_t STEP = STATE_LFSR & 0b11;
    spi_write(STEP);
    return STEP;
}
volatile uint16_t sequence_store[51];
volatile uint16_t player_input[51];
int sequence_GEN(void)
{
    uint16_t i;

    for (i = 0; i <= 51; i++)
    {
        sequence_store[i] = sequenceSTEP();
        spi_write(sequence_store[i]);
    }
}

void pb_init(void)
{
    // already configured as inputs
    // PBs PA4-7, enable pullup resistors
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
}

// debouncing function for pushbuttons
void pb_debounce(void)
{

    static uint8_t count0 = 0;
    static uint8_t count1 = 0;

    // check if button has been pushed
    uint8_t pb_edge = pb_debounced ^ PORTA.IN;

    // Vertical counter

    count1 = (count1 ^ count0) & pb_edge;

    count0 = ~count0 & pb_edge;

    // New debounced state

    // Update if PB high for three samples

    pb_debounced ^= (count1 & count0);
}

typedef enum
{
    TONE1,
    TONE2,
    TONE3,
    TONE4,
    WAIT

} tone_state;


 //Write your code for Ex 12.0 above this lin
void sequence_play(uint8_t num)
{

    switch (num)
    {
        // turn off

    case 0:
        // buzzer off initially
        //  delay(1);
        // need delays!! playback bullshit ISR?
        // current time = curtime + 1000;

        segs[0] = SEGS_OFF;
        segs[1] = SEGS_OFF;
        TCA0.SINGLE.CMP0BUF = 0;

        break;
    // tone 1
    // play tone, then go into off sequence with timer (?)
    case 1:
        num = 0;

        segs[0] = SEGS_EF;
        segs[1] = SEGS_OFF;
        TCA0.SINGLE.PERBUF = TONE1_PER;
        TCA0.SINGLE.CMP0BUF = TONE1_PER >> 1;

        break;
    // tone 2
    case 2:
        num = 0;

        segs[0] = SEGS_BC;
        segs[1] = SEGS_OFF;
        TCA0.SINGLE.PERBUF = TONE2_PER;
        TCA0.SINGLE.CMP0BUF = TONE2_PER >> 1;
        break;
    // tone 3
    case 3:
        num = 0;

        segs[1] = SEGS_EF;
        segs[0] = SEGS_OFF;
        TCA0.SINGLE.PERBUF = TONE3_PER;
        TCA0.SINGLE.CMP0BUF = TONE3_PER >> 1;
        break;
    // tone 4
    case 4:
        num = 0;

        segs[1] = SEGS_BC;
        segs[0] = SEGS_OFF;
        TCA0.SINGLE.PERBUF = TONE4_PER;
        TCA0.SINGLE.CMP0BUF = TONE4_PER >> 1;
        break;
    }
}

void fakef(char* string) {

    char* point = string;
    while (*point != NULL) {
        putchar(*point);
        point++;
    }
}

int main(void)
{
    cli();                                                      // Disable interrupts
    CCP = CCP_IOREG_gc;                                         // Configuration change enable			            
   //CLKCTRL.MCLKCTRLB = CLKCTRL_PDIV_2X_gc | CLKCTRL_PEN_bm;    // Set clock to 10 MHz (x2 prescaler enabled)
    serial_init();                                              // Init serial & stdio
    sei();         
    // calling initialisation functions
   pb_init();
   timer_init();
   spi_init();
   buzzer_init();
   sequence_GEN();
   get_time();
   

    // initial state call
tone_state state = WAIT;

    // initial sequence length
volatile uint16_t seq_len = 0;

    // main loop

    
  
    while (1)
    {
        fakef("Hello\n");
        sprintf();
        
        //  printf("hello");
        // determine pushbutton state

        pb_previous_state = pb_new_state;
        pb_new_state = pb_debounced;

        // find falling edge
        uint8_t pb_falling_edge = (pb_previous_state ^ pb_new_state) & pb_previous_state;
        // find rising edge
        uint8_t pb_rising_edge = (pb_previous_state ^ pb_new_state) & pb_new_state;

        // SIMON LOGIC (????)

       // start
        //show sequence
       // wait for input
      
        uint8_t current_step = 0;
        int j;
        for (j = 0; j <= current_step; j++)
        {
            // play tone/display current step in sequence
            sequence_play(sequence_store[j]);
        }
        int k;
        for (k = 0; k <= current_step; k++)
        {
            if (sequence_store[k] = player_input[k])
            {
                // score + 1
                // win
            }
            else
            {
                //current_step = 0;
                // lose
                // state = high score
            }
        }
        // state machine START for tone and light up
        // edit this so it only goes for playback time not until button unpressed

        // player input???
        switch (state)
        {
        case WAIT:
            // turn off
            TCA0.SINGLE.CMP0BUF = 0;

            if (pb_falling_edge & PB1)
            {
                state = TONE1;
                segs[0] = SEGS_EF;
                segs[1] = SEGS_OFF;
                TCA0.SINGLE.PERBUF = TONE1_PER;
                TCA0.SINGLE.CMP0BUF = TONE1_PER >> 1;
                player_input[current_step] = 1;
                current_step += 1;
            }
            else if (pb_falling_edge & PB2)
            {
                state = TONE2;
                segs[0] = SEGS_BC;
                segs[1] = SEGS_OFF;
                TCA0.SINGLE.PERBUF = TONE2_PER;
                TCA0.SINGLE.CMP0BUF = TONE2_PER >> 1;
                player_input[current_step] = 2;
                current_step += 1;
            }
            else if (pb_falling_edge & PB3)
            {
                state = TONE3;
                segs[1] = SEGS_EF;
                segs[0] = SEGS_OFF;
                TCA0.SINGLE.PERBUF = TONE3_PER;
                TCA0.SINGLE.CMP0BUF = TONE3_PER >> 1;
                player_input[current_step] = 3;
                current_step += 1;
            }
            else if (pb_falling_edge & PB4)
            {
                state = TONE4;
                segs[1] = SEGS_BC;
                segs[0] = SEGS_OFF;
                TCA0.SINGLE.PERBUF = TONE4_PER;
                TCA0.SINGLE.CMP0BUF = TONE4_PER >> 1;
                player_input[current_step] = 4;
                current_step += 1;
            }

            break;
        case TONE1:
            if (pb_rising_edge & PB1)
            {
                state = WAIT;
                segs[0] = SEGS_OFF;
                segs[1] = SEGS_OFF;
                TCA0.SINGLE.CMP0BUF = 0;
            }

            break;
        case TONE2:
            if (pb_rising_edge & PB2)
            {
                state = WAIT;
                segs[0] = SEGS_OFF;
                segs[1] = SEGS_OFF;
                TCA0.SINGLE.CMP0BUF = 0;
            }

            break;
        case TONE3:
            if (pb_rising_edge & PB3)
            {
                state = WAIT;
                segs[0] = SEGS_OFF;
                segs[1] = SEGS_OFF;
                TCA0.SINGLE.CMP0BUF = 0;
            }

            break;
        case TONE4:
            if (pb_rising_edge & PB4)
            {
                state = WAIT;
                segs[0] = SEGS_OFF;
                segs[1] = SEGS_OFF;
                TCA0.SINGLE.CMP0BUF = 0;
            }
            break;
        default:
            WAIT;
            break;
        }
    }
}
