

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
// initiualise push buttons
void pb_init(void)
{
    // already configured as inputs
    // PBs PA4-7, enable pullup resistors
    PORTA.PIN4CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN5CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN6CTRL = PORT_PULLUPEN_bm;
    PORTA.PIN7CTRL = PORT_PULLUPEN_bm;
}
// button debounce called in interrupts
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
// state machine definition
typedef enum
{
    WAIT,          // delay for sequence playback
    PLAY_SEQUENCE, // plays simon sequence up until current index
    PLAYER_INPUT,  // records player input
    GAME_OVER,     // game over
    GAME_WIN,      // sucess
    COMPARE        // logic for comparing if input is correct

} game_state;
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

    return STEP;
}
//--------------------------------------------------------------------------------------------------------------------
volatile uint16_t sequence_store[32]; // 32 acccording to gradescope length
volatile uint16_t player_input[32];
uint16_t player_score;
uint8_t current_input = 0;
// generate sequence and store in array
void sequence_GEN(void)
{
    uint16_t i;

    for (i = 0; i <= 51; i++)
    {
        sequence_store[i] = sequence_STEP();

        uint8_t seqq = sequence_store[i];
    }
}
//--------------------------------------------------------------------------------------------------------------------
// Play sequence at desired tone and playback
void sequence_play(uint8_t num, uint8_t playback, uint8_t manual)
{
    // arbitrary number to do math with so loop delyays
    int arb;
    uint8_t k = 0;
    uint8_t toggle = 1;
    // toggle for buzzer on and off

    switch (toggle)
    {

    case 1:
        // tone 1
        if (num == 0)
        {

            segs[0] = SEGS_EF;
            segs[1] = SEGS_OFF;
            TCA0.SINGLE.PERBUF = TONE1_PER;
            TCA0.SINGLE.CMP0BUF = TONE1_PER >> 1;

            toggle = 0;
        }
        // tone 2
        else if (num == 1)
        {

            segs[0] = SEGS_BC;
            segs[1] = SEGS_OFF;
            TCA0.SINGLE.PERBUF = TONE2_PER;
            TCA0.SINGLE.CMP0BUF = TONE2_PER >> 1;

            toggle = 0;
        }
        // tone 3
        else if (num == 2)
        {

            segs[1] = SEGS_EF;
            segs[0] = SEGS_OFF;
            TCA0.SINGLE.PERBUF = TONE3_PER;
            TCA0.SINGLE.CMP0BUF = TONE3_PER >> 1;

            toggle = 0;
        }
        // tone 4
        else if (num == 3)
        {

            segs[1] = SEGS_BC;
            segs[0] = SEGS_OFF;
            TCA0.SINGLE.PERBUF = TONE4_PER;
            TCA0.SINGLE.CMP0BUF = TONE4_PER >> 1;

            toggle = 0;
        }
        // fall through case to turn off after delay
    case 0:
        printf("OFF");
        for (k = 0; k <= playback; k++)
        {
            arb = arb + 1;

            printf("k%d", k);
        }

        segs[0] = SEGS_OFF;
        segs[1] = SEGS_OFF;
        TCA0.SINGLE.CMP0BUF = 0;

        break;
    }
}

//--------------------------------------------------------------------------------------------------------------------
// MAIN

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
    // initialise states
    game_state game_state = PLAY_SEQUENCE;

    uint8_t current_index = 0;

    uint8_t keep_running = 1;
    uint8_t check_index = 0;
    //--------------------------------------------------------------------------------------------------------------------
    // main while loop
    while (keep_running)
    {
        uint8_t testt = ADC0.RESULT;

        uint16_t plybk = MAP(testt, -1, 255, 2500, 20000);
        // plybk = plybk >>1;

        // debouncing variables run constantly for buttons
        pb_previous_state = pb_new_state;
        pb_new_state = pb_debounced;

        // find falling edge
        uint8_t pb_falling_edge = (pb_previous_state ^ pb_new_state) & pb_previous_state;
        // find rising edge
        uint8_t pb_rising_edge = (pb_previous_state ^ pb_new_state) & pb_new_state;

        // arbitrary value for math related delays
        uint8_t arb;
        switch (game_state)
        {
            // delay before playing seqence
        case WAIT:
            for (int k = 0; k <= 100; k++)
            {
                arb = arb + 1;
                printf("k%d", k);
            }

            // then play sequence
            game_state = PLAY_SEQUENCE;
            break;

        case PLAY_SEQUENCE:

            //  loop for how many steps we are up to
            for (int j = 0; j <= current_index; j++)
            {

                // play tone/display current step in sequence
                sequence_play(sequence_store[j], plybk, 0);
            }

            // reset value to test sequence  correctly
            current_input = NULL;
            // move to player input so player can try input sequence

            game_state = PLAYER_INPUT;

            break;
            // player input case
        case PLAYER_INPUT:
            // test for button presses
            // button 1
            if (pb_falling_edge & PB1)
            {

                current_input = 0;
                sequence_play(0, plybk, 0);

                printf("tone1\n");
                // button 1 released
                game_state = COMPARE;
            }
            // button 2
            else if (pb_falling_edge & PB2)
            {
                sequence_play(1, plybk, 0);
                current_input = 1;
                printf("tone2\n");
                game_state = COMPARE;
            }
            // button 3
            else if (pb_falling_edge & PB3)
            {
                sequence_play(2, plybk, 0);
                current_input = 2;
                printf("tone3\n");
                game_state = COMPARE;
            }
            // button 4;
            else if (pb_falling_edge & PB4)
            {
                sequence_play(3, plybk, 0);
                current_input = 3;
                printf("tone4\n");
                game_state = COMPARE;
            }

            break;
            // logic state
        case COMPARE:
            // lose if press wrong button
            if (current_input != sequence_store[check_index])
            {
                game_state = GAME_OVER;
                break;
            }
            // continue loop if correct
            if (check_index++ == current_index)
            {
                current_index++;
                player_score = current_index++ + player_score;
                check_index = 0;
                printf("SUCCESS\n");

                printf("SCORE: %d\n"), player_score;
                game_state = WAIT;
            }
            // win case
            if (current_index == 32)
            {
                game_state = GAME_WIN;
            }
            // return to player input after logic state
            if (game_state == COMPARE)
            {
                game_state = PLAYER_INPUT;
            }
            break;
            // win, light up segments
        case GAME_WIN:
            printf("SUCCESS\n");
            printf("SCORE: %d\n", player_score);

            // break while
            keep_running = false;
            break;
            // lose, break while
        case GAME_OVER:
            printf("GAME OVER\n");
            printf("SCORE\n");
            keep_running = false;
            break;
        }
    }
}