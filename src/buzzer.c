#include "buzzer.h"

void buzzer_init(void)
{
    PORTB.DIRSET = PIN0_bm;

    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc;


    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm;

  
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;

}

