#include "hardware.h"
#include <stdio.h>
//initialise buzzer
void buzzer_init(void)
{
    PORTB.DIRSET = PIN0_bm;

    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV1_gc;


    TCA0.SINGLE.CTRLB = TCA_SINGLE_WGMODE_SINGLESLOPE_gc | TCA_SINGLE_CMP0EN_bm;

  
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;

}

void pot_init(void){
     // Enable ADC
    ADC0.CTRLA = ADC_ENABLE_bm;
    // /2 clock prescaler
    ADC0.CTRLB = ADC_PRESC_DIV2_gc;
    // Need 4 CLK_PER cycles @ 3.3 MHz for 1us, select VDD as ref
    ADC0.CTRLC = (4 << ADC_TIMEBASE_gp) | ADC_REFSEL_VDD_gc;
    // Sample duration of 64
    ADC0.CTRLE = 64;
    // Free running, left adjust result
    ADC0.CTRLF = ADC_FREERUN_bm | ADC_LEFTADJ_bm;
    // Select AIN2 (potentiomenter R1)
    ADC0.MUXPOS = ADC_MUXPOS_AIN2_gc;
    // Select 12-bit resolution, single-ended
    ADC0.COMMAND = ADC_MODE_SINGLE_8BIT_gc | ADC_START_IMMEDIATE_gc;
}
//function to constrain pot values to desired outcome
uint32_t MAP(uint32_t VALUE, uint32_t INmin, uint32_t INmax, uint32_t OUTmin, uint32_t OUTmax)
{
    return ((((VALUE - INmin)*(OUTmax - OUTmin))/(INmax - INmin)) + OUTmin);
}

