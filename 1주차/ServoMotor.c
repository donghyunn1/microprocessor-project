#include <TM4C123GH6PM.h>

void SysTick_Init(void) {
    NVIC_ST_CTRL_R = 0;                // 1. SysTick 비활성화
    NVIC_ST_RELOAD_R = 0x00FFFFFF;     // 2. 최대 재장전 값 설정
    NVIC_ST_CURRENT_R = 0;             // 3. 현재 값 초기화
    // 4. SysTick 설정
    // - 시스템 클럭 사용 (비트 2: 1)
    // - 인터럽트 비활성화 (비트 1: 0)
    // - 타이머 활성화 (비트 0: 1)
    NVIC_ST_CTRL_R = 0x00000005;
}

void Servo_Init(void) {
    // GPIO 설정
    SYSCTL_RCGC2_R |= 0x00000001;     // PORTA 클럭 활성화
    GPIO_PORTA_AMSEL_R &= ~0x20;       // PA5 아날로그 기능 비활성화
    GPIO_PORTA_PCTL_R &= ~0x00F00000;  // PA5 일반 GPIO
    GPIO_PORTA_DIR_R |= 0x20;          // PA5 출력 설정  
    GPIO_PORTA_AFSEL_R &= ~0x20;       // PA5 대체기능 비활성화
    GPIO_PORTA_DEN_R |= 0x20;          // PA5 디지털 활성화
    GPIO_PORTA_DATA_R &= ~0x20;        // PA5 초기값 0
    
    // SysTick 초기화 추가
    SysTick_Init();
}

void Delay_us(unsigned long us) {
    NVIC_ST_RELOAD_R = us*16-1;        // 16MHz 기준, 마이크로초 단위 지연   
    NVIC_ST_CURRENT_R = 0;             // 카운터 초기화
    NVIC_ST_CTRL_R = 0x5;              // 활성화 및 시스템 클럭 선택
    while((NVIC_ST_CTRL_R&0x10000)==0); // COUNT 플래그 대기
}

void Delay_ms(unsigned long ms) {
    unsigned long i;
    for(i=0; i<ms; i++) {
        Delay_us(1000);  
    }
}

void Servo_Set_Angle(unsigned long angle) {
    unsigned long pulseWidth;
    int i;  // 변수를 루프 밖에서 선언
    
    pulseWidth = 500 + (((angle * 2000) / 180));  
    
    for(i=0; i<50; i++) {  
        GPIO_PORTA_DATA_R |= 0x20;     
        Delay_us(pulseWidth);          
        GPIO_PORTA_DATA_R &= ~0x20;    
        Delay_us(12500-pulseWidth);    
    }
}

int main(void) {
    Servo_Init();  

    while(1) {
            
            Servo_Set_Angle(0);
            Delay_ms(1000);
      
            Servo_Set_Angle(90);
            Delay_ms(1000);
         
        Servo_Set_Angle(180);    
        Delay_ms(1000);        
              
    }
}