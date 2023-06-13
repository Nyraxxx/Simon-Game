#include <avr/io.h>
#include <avr/interrupt.h>

 /***
  * Ex 12.2
  * Determine your frequencies according to your student number, and 
  * work out appropriate values for TCA0.SINGLE.PER in each case.  
  * Update the #defines below to reflect these values.
  */
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
