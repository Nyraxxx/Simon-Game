#include "hardware.h"
#include <stdio.h>

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
//function to constrain pot values
uint32_t MAP(uint32_t VALUE, uint32_t INmin, uint32_t INmax, uint32_t OUTmin, uint32_t OUTmax)
{
    return ((((VALUE - INmin)*(OUTmax - OUTmin))/(INmax - INmin)) + OUTmin);
}

uint8_t playback_gen(uint32_t pot_read){
    uint8_t result;
    
    if( pot_read <= 0x00){
        result = 0.25;
      //  printf("0.2");

    }else if ((pot_read >= 0x00) && (pot_read <= 0x20) ){
        result = 0.35;
       // printf("0.3");
    }
    else if ((pot_read >= 0x20) && (pot_read <= 0x40) ){
        result = 0.45;
       // printf("0.4");
    }
    else if ((pot_read >= 0x40) && (pot_read <= 0x60) ){
        result = 0.55;
    }
    else if ((pot_read >= 0x60) && (pot_read <= 0x70) ){
        result = 0.65;
    }
    else if ((pot_read >= 0x70) && (pot_read <= 0x80) ){
        result = 0.75;
    }
    else if ((pot_read >= 0x80) && (pot_read <= 0x90) ){
        result = 0.85;
    }
    else if ((pot_read >= 0x90) && (pot_read <= 0xAF) ){
        result = 0.95;
    }
    else if ((pot_read >= 0xAF) && (pot_read <= 0xBF) ){
        result = 1.05;
    }
    else if ((pot_read >= 0xBF) && (pot_read <= 0xCF) ){
        result = 1.15;
    }
    else if ((pot_read >= 0xCF) && (pot_read <= 0xDF) ){
        result = 1.25;
    }
    else if ((pot_read >= 0xDF) && (pot_read <= 0xEF) ){
        result = 1.35;
    }
    else if ((pot_read >= 0xEF) && (pot_read <= 0xFF) ){
        result = 2.0;
    }
    return result;
}