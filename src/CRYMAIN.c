

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
    COMPARE

} game_state;
//--------------------------------------------------------------------------------------------------------------------
typedef enum
{
    TONE1,
    TONE2,
    TONE3,
    TONE4,
    WAIT_T,
    // COMPARE

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
    printf("\nSEQUENCE MASTER: \n");
    for (i = 0; i <= 51; i++)
    {
        sequence_store[i] = sequence_STEP();

        // printf(sequence_store[i]);
        uint8_t seqq = sequence_store[i];
        printf(" %d", seqq);
    }
}
//--------------------------------------------------------------------------------------------------------------------
void sequence_play(uint8_t num, uint8_t playback, uint8_t manual)
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
        printf("OFF");
        for (k = 0; k <= playback; k++)
        {
            arb = arb + 1;

            printf("k%d", k);
        }
        // printf("CASE 0");
        // if (manual == 1)
        //{
        //    while (1)
        //    {
        //        pb_previous_state = pb_new_state;
        //        pb_new_state = pb_debounced;
        //
        //        // find rising edge
        //        uint8_t rising_edge = (pb_previous_state ^ pb_new_state) & pb_new_state;
        //        if (rising_edge & (PB1 | PB2 | PB3 | PB4))
        //        {
        //            segs[0] = SEGS_OFF;
        //            segs[1] = SEGS_OFF;
        //            TCA0.SINGLE.CMP0BUF = 0;
        //            break;
        //        }
        //    }
        //}
        // else
        //{

        //

        segs[0] = SEGS_OFF;
        segs[1] = SEGS_OFF;
        TCA0.SINGLE.CMP0BUF = 0;
        printf("stop\n");
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
    //  uint8_t success = 0;
    char UART_IN;
    //.char UART_IN = getchar();
    //--------------------------------------------------------------------------------------------------------------------
    uint8_t keep_running = 1;
    uint8_t check_index = 0;

    while (keep_running)
    {
        uint8_t testt = 255 - ADC0.RESULT;

        uint8_t plybk = MAP(testt, 0, 255, 25, 200);

        pb_previous_state = pb_new_state;
        pb_new_state = pb_debounced;

        // find falling edge
        uint8_t pb_falling_edge = (pb_previous_state ^ pb_new_state) & pb_previous_state;
        // find rising edge
        uint8_t pb_rising_edge = (pb_previous_state ^ pb_new_state) & pb_new_state;

        // button 1

        /*
        bool keep_running = true;

    while (keep_running)
    {
    switch (game_state)
    {
        case WAIT
            break;

        case OUTPUT
            for (i from 0 to current_limit)
                print(stored[i]);

            game_state = INPUT;
            break;

        case INPUT
            last_input = get_input();
            game_state = CHECK;
            break;

        case CHECK
            if (last_input != stored[check_index])
            {
                game_state = LOSE;
                break;
            }

            if (counter++ == current_limit)
            {
                current_limit++;
                counter = 0;
                game_state = OUTPUT;
            }

            if (current_limit == total_length)
            {
                game_state = WIN;
            }

            break;

        case WIN
            print "YOU WIN LOL";
            keep_running = false;
            break;

        case LOSE
            print "FAK YOU GOOBY";
            keep_running = false;
            break;
    }
    }
        */
       uint8_t arb;
        switch (game_state)
        {
        case WAIT:
            for (int k = 0; k <= 100; k++)
            {
                    arb = arb + 1;
                printf("k%d", k);
            }
            game_state = PLAY_SEQUENCE;
            break;
        case PLAY_SEQUENCE:

            //  loop for how many steps we are up to
            for (int j = 0; j <= current_index; j++)
            {

                // play tone/display current step in sequence
                sequence_play(sequence_store[j], 50, 0);
                printf("sequence store val @ index %d\n", sequence_store[j]);
                printf("index of for loop %d\n", j);
            }
            printf("current highest step %d\n", current_index);
            printf("END OF PLAY SEQ\n");
            // move up to the next step
            // current_index++;
            // move to player input so player can try input sequence
            current_input = NULL;
            game_state = PLAYER_INPUT;

            break;
        case PLAYER_INPUT:

            if (pb_falling_edge & PB1)
            {

                current_input = 0;
                sequence_play(0, plybk, 0);

                printf("tone1\n");
                // button 1 released
                game_state = COMPARE;
            }
            else if (pb_falling_edge & PB2)
            {
                sequence_play(1, plybk, 0);
                current_input = 1;
                printf("tone2\n");
                game_state = COMPARE;
            }
            else if (pb_falling_edge & PB3)
            {
                sequence_play(2, plybk, 0);
                current_input = 2;
                printf("tone3\n");
                game_state = COMPARE;
            }
            else if (pb_falling_edge & PB4)
            {
                sequence_play(3, plybk, 0);
                current_input = 3;
                printf("tone4\n");
                game_state = COMPARE;
            }

            break;
        case COMPARE:
            if (current_input != sequence_store[check_index])
            {
                game_state = GAME_OVER;
                break;
            }

            if (check_index++ == current_index)
            {
                current_index++;
                check_index = 0;
                game_state = WAIT;
            }

            if (current_index == 32)
            {
                game_state = GAME_WIN;
            }
            if (game_state == COMPARE)
            {
                game_state = PLAYER_INPUT;
            }
            break;
        case GAME_WIN:
            printf("YOU WIN LOL\n");
            keep_running = false;
            break;
        case GAME_OVER:
            printf("FAK YOU GOOBY\n");
            keep_running = false;
            break;
        }
    }
}