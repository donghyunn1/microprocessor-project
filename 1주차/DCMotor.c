#include <stdint.h>
#include "tm4c123gh6pm.h"

// 전역 변수
volatile unsigned long H = 40000;  // PWM High time
volatile unsigned long L = 40000;  // PWM Low time

void PortF_Init(void) {
    SYSCTL_RCGCGPIO_R |= 0x20;      // F port clock
    GPIO_PORTF_LOCK_R = 0x4C4F434B;  // unlock PF0
    GPIO_PORTF_CR_R = 0x11;          // allow PF0, PF4 to be changed
    GPIO_PORTF_DIR_R &= ~0x11;       // input on PF0, PF4
    GPIO_PORTF_PUR_R |= 0x11;        // enable pullup resistors
    GPIO_PORTF_DEN_R |= 0x11;        // digital enable
    
    GPIO_PORTF_IS_R &= ~0x11;        // edge-sensitive
    GPIO_PORTF_IBE_R &= ~0x11;       // not both edges
    GPIO_PORTF_IEV_R &= ~0x11;       // falling edge
    GPIO_PORTF_ICR_R = 0x11;         // clear flags
    GPIO_PORTF_IM_R |= 0x11;         // arm interrupt
    NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF00FFFF) | 0x00A00000;
    NVIC_EN0_R = 0x40000000;         // enable interrupt
}

void Motor_Init(void) {
    SYSCTL_RCGCGPIO_R |= 0x01;      // A port clock
    GPIO_PORTA_DIR_R |= 0x20;        // output on PA5
    GPIO_PORTA_DR8R_R |= 0x20;       // drive strength
    GPIO_PORTA_DEN_R |= 0x20;        // digital enable
    GPIO_PORTA_DATA_R &= ~0x20;      // start low
    
    NVIC_ST_CTRL_R = 0;              // disable SysTick
    NVIC_ST_RELOAD_R = L-1;          // reload value
    NVIC_ST_CURRENT_R = 0;           // clear current
    NVIC_ST_CTRL_R = 0x07;           // enable with interrupts
}

void SysTick_Handler(void) {
    if(GPIO_PORTA_DATA_R & 0x20) {    // currently high
        GPIO_PORTA_DATA_R &= ~0x20;    // go low
        NVIC_ST_RELOAD_R = L-1;        // set next period
    } else {
        GPIO_PORTA_DATA_R |= 0x20;     // go high
        NVIC_ST_RELOAD_R = H-1;        // set next period
    }
}

void GPIOPortF_Handler(void) {
    if(GPIO_PORTF_RIS_R & 0x01) {     // SW2 pressed
        GPIO_PORTF_ICR_R = 0x01;       // clear flag
        if(L > 8000) L = L - 8000;     // increase speed
    }
    if(GPIO_PORTF_RIS_R & 0x10) {     // SW1 pressed
        GPIO_PORTF_ICR_R = 0x10;       // clear flag
        if(L < 72000) L = L + 8000;    // decrease speed
    }
    H = 80000 - L;                     // maintain period
}

int main(void) {
    PortF_Init();
    Motor_Init();
    while(1) {
        // wait for interrupt
    }
}