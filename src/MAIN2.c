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
    WAIT_T

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
void sequence_GEN(void)
{
    uint16_t i;

    for (i = 0; i <= 51; i++)
    {
        sequence_store[i] = sequence_STEP();
        // printf("seq\n");
        // printf(sequence_store[i]);
        uint8_t seqq = sequence_store[i];
        // printf("sequence step %d\n", seqq);
    }
}
//--------------------------------------------------------------------------------------------------------------------
void test_play(uint8_t num, uint8_t playbac)
{

    uint8_t k = 0;
    if (num == 1)
    {
        for (k = 0; k <= playbac; k++)
        {
            segs[0] = SEGS_EF;
            segs[1] = SEGS_OFF;
            // TCA0.SINGLE.PERBUF = TONE1_PER;
            // TCA0.SINGLE.CMP0BUF = TONE1_PER >> 1;
            printf("pb %d\n", k);
        }

        segs[0] = SEGS_OFF;
        segs[1] = SEGS_OFF;
        TCA0.SINGLE.CMP0BUF = 0;
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
        if (num == 1)
        {

            segs[0] = SEGS_EF;
            segs[1] = SEGS_OFF;
            TCA0.SINGLE.PERBUF = TONE1_PER;
            TCA0.SINGLE.CMP0BUF = TONE1_PER >> 1;
            toggle = 0;
        }
        else if (num == 2)
        {

            segs[0] = SEGS_BC;
            segs[1] = SEGS_OFF;
            TCA0.SINGLE.PERBUF = TONE2_PER;
            TCA0.SINGLE.CMP0BUF = TONE2_PER >> 1;
            toggle = 0;
        }
        else if (num == 3)
        {

            segs[1] = SEGS_EF;
            segs[0] = SEGS_OFF;
            TCA0.SINGLE.PERBUF = TONE3_PER;
            TCA0.SINGLE.CMP0BUF = TONE3_PER >> 1;
            toggle = 0;
        }
        else if (num == 4)
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

            // printf("k%d", k);
        }
        // printf("OFF");

        segs[0] = SEGS_OFF;
        segs[1] = SEGS_OFF;
        TCA0.SINGLE.CMP0BUF = 0;

        break;
    }
}
uint8_t current_step = 0;
uint8_t fake_bool = 0;
uint8_t check_player_ans()
{
    for (int i = 0; i < current_step; i++)
    {
        uint8_t expected_input = sequence_store[i];
        uint8_t actual_input;
        if (player_input[i] != sequence_store[i])
        {
            fake_bool = 0;
        }
        else
        {
            fake_bool = 1;
        }
    }
    return fake_bool;
}

