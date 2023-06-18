#include <avr/io.h>
#include <avr/interrupt.h>


#define TONE1_PER 10965
#define TONE2_PER 13021
#define TONE3_PER 8210
#define TONE4_PER 21929
//
//#define TONE1_PER 304
//#define TONE2_PER 256
//#define TONE3_PER 406
//#define TONE4_PER 152
// Write your code for Ex 12.2 above this line

void buzzer_init(void);
void pot_init(void);
uint32_t MAP(uint32_t VALUE, uint32_t INmin, uint32_t INmax, uint32_t OUTmin, uint32_t OUTmax);
