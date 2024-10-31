/*
PIC Config

    OSCCON - OSCILLATOR CONTROL REGISTER
        100 = 1 MHz
        101 = 2 MHz
        110 = 4 MHz
        111 = 8 MHz

    A/D CONVERSION CLOCK
        000 = Fosc / 2
        100 = Fosc / 4
        001 = Fosc / 8
        101 = Fosc / 16
        010 = Fosc / 32
        110 = Fosc / 64

    SPI CLOCK
        0000 = Fosc / 4
        0001 = Fosc / 16
        0010 = Fosc / 64

 */


 // 내부클럭 사용시(기본 4MHz)
 //#pragma config FOSC = INTRCIO 

 //외부 클럭 사용 (20MHz-FPGA랑 같은 OSC)
#pragma config FOSC = HS        
#pragma config WDTE = OFF      
#pragma config PWRTE = OFF     
#pragma config MCLRE = ON      
#pragma config CP = OFF        
#pragma config CPD = OFF       
#pragma config BOREN = OFF     
#pragma config IESO = OFF      
#pragma config FCMEN = OFF     

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>

//#define _XTAL_FREQ 4000000     
#define _XTAL_FREQ 20000000 

#define DS_1 PORTCbits.RC0
#define DS_2 PORTCbits.RC1
#define DS_3 PORTCbits.RC2
#define DS_4 PORTCbits.RC5
#define DS_5 PORTCbits.RC4
#define DS_6 PORTCbits.RC3
#define DS_7 PORTCbits.RC6
#define DS_8 PORTCbits.RC7
#define DS_9 PORTBbits.RB4
#define DS_10 PORTBbits.RB5

#define P_DS_1 TRISCbits.TRISC0
#define P_DS_2 TRISCbits.TRISC1
#define P_DS_3 TRISCbits.TRISC2
#define P_DS_4 TRISCbits.TRISC5
#define P_DS_5 TRISCbits.TRISC4
#define P_DS_6 TRISCbits.TRISC3
#define P_DS_7 TRISCbits.TRISC6
#define P_DS_8 TRISCbits.TRISC7
#define P_DS_9 TRISBbits.TRISB4
#define P_DS_10 TRISBbits.TRISB5


#define TX PORTBbits.RB7
#define RX PORTBbits.RB6

#define P_TX TRISBbits.TRISB7
#define P_RX TRISBbits.TRISB6

/*
    AN2 핀 사용
    FOSC/16
        ->  TAD = 16/20MHz = 0.8
            A/D 변환주기 = 11*0.8

        ->  샘플링 주파수 = 1/8.8 = 113.6
        ->  샘플링 주파수 113.6kHz
 */
void ADC_Init() {
    ANSEL = 0x00;
    ANSELH = 0x00;
    TRISAbits.TRISA2 = 1;
    ANSELbits.ANS2 = 1;
    ADCON0bits.CHS = 2;
    ADCON1bits.ADCS = 0b101;
    ADCON0bits.ADFM = 1;
    ADCON0bits.ADON = 1;
    ADCON0bits.VCFG = 0;
}


/*
    ADC 변환값 : 10bit
    ADRESH : 상위 2bit
    ADRESL : 하위 8bit
*/
unsigned short ADC_Read() {
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO_nDONE);
    return ((unsigned short)ADRESH << 8) | ADRESL;
}

// 1~10bit 각각의 핀으로 동시에 데이터 전송
void sendParallelData(unsigned short data) {
    DS_1 = (data >> 0) & 1;
    DS_2 = (data >> 1) & 1;
    DS_3 = (data >> 2) & 1;
    DS_4 = (data >> 3) & 1;
    DS_5 = (data >> 4) & 1;
    DS_6 = (data >> 5) & 1;
    DS_7 = (data >> 6) & 1;
    DS_8 = (data >> 7) & 1;
    DS_9 = (data >> 8) & 1;
    DS_10 = (data >> 9) & 1;
}

void main(void) {
    ADC_Init();

    P_DS_1 = 0;
    P_DS_2 = 0;
    P_DS_3 = 0;
    P_DS_4 = 0;
    P_DS_5 = 0;
    P_DS_6 = 0;
    P_DS_7 = 0;
    P_DS_8 = 0;
    P_DS_9 = 0;
    P_DS_10 = 0;

    P_TX = 0;
    P_RX = 1;


    TX = 0; //전송대기

    while (1) {
        /*
            RX = 0 : FPGA 수신 준비완료
            RX = 1 : FPGA 데이터 처리 중

            TX = 0 : FPGA로 데이터 보냄
            TX = 1 : FPGA 데이터 전송 끝
        */
        if (RX == 1) {
            unsigned short data = ADC_Read();
            TX = 1;
            sendParallelData(data);
            TX = 0;
        }
    }
}