//--------------------------------------------------------------------------------------------------------------------
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
    // initial sequence length

    // sequence_play(1, 25);
    int j = 0;

    tone_state state = WAIT_T;
    game_state game_state = PLAYER_INPUT;

    // initial sequence length

    //--------------------------------------------------------------------------------------------------------------------
    int inp = 0;
    while (1)
    {
        uint8_t testt = 255 -  ADC0.RESULT;
        printf("aedc %d\n", testt);

        uint8_t plybk = MAP(testt, 0, 255, 25, 200);

        //--------------------------------------------------------------------------------------------------------------------
        int c = 0;
        int points = 0;
        switch (game_state)
        {
        case WAIT:
            // //check player answers
            // printf("play in %d\n", player_input[c+1] );
            // printf("seq st %d\n", sequence_store[c] );
            // uint8_t test_case = check_player_ans();
            // if (test_case == 1){
            //      printf("CORRECT\n");
            //      game_state = PLAYER_INPUT;
            // } else if (test_case == 0){
            //  printf("LOSE\n");
            //  break;
            // }
            while (c <= current_step - 1)
            {
                c = c + 1;

                printf("SEQ %d", sequence_store[c]);
                printf("INP %d", inp);
                if (inp == sequence_store[c])
                {
                    
                    game_state = PLAYER_INPUT;
                }
                else if (inp != sequence_store[c])
                {
                    printf("LOSE\n");
                    game_state = GAME_OVER;
                    break;
                }

                break;
            }
            case PLAY_SEQUENCE:
                for (int j = 0; j < current_step; j++)
                {

                    // play tone/display current step in sequence
                    sequence_play(sequence_store[j], plybk);
                    printf("sequence store val %d\n", sequence_store[j]);
                    printf("j %d\n", j);
                    printf("step %d\n", current_step);
                }

                printf("MARK");

                game_state = PLAYER_INPUT;

                break;
            case PLAYER_INPUT:

                pb_previous_state = pb_new_state;
                pb_new_state = pb_debounced;

                // find falling edge
                uint8_t pb_falling_edge = (pb_previous_state ^ pb_new_state) & pb_previous_state;
                // find rising edge
                uint8_t pb_rising_edge = (pb_previous_state ^ pb_new_state) & pb_new_state;
                switch (state)
                {
                case WAIT_T:
                    // turn off everything
                    TCA0.SINGLE.CMP0BUF = 0;
                    segs[0] = SEGS_OFF;
                    segs[1] = SEGS_OFF;

                    if (pb_falling_edge & PB1)
                    {

                        state = TONE1;
                        segs[0] = SEGS_EF;
                        segs[1] = SEGS_OFF;
                        TCA0.SINGLE.PERBUF = TONE1_PER;
                        TCA0.SINGLE.CMP0BUF = TONE1_PER >> 1;
                        player_input[current_step] = 1;
                        inp = 1;

                        printf("tone1");
                        printf("step %d\n", current_step);
                        printf("input %d \n", player_input[current_step]);
                        current_step = current_step + 1;
                    }
                    else if (pb_falling_edge & PB2)
                    {
                        state = TONE2;
                        segs[0] = SEGS_BC;
                        segs[1] = SEGS_OFF;
                        TCA0.SINGLE.PERBUF = TONE2_PER;
                        TCA0.SINGLE.CMP0BUF = TONE2_PER >> 1;
                        player_input[current_step] = 2;
                        inp = 2;

                        printf("tone2\n");
                        printf("input %d \n", player_input[current_step]);
                        current_step = current_step + 1;
                    }
                    else if (pb_falling_edge & PB3)
                    {
                        state = TONE3;
                        segs[1] = SEGS_EF;
                        segs[0] = SEGS_OFF;
                        TCA0.SINGLE.PERBUF = TONE3_PER;
                        TCA0.SINGLE.CMP0BUF = TONE3_PER >> 1;
                        player_input[current_step] = 3;
                        inp = 3;

                        printf("tone3\n");
                        printf("input %d \n", player_input[current_step]);
                        current_step = current_step + 1;
                    }
                    else if (pb_falling_edge & PB4)
                    {
                        state = TONE4;
                        segs[1] = SEGS_BC;
                        segs[0] = SEGS_OFF;
                        TCA0.SINGLE.PERBUF = TONE4_PER;
                        TCA0.SINGLE.CMP0BUF = TONE4_PER >> 1;
                        player_input[current_step] = 4;
                        printf("tone4\n");
                        printf("input %d \n", player_input[current_step]);
                        current_step = current_step + 1;
                        inp = 4;
                    }

                    break;
                case TONE1:
                    if (pb_rising_edge & PB1)
                    {

                        game_state = WAIT;
                        state = WAIT_T;
                        segs[0] = SEGS_OFF;
                        segs[1] = SEGS_OFF;
                        TCA0.SINGLE.CMP0BUF = 0;
                    }

                    break;
                case TONE2:
                    if (pb_rising_edge & PB2)
                    {

                        game_state = WAIT;
                        state = WAIT_T;
                        segs[0] = SEGS_OFF;
                        segs[1] = SEGS_OFF;
                        TCA0.SINGLE.CMP0BUF = 0;
                    }

                    break;
                case TONE3:
                    if (pb_rising_edge & PB3)
                    {
                        game_state = WAIT;
                        state = WAIT_T;
                        segs[0] = SEGS_OFF;
                        segs[1] = SEGS_OFF;
                        TCA0.SINGLE.CMP0BUF = 0;
                    }

                    break;
                case TONE4:
                    if (pb_rising_edge & PB4)
                    {
                        game_state = WAIT;
                        state = WAIT_T;
                        segs[0] = SEGS_OFF;
                        segs[1] = SEGS_OFF;
                        TCA0.SINGLE.CMP0BUF = 0;
                    }
                    break;
                }

                break;
            case GAME_OVER:
                break;
            case GAME_WIN:
                break;
            }
        }
    }