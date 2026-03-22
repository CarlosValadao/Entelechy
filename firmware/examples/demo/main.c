/*
    AT89S51
    12MHz Crystal
*/

/*
    * Verificar o possível travamento ao habilitar o print da leitura do dht11 no display LCD
    * Verificar se o código do 595 está funcionando conforme visto no pulse view
*/


#include "lcd16x2_i2c.h"
#include <reg51.h>
#include <intrins.h>
#include <stdio.h>


#define T0_RELOAD_HIGH_50MS 0x3C
#define T0_RELOAD_LOW_50MS  0xB0

#define LCD16x2_SLAVE_ADDR 0x4E

#define DELAY_8US() do { \
                        _nop_(); \
                        _nop_(); \
                        _nop_(); \
                        _nop_(); \
                        _nop_(); \
                        _nop_(); \
                    } while(0)

/*
   All arguments shoud be sbit type
*/
#define IS_BUTTON_PRESSED(key) (!(key))
#define IS_SWITCH_ON(sw) (!(sw))
#define TURN_ON_LED(led) ((led) = 0)
#define TURN_OFF_LED(led) ((led) = 1)


#define IS_FOUR_PUSH_BUTTONS_PRESSED()  IS_BUTTON_PRESSED(KEY0) && \
                                        IS_BUTTON_PRESSED(KEY1) && \
                                        IS_BUTTON_PRESSED(KEY2) && \
                                        IS_BUTTON_PRESSED(KEY3)

#define FALSE 0
#define TRUE  1

sbit SW0 = P1^0;
sbit SW1 = P1^1;
sbit SW2 = P1^2;
sbit SW3 = P1^3;
sbit SW4 = P1^4;

sbit KEY0 = P3^0;
sbit KEY1 = P3^1;
sbit KEY2 = P3^2;
sbit KEY3 = P3^3;

sbit LED0 = P0^2;
sbit LED1 = P0^3;
sbit LED2 = P0^4;

sbit LED_RGB_RED   = P0^5;
sbit LED_RGB_GREEN = P0^6;
sbit LED_RGB_BLUE  = P0^7;

sbit HC595_SER     = P2^0;
sbit HC595_RCLK    = P2^1;
sbit HC595_SRCLK   = P2^2;
sbit HC595_SRCLR_N = P2^3;

sbit DHT11_DATA    =  P2^4;

sbit BUZZER_SIG    = P2^5;

sbit DIG1_7SEG     =  P2^6;
sbit DIG2_7SEG     =  P2^7;

bit DHT11_READABLE = FALSE;

volatile unsigned char dht11_data[5] = {0};
volatile char buffer[3];

volatile unsigned char timer0_tick_count = 0;
volatile unsigned char dht11_tick_count  = 0;

// Padrão do símbolo de grau °
unsigned char code degree_char[8] = {
    0x07,  // 0 1 1 1 0
    0x05,  // 0 1 0 1 0
    0x07,  // 0 1 1 1 0
    0x00,  // 0 0 0 0 0
    0x00,  // 0 0 0 0 0
    0x00,  // 0 0 0 0 0
    0x00,  // 0 0 0 0 0
    0x00   // 0 0 0 0 0
};


unsigned char dht11_get_byte() {
    unsigned char i, byte;
    /*unsigned char timeout;
    timeout = 0;*/  
    byte = 0x00;
    for(i = 0; i < 8; i++) {
        while(!DHT11_DATA);
        DELAY_8US();
        DELAY_8US();
        DELAY_8US();
        DELAY_8US();
        DELAY_8US();
        byte <<= 1;
        byte |= DHT11_DATA;
        while(DHT11_DATA);
    }
    return byte;
}


void dht11_get_raw_data() {
    EA = 0;
    // unsigned char t = 0;
    TMOD &= 0x0F; // clear Timer1 bits
    TMOD |= 0x10; // setup 16bit mode
    TF1 = 0; // clear the timer1 overflow flag
    TH1 = 0xB9; // 18ms counting
    TL1 = 0xB0;
    DHT11_DATA = 0;
    TR1 = 1; // enable timer1
    while(!TF1); // waiting for the timer1 overflow
    DHT11_DATA = 1;
    TR1 = 0; // disable timer1
    while(DHT11_DATA); // wait for dht11 response
    while(!DHT11_DATA); // 80us low
    while(DHT11_DATA); // 80us high
    dht11_data[0] = dht11_get_byte(); // relative humidity integer part
    dht11_data[1] = dht11_get_byte(); // relative humidity float part
    dht11_data[2] = dht11_get_byte(); // temperature integer part
    dht11_data[3] = dht11_get_byte(); // temperature float part
    dht11_data[4] = dht11_get_byte(); // checksum
    EA = 1;
}


void timer0_isr(void) interrupt 1 {
    // Setting up timer0 to count 50ms
    TH0 = T0_RELOAD_HIGH_50MS;
    TL0 = T0_RELOAD_LOW_50MS;
    /*
    if (dht11_tick_count > 5) {
        DHT11_READABLE = TRUE;
        dht11_tick_count = 0;
    }*/
    if (timer0_tick_count < 20) {
        DIG1_7SEG = !DIG1_7SEG;
        DIG2_7SEG = !DIG2_7SEG;
        timer0_tick_count++;
    }
    else {
        unsigned char i = 8;
        HC595_SER = ~HC595_SER;
        while(i--) {
            HC595_SRCLK = 1;
            HC595_SRCLK = 0;
        }
        HC595_RCLK = 1;
        HC595_RCLK = 0;

        timer0_tick_count = 0;
        dht11_tick_count++;
    }
}


