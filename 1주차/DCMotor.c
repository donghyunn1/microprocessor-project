#include <stdint.h>
#include "tm4c123gh6pm.h"

unsigned long H, L; 

void Motor_Init(void) {
    SYSCTL_RCGC2_R |= 0x00000001;        // activate clock for port A
    L = H = 40000;                        // 50%
    GPIO_PORTA_AMSEL_R &= ~0x20;          // disable analog on PA5
    GPIO_PORTA_PCTL_R &= ~0x00F00000;     // configure PA5 as GPIO
    GPIO_PORTA_DIR_R |= 0x20;             // make PA5 out
    GPIO_PORTA_DR8R_R |= 0x20;            // enable 8 mA drive
    GPIO_PORTA_AFSEL_R &= ~0x20;          // disable alt funct
    GPIO_PORTA_DEN_R |= 0x20;             // enable digital I/O
    GPIO_PORTA_DATA_R &= ~0x20;           // make PA5 low
    NVIC_ST_CTRL_R = 0;                   // disable SysTick during setup
    NVIC_ST_RELOAD_R = L-1;               // reload value for 500us
    NVIC_ST_CURRENT_R = 0;                // any write to current clears it
    NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000; // priority 2
    NVIC_ST_CTRL_R = 0x00000007;          // enable with core clock and interrupts
}

void SysTick_Handler(void) {
    if(GPIO_PORTA_DATA_R&0x20) {          // toggle PA5
        GPIO_PORTA_DATA_R &= ~0x20;        // make PA5 low
        NVIC_ST_RELOAD_R = L-1;           // reload value for low phase
    } else {
        GPIO_PORTA_DATA_R |= 0x20;         // make PA5 high
        NVIC_ST_RELOAD_R = H-1;           // reload value for high phase
    }
}

int main(void) {
    DisableInterrupts();                  // disable interrupts while initializing
    PLL_Init();                           // bus clock at 80 MHz
    Motor_Init();                         // output from PA5, SysTick interrupts
    EnableInterrupts();                   // enable after all initialization done
    while(1) {
        WaitForInterrupt();               // low power mode
    }
}