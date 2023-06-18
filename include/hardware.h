#include <avr/io.h>
#include <avr/interrupt.h>


#define TONE1_PER 10965
#define TONE2_PER 13021
#define TONE3_PER 8210
#define TONE4_PER 21929

//prototypes
void buzzer_init(void);
void pot_init(void);
uint32_t MAP(uint32_t VALUE, uint32_t INmin, uint32_t INmax, uint32_t OUTmin, uint32_t OUTmax);
