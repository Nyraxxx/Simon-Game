#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdbool.h>
#include "timer.h"
#include "spi.h"
#include "hardware.h"
#include "uart.h"
#include "serial.h"

//--------------------------------------------------------------------------------------------------------------------
// Definitions for segments to light up
#define SEGS_EF 0b00111110
#define SEGS_BC 0b01101011
#define SEGS_OFF 0b01111111

// array for triggering the correct segments
volatile uint8_t segs[] = {SEGS_OFF, SEGS_OFF}; // segs initially off
//--------------------------------------------------------------------------------------------------------------------
// button pin bitmaps
#define PB1 PIN4_bm
#define PB2 PIN5_bm
#define PB3 PIN6_bm
#define PB4 PIN7_bm

// inital debouncing variables, initially 0 as no debouncing has occured
volatile uint8_t pb_debounced = 0xFF;
uint8_t pb_previous_state = 0xFF;
uint8_t pb_new_state = 0xFF;

void pb_init(void)
{
    // already configured as inputs
    // PBs PA4-7, enable pullup resistors
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
}

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
//--------------------------------------------------------------------------------------------------------------------
typedef enum
{
    WAIT, //(??)
    PLAY_SEQUENCE,
    PLAYER_INPUT,
    GAME_OVER,
    GAME_WIN,
} game_state;
//--------------------------------------------------------------------------------------------------------------------
typedef enum
{
    TONE1,
    TONE2,
    TONE3,
    TONE4,
    WAIT_T,
    COMPARE

} tone_state;
//--------------------------------------------------------------------------------------------------------------------
volatile uint32_t STATE_LFSR = 0x11079606;
uint8_t sequence_STEP()
{
    static uint32_t MASK = 0xE2023CAB;

    // get LSB
    volatile uint32_t BIT = STATE_LFSR & 0x00000001;
    STATE_LFSR = STATE_LFSR >> 1;
    if (BIT == 1)
    {
        STATE_LFSR = STATE_LFSR ^ MASK;
    }
    volatile uint8_t STEP = STATE_LFSR & 0b11;
    // printf("step %0d\n", STEP);
    return STEP;
}
//--------------------------------------------------------------------------------------------------------------------
volatile uint16_t sequence_store[51];
volatile uint16_t player_input[51];
uint8_t current_input = 0;
void sequence_GEN(void)
{
    uint16_t i;
    printf("SEQUENCE MASTER: \n");
    for (i = 0; i <= 51; i++)
    {
        sequence_store[i] = sequence_STEP();

        // printf(sequence_store[i]);
        uint8_t seqq = sequence_store[i];
        printf(" %d", seqq);
    }
}
//--------------------------------------------------------------------------------------------------------------------
void sequence_play(uint8_t num, uint8_t playback)
{
    // this doesnt live update hmmm
    int arb;
    uint8_t k = 0;
    uint8_t toggle = 1;

    switch (toggle)
    {

    case 1:
        if (num == 0)
        {

            segs[0] = SEGS_EF;
            segs[1] = SEGS_OFF;
            TCA0.SINGLE.PERBUF = TONE1_PER;
            TCA0.SINGLE.CMP0BUF = TONE1_PER >> 1;
            toggle = 0;
        }
        else if (num == 1)
        {

            segs[0] = SEGS_BC;
            segs[1] = SEGS_OFF;
            TCA0.SINGLE.PERBUF = TONE2_PER;
            TCA0.SINGLE.CMP0BUF = TONE2_PER >> 1;
            toggle = 0;
        }
        else if (num == 2)
        {

            segs[1] = SEGS_EF;
            segs[0] = SEGS_OFF;
            TCA0.SINGLE.PERBUF = TONE3_PER;
            TCA0.SINGLE.CMP0BUF = TONE3_PER >> 1;
            toggle = 0;
        }
        else if (num == 3)
        {

            segs[1] = SEGS_BC;
            segs[0] = SEGS_OFF;
            TCA0.SINGLE.PERBUF = TONE4_PER;
            TCA0.SINGLE.CMP0BUF = TONE4_PER >> 1;
            toggle = 0;
        }

    case 0:
        // printf("CASE 0");

        for (k = 0; k <= playback; k++)
        {
            arb = arb + 1;

            printf("k%d", k);
        }
        // printf("OFF");

        segs[0] = SEGS_OFF;
        segs[1] = SEGS_OFF;
        TCA0.SINGLE.CMP0BUF = 0;
        printf("\n");
        break;
    }
}

