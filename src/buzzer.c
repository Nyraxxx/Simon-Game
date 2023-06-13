#include "buzzer.h"

void buzzer_init(void)
{
    /***
     * Ex 12.2
     *
     * Complete the code in the function buzzer_init(void) to initialise
     * PORTB PIN0 and TCA0 to drive the buzzer.
     */

    PORTB.DIRSET = PIN0_bm;

    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc;


    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm;

  
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
    // Write your code for Ex 12.2 above this line
}

