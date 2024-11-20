#include <stdint.h>
#include "tm4c123gh6pm.h"
void PLL_Init(void);
void DisableInterrupts(void);
void EnableInterrupts(void);
void WaitForInterrupt(void);
unsigned long H1, L1, H2, L2;  // 두 모터의 PWM 값

void PortF_Init(void) {
   SYSCTL_RCGC2_R |= 0x00000020;     // F 포트 클럭 활성화
   while((SYSCTL_PRGPIO_R & 0x20) == 0) {};
   GPIO_PORTF_LOCK_R = 0x4C4F434B;   // 잠금 해제
   GPIO_PORTF_CR_R = 0x1F;           // 설정 허용
   GPIO_PORTF_DIR_R = 0x0E;          // 입출력 방향 설정
   GPIO_PORTF_PUR_R = 0x11;          // 풀업 저항 설정
   GPIO_PORTF_DEN_R = 0x1F;          // 디지털 활성화
   
   // 인터럽트 설정
   GPIO_PORTF_IS_R &= ~0x11;     // edge-sensitive
   GPIO_PORTF_IBE_R &= ~0x11;    // single edge
   GPIO_PORTF_IEV_R &= ~0x11;    // falling edge
   GPIO_PORTF_ICR_R = 0x11;      // clear flags
   GPIO_PORTF_IM_R |= 0x11;      // arm interrupt
   NVIC_PRI7_R = (NVIC_PRI7_R & 0xFF00FFFF) | 0x00A00000;
   NVIC_EN0_R = 0x40000000;      // enable interrupt 30
}

void Motor_Init(void) {
   SYSCTL_RCGC2_R |= 0x00000001;     // A 포트 클럭 활성화
   while((SYSCTL_PRGPIO_R & 0x01) == 0) {};
   
   L1 = L2 = 40000;                   // 초기 PWM 값
   H1 = H2 = 40000;
   
   // PA5, PA6 설정
   GPIO_PORTA_AMSEL_R &= ~0x60;       // PA5,6 아날로그 비활성화
   GPIO_PORTA_PCTL_R &= ~0x0FF00000;  // GPIO 기능
   GPIO_PORTA_DIR_R |= 0x60;          // 출력 설정
   GPIO_PORTA_DR8R_R |= 0x60;         // 8mA 드라이브
   GPIO_PORTA_AFSEL_R &= ~0x60;       // 대체 기능 비활성화
   GPIO_PORTA_DEN_R |= 0x60;          // 디지털 활성화
   GPIO_PORTA_DATA_R &= ~0x60;        // 초기값 0
   
   NVIC_ST_CTRL_R = 0;                // 비활성화
   NVIC_ST_RELOAD_R = L1-1;           // 리로드 값 설정
   NVIC_ST_CURRENT_R = 0;             // 현재 값 초기화
   NVIC_SYS_PRI3_R = (NVIC_SYS_PRI3_R&0x00FFFFFF)|0x40000000;
   NVIC_ST_CTRL_R = 0x07;             // 활성화
}

void SysTick_Handler(void) {
   static uint8_t motor = 1;
   
   if(motor == 1) {
       if(GPIO_PORTA_DATA_R & 0x20) {  // 좌측 모터 (PA5)
           GPIO_PORTA_DATA_R &= ~0x20;
           NVIC_ST_RELOAD_R = L1-1;
       } else {
           GPIO_PORTA_DATA_R |= 0x20;
           NVIC_ST_RELOAD_R = H1-1;
       }
       motor = 2;
   }
   else {
       if(GPIO_PORTA_DATA_R & 0x40) {  // 우측 모터 (PA6)
           GPIO_PORTA_DATA_R &= ~0x40;
           NVIC_ST_RELOAD_R = L2-1;
       } else {
           GPIO_PORTA_DATA_R |= 0x40;
           NVIC_ST_RELOAD_R = H2-1;
       }
       motor = 1;
   }
}

// H = 40000, L = 40000 -> 50% Duty cycle
// H = 20000, L = 60000 -> 25% Dyty cycle
// 시스템 클럭이 80MHz이므로, 하나의 클럭은 12.5ns
void GPIOPortF_Handler(void) {
   static int state1 = 0;  
   static int state2 = 0;  
   
   if(GPIO_PORTF_RIS_R & 0x10) {      // SW1 눌림
       GPIO_PORTF_ICR_R = 0x10;
       state1 ^= 1;                    // 토글 (0->1 또는 1->0)
       if(state1) {                    // 감속
           L2 = 60000;
           H2 = 20000;
       } else {                        // 정상속도
           L2 = 40000;
           H2 = 40000;
       }
   }
   
   if(GPIO_PORTF_RIS_R & 0x01) {      // SW2 눌림
       GPIO_PORTF_ICR_R = 0x01;
       state2 ^= 1;                    // 토글
       if(state2) {                    // 감속
           L1 = 60000;
           H1 = 20000;
       } else {                        // 정상속도
           L1 = 40000;
           H1 = 40000;
       }
   }
}

int main(void) {
   DisableInterrupts();                // 인터럽트 비활성화
   PLL_Init();                         // PLL 초기화 (80MHz)
   Motor_Init();                       // 모터 초기화
   PortF_Init();                       // 포트 F 초기화
   EnableInterrupts();                 // 인터럽트 활성화
   
   while(1) {
       WaitForInterrupt();             // 인터럽트 대기
   }
}