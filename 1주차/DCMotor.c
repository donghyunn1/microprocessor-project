#include <stdint.h>
#include "tm4c123gh6pm.h"

unsigned long H1, L1, H2, L2;  // 두 모터의 PWM 값

void Motor_Init(void) {
   SYSCTL_RCGC2_R |= 0x00000001;        // activate clock for port A
   H1 = L1 = H2 = L2 = 40000;           // 50% duty cycle for both motors
   
   // Configure PA5 and PA6 for motors
   GPIO_PORTA_AMSEL_R &= ~0x60;          // disable analog on PA5,6
   GPIO_PORTA_PCTL_R &= ~0x0FF00000;     // configure as GPIO
   GPIO_PORTA_DIR_R |= 0x60;             // make outputs
   GPIO_PORTA_DR8R_R |= 0x60;            // enable 8 mA drive
   GPIO_PORTA_AFSEL_R &= ~0x60;          // disable alt funct
   GPIO_PORTA_DEN_R |= 0x60;             // enable digital I/O
   GPIO_PORTA_DATA_R &= ~0x60;           // make low
   
   NVIC_ST_CTRL_R = 0;                   
   NVIC_ST_RELOAD_R = L1-1;              
   NVIC_ST_CURRENT_R = 0;                
   NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000;
   NVIC_ST_CTRL_R = 0x00000007;          
}

void SysTick_Handler(void) {
   static uint8_t motor = 1;  // 모터 번호 추적
   
   if(motor == 1) {
       if(GPIO_PORTA_DATA_R & 0x20) {    // Motor 1 (PA5)
           GPIO_PORTA_DATA_R &= ~0x20;    
           NVIC_ST_RELOAD_R = L1-1;
       } else {
           GPIO_PORTA_DATA_R |= 0x20;
           NVIC_ST_RELOAD_R = H1-1;
       }
       motor = 2;
   }
   else {
       if(GPIO_PORTA_DATA_R & 0x40) {    // Motor 2 (PA6)
           GPIO_PORTA_DATA_R &= ~0x40;    
           NVIC_ST_RELOAD_R = L2-1;
       } else {
           GPIO_PORTA_DATA_R |= 0x40;
           NVIC_ST_RELOAD_R = H2-1;
       }
       motor = 1;
   }
}

int main(void) {
   DisableInterrupts();
   PLL_Init();
   Motor_Init();
   EnableInterrupts();
   while(1) {
       WaitForInterrupt();
   }
}