#include "t"
void simon_gen(uint_8){
    
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
            }
            else if (pb_falling_edge & PB2)
            {
                state = TONE2;
                segs[0] = SEGS_BC;
                segs[1] = SEGS_OFF;
                TCA0.SINGLE.PERBUF = TONE2_PER;
                TCA0.SINGLE.CMP0BUF = TONE2_PER >> 1;
            }
            else if (pb_falling_edge & PB3)
            {
                state = TONE3;
                segs[1] = SEGS_EF;
                segs[0] = SEGS_OFF;
                TCA0.SINGLE.PERBUF = TONE3_PER;
                TCA0.SINGLE.CMP0BUF = TONE3_PER >> 1;
            }
            else if (pb_falling_edge & PB4)
            {
                state = TONE4;
                segs[1] = SEGS_BC;
                segs[0] = SEGS_OFF;
                TCA0.SINGLE.PERBUF = TONE4_PER;
                TCA0.SINGLE.CMP0BUF = TONE4_PER >> 1;
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