void timer1_isr(void) interrupt 3 { BUZZER_SIG = !BUZZER_SIG; }

void main(void)
{
    // TMOD: Timer 1 = Mode 2 (8-bit auto), Timer 0 = Mode 1 (16-bit)
    TMOD = 0x21;

    // For 2kHz output (4kHz interrupt) at 12MHz:
    // 1MHz / 4000 = 250. Reload = 256 - 250 = 6.
    TH1 = 0x06;
    TL1 = 0x06; // Initial load
    
    // Setting up timer0 to count 50ms
    TH0 = T0_RELOAD_HIGH_50MS;
    TL0 = T0_RELOAD_LOW_50MS;

    HC595_SER = 0;
    HC595_RCLK = 0;
    HC595_SRCLR_N = 1;

    DIG1_7SEG = 1;
    DIG2_7SEG = 0;

    ET0 = 1; // enable timer0 interrupt
    
    ET1 = 1; // enable timer1 interrupt

    EA = 0; // disable global interruptions
    
    lcd_init(LCD16x2_SLAVE_ADDR);
    
    lcd_send_cmd(0x80, LCD16x2_SLAVE_ADDR); // Linha 1
    lcd_send_str(" < Entelechy >  ", LCD16x2_SLAVE_ADDR);
    
    lcd_send_cmd(0xC0, LCD16x2_SLAVE_ADDR); // Linha 2
    // lcd_send_str("Entelechy | ", LCD16x2_SLAVE_ADDR);
    
    dht11_get_raw_data();
    sprintf(buffer, "%bd", dht11_data[0]);
    lcd_send_str(buffer, LCD16x2_SLAVE_ADDR);
    lcd_send_str("hum e ", LCD16x2_SLAVE_ADDR);
    // lcd_send_str("%  ", LCD16x2_SLAVE_ADDR);
    sprintf(buffer, "%bd", dht11_data[2]);
    lcd_send_str(buffer, LCD16x2_SLAVE_ADDR);
    lcd_send_str("deg", LCD16x2_SLAVE_ADDR);
    
    EA = 1; // enable global interrupt
    TR0 = 1; //timer0 run
    TR1 = 1; // timer1 run

    P3 = 0xFF;
    P1 = 0x0F; // 0 parte baixa e f a parte alta
    P0 &= 0xFF;

    // polling for toggle switches and keys
    /*
        SW0 - Turn on the LED0
        SW1 - Turn on the LED1
        SW2 - Turn on the LED2
        SW3 - Turn on the Green RGB LED
        SW4 - Turn on All the Lights RGB LED
        
        Push the 4 buttons at the same time turn off all the lights
        
    */
    while(1) {
        if(IS_BUTTON_PRESSED(KEY1)) { TURN_ON_LED(LED0); }
        else { TURN_OFF_LED(LED0); }

        if(IS_BUTTON_PRESSED(KEY0)) { TURN_ON_LED(LED1); }
        else { TURN_OFF_LED(LED1); }
        
        if(IS_BUTTON_PRESSED(KEY2)) { TURN_ON_LED(LED2); }
        else { TURN_OFF_LED(LED2); }
        
        if(IS_BUTTON_PRESSED(KEY3)) { TURN_ON_LED(LED_RGB_BLUE); }
        else { TURN_OFF_LED(LED_RGB_BLUE); }
        
        if(IS_SWITCH_ON(SW0)) { TURN_ON_LED(LED_RGB_RED); }
        else { TURN_OFF_LED(LED_RGB_RED); }
        
        if(IS_SWITCH_ON(SW1)) { TURN_ON_LED(LED_RGB_GREEN); }
        else { TURN_OFF_LED(LED_RGB_GREEN); }
        /*
        if(IS_SWITCH_ON(SW1)) TURN_ON_LED(LED1);
        else TURN_OFF_LED(LED1);

        if(IS_SWITCH_ON(SW2)) TURN_ON_LED(LED2);
        else TURN_OFF_LED(LED2);

        if(IS_SWITCH_ON(SW3)) TURN_ON_LED(LED_RGB_GREEN);
        else TURN_OFF_LED(LED_RGB_GREEN);
        
        if(IS_SWITCH_ON(SW4)) {
            TURN_ON_LED(LED_RGB_RED);
            TURN_ON_LED(LED_RGB_GREEN);
            TURN_ON_LED(LED_RGB_BLUE);
        }
        
        if(IS_FOUR_PUSH_BUTTONS_PRESSED()) {
            TURN_OFF_LED(LED0);
            TURN_OFF_LED(LED1);
            TURN_OFF_LED(LED2);
            TURN_OFF_LED(LED_RGB_RED);
            TURN_OFF_LED(LED_RGB_GREEN);
            TURN_OFF_LED(LED_RGB_BLUE);
        }*/
    } // END WHILE
} // END main