int main()
{
    // initialise functions

    spi_init();
    serial_init();
    buzzer_init();
    timer_init();
    pot_init();
    pb_init();
    sequence_GEN();
    //--------------------------------------------------------------------------------------------------------------------
    game_state game_state = PLAY_SEQUENCE;
    tone_state state = WAIT_T;
    uint8_t current_index = 0;
    uint8_t success = 0;

    //--------------------------------------------------------------------------------------------------------------------
    while (1)
    {
        uint8_t testt = 255 -  ADC0.RESULT;
     

        uint8_t plybk = MAP(testt, 0, 255, 25, 200);
        switch (game_state)
        {
        case WAIT:
            // printf("WAIT\n");
            break;
        case PLAY_SEQUENCE:
            // plybk = ADC0.RESULT;
            //  loop for how many steps we are up to
            for (int j = 0; j <= current_index; j++)
            {

                // play tone/display current step in sequence
                sequence_play(sequence_store[j], plybk);
                // printf("sequence store val @ index %d\n", sequence_store[j]);
                // printf("index of for loop %d\n", j);
            }
            // printf("current highest step %d\n", current_index);
            printf("END OF PLAY SEQ\n");
            // move up to the next step
            // current_index++;
            // move to player input so player can try input sequence
            current_input = NULL;
            game_state = PLAYER_INPUT;

            break;

        case PLAYER_INPUT:

            // get player input

            pb_previous_state = pb_new_state;
            pb_new_state = pb_debounced;

            // find falling edge
            uint8_t pb_falling_edge = (pb_previous_state ^ pb_new_state) & pb_previous_state;
            // find rising edge
            uint8_t pb_rising_edge = (pb_previous_state ^ pb_new_state) & pb_new_state;
            switch (state)
            {
            case COMPARE:
                // sequence_store[current_index]
                //  after button pressed, compare it to master array at relevant pos
                success = 1;
                // i loops what tone in sequence yes
                for (int i = 0; i <= current_index; i++)
                {
                    printf("i = %d\n", i);
                    if (sequence_store[i] != current_input)
                    {
                        success = 0;
                        break;
                    }
                    // if correct

                }
                if (success == 1)
                {
                    current_index++;
                    current_input = NULL;
                    state = WAIT_T;
                    game_state = PLAY_SEQUENCE;
                }

                break;
            case WAIT_T:
                // printf("wait\n");

                // turn off everything
                TCA0.SINGLE.CMP0BUF = 0;
                segs[0] = SEGS_OFF;
                segs[1] = SEGS_OFF;

                // button 1
                if (pb_falling_edge & PB1)
                {
                    // set state to turn off tone when button releases
                    state = TONE1;
                    // turn on correct screen display
                    segs[0] = SEGS_EF;
                    segs[1] = SEGS_OFF;
                    // start playing tone on buzzer
                    TCA0.SINGLE.PERBUF = TONE1_PER;
                    TCA0.SINGLE.CMP0BUF = TONE1_PER >> 1;
                    // record player input in array
                    current_input = 0;

                    printf("tone1\n");
                    // printf("step %d\n", current_step);
                    // printf("input %d \n", player_input[current_step]);
                    // current_step = current_step + 1;
                }
                else if (pb_falling_edge & PB2)
                {
                    state = TONE2;
                    segs[0] = SEGS_BC;
                    segs[1] = SEGS_OFF;
                    TCA0.SINGLE.PERBUF = TONE2_PER;
                    TCA0.SINGLE.CMP0BUF = TONE2_PER >> 1;
                    current_input = 1;
                    printf("tone2\n");
                }
                else if (pb_falling_edge & PB3)
                {
                    state = TONE3;
                    segs[1] = SEGS_EF;
                    segs[0] = SEGS_OFF;
                    TCA0.SINGLE.PERBUF = TONE3_PER;
                    TCA0.SINGLE.CMP0BUF = TONE3_PER >> 1;
                    current_input = 2;
                    printf("tone3\n");
                }
                else if (pb_falling_edge & PB4)
                {
                    state = TONE4;
                    segs[1] = SEGS_BC;
                    segs[0] = SEGS_OFF;
                    TCA0.SINGLE.PERBUF = TONE4_PER;
                    TCA0.SINGLE.CMP0BUF = TONE4_PER >> 1;
                    current_input = 3;
                    printf("tone4\n");
                }

                break;
            case TONE1:
                // button 1 released
                if (pb_rising_edge & PB1)
                {
                    // game_state = WAIT;
                    state = COMPARE;
                    segs[0] = SEGS_OFF;
                    segs[1] = SEGS_OFF;
                    TCA0.SINGLE.CMP0BUF = 0;
                }
                break;
            case TONE2:
                if (pb_rising_edge & PB2)
                {

                    // game_state = WAIT;
                    state = COMPARE;
                    segs[0] = SEGS_OFF;
                    segs[1] = SEGS_OFF;
                    TCA0.SINGLE.CMP0BUF = 0;
                }

                break;
            case TONE3:
                if (pb_rising_edge & PB3)
                {
                    // game_state = WAIT;
                    state = COMPARE;
                    segs[0] = SEGS_OFF;
                    segs[1] = SEGS_OFF;
                    TCA0.SINGLE.CMP0BUF = 0;
                }

                break;
            case TONE4:
                if (pb_rising_edge & PB4)
                {
                    // game_state = WAIT;
                    state = COMPARE;
                    segs[0] = SEGS_OFF;
                    segs[1] = SEGS_OFF;
                    TCA0.SINGLE.CMP0BUF = 0;
                }
                break;

                // printf("PLAY INP\n");
                // if (current_index == 5)
                //{
                //     game_state = WAIT;
                // }
            }
            break;

        case GAME_WIN:
            printf("---WIN--\n");
            break;

        case GAME_OVER:
            printf("---LOSE--\n");
            break;
        }
    }
}
