// LaunchPadSwitches.c
// Runs on TM4C123
// Provide functions that initialize a GPIO as an input pin and
// allow reading of two negative logic switches on PF0 and PF4
// Output to LEDs
// Use bit-banded I/O.

#include <stdint.h>
#include "inc/tm4c123gh6pm.h"

#define GPIO_LOCK_KEY           0x4C4F434B  // Unlocks the GPIO_CR register
#define PF0                     (*((volatile uint32_t *)0x40025004))
#define PF4                     (*((volatile uint32_t *)0x40025040))
#define SWITCHES                (*((volatile uint32_t *)0x40025044))
#define SW1       0x10                      // on the left side of the Launchpad board
#define SW2       0x01                      // on the right side of the Launchpad board
#define PF1                     (*((volatile uint32_t *)0x40025008))

//------------Board_Init------------
// Initialize GPIO Port F for negative logic switches on PF0 and
// PF4 as the Launchpad is wired.  Weak internal pull-up
// resistors are enabled, and the NMI functionality on PF0 is
// disabled.
// Input: none
// Output: none
void Board_Init(void){       
  SYSCTL_RCGCGPIO_R |= 0x20;     // 1) activate Port F
  while((SYSCTL_PRGPIO_R & 0x20)!=0x20){}; // wait to finish activating     
  GPIO_PORTF_LOCK_R = GPIO_LOCK_KEY;// 2a) unlock GPIO Port F Commit Register
  GPIO_PORTF_CR_R = 0x1F;        // 2b) enable commit for PF4-PF0     
  GPIO_PORTF_AMSEL_R &= ~0x1F;   // 3) disable analog functionality on PF4-PF0     
  GPIO_PORTF_PCTL_R = 0x00000000;// 4) configure PF0-PF4 as GPIO
  GPIO_PORTF_DIR_R = 0x0E;       // 5) make PF0 and PF4 in PF3-1 output                        
  GPIO_PORTF_AFSEL_R = 0;        // 6) disable alt funct on PF0 and PF4
  GPIO_PORTF_PUR_R = 0x11;       // enable weak pull-up on PF0 and PF4
  GPIO_PORTF_DEN_R = 0x1F;       // 7) enable digital I/O on PF0-PF4
}

//------------Board_Input------------
// Read and return the status of the switches.
// Input: none
// Output: 2 if only Switch 1 is pressed
//         1 if only Switch 2 is pressed
//         3 if both switches are pressed
//         0 if no switches are pressed
uint32_t Board_Input(void){
  return (~((PF4>>3)+PF0))&0x03;
}
//------------LED_On------------
// Turn on LED
// Input: none
// Output: none
void LED_On(void){
  PF1 = 0x02;
}
//------------LED_Off------------
// Turn on LED
// Input: none
// Output: none
void LED_Off(void){
  PF1 = 0x00;
}
//------------LED_Toggle------------
// Turn on LED
// Input: none
// Output: none
void LED_Toggle(void){
  PF1 ^= 0x02;
}

/****************Int2Str***************
 converts signed integer number to ASCII string
 format signed 32-bit 
 range -99999 to +99999
 Input: signed 32-bit integer 
 Output: null-terminated string exactly 7 characters plus null
 Examples
  12345 to " 12345"  
 -82100 to "-82100"
   -102 to "  -102" 
     31 to "    31" 
   
 */ 
void Int2Str(int32_t const num, char *string){
  int32_t n;

  if(num<0){
    n = -num;
  } else{
    n = num;
  }
  if(n>99999){      // too big
    string[0] = '*';
    string[1] = '*';
    string[2] = '*';
    string[3] = '*';
    string[4] = '*';
    string[5] = '*';
 //   string[6] = 0;
    return;
  }
  if(n>9999){  // 10000 to 99999
    if(num<0){
      string[0] = '-';
    } else {
      string[0] = ' ';
    }
    string[1] = '0'+n/10000;
    n = n%10000;
    string[2] = '0'+n/1000;
    n = n%1000;
    string[3] = '0'+n/100;
    n = n%100;
    string[4] = '0'+n/10;
    n = n%10;
    string[5] = '0'+n;
 //   string[6] = 0; 
    return; 
  } 
  if(n>999){   // 1000 to 9999
    string[0] = ' ';
    if(num<0){
      string[1] = '-';
    } else {
      string[1] = ' ';
    }
    string[2] = '0'+n/1000;
    n = n%1000;
    string[3] = '0'+n/100;
    n = n%100;
    string[4] = '0'+n/10;
    n = n%10;
    string[5] = '0'+n;
  //  string[6] = 0; 
    return; 
  } 
  if(n>99){   // 100 to 999
    string[0] = ' ';
    string[1] = ' ';
    if(num<0){
      string[2] = '-';
    } else {
      string[2] = ' ';
    }
    string[3] = '0'+n/100;
    n = n%100;
    string[4] = '0'+n/10;
    n = n%10;
    string[5] = '0'+n;
 //   string[6] = 0; 
    return; 
  } 
  if(n>9){   // 10 to 99
    string[0] = ' ';
    string[1] = ' ';
    string[2] = ' ';
    if(num<0){
      string[3] = '-';
    } else {
      string[3] = ' ';
    }
    string[4] = '0'+n/10;
    n = n%10;
    string[5] = '0'+n;
  //  string[6] = 0; 
    return; 
  } 
  // 0 to 9
  string[0] = ' ';
  string[1] = ' ';
  string[2] = ' ';
  string[3] = ' ';
  if(num<0){
    string[4] = '-';
  } else {
    string[4] = ' ';
  }
  string[5] = '0'+n;
//  string[6] = 0; 
}
