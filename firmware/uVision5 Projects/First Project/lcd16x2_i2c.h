#ifndef LCD16X2_I2C_H
#define LCD16X2_I2C_H

#include <reg51.h>
#include <intrins.h>

sbit LCD16x2_SCL = P0^6;
sbit LCD16x2_SDA = P0^7;

void i2c_start(void);
void i2c_stop(void);
void i2c_ACK(void);
void i2c_write(unsigned char);
void i2c_DevWrite(unsigned char);

void lcd_init(unsigned char);
void lcd_send_cmd(unsigned char, unsigned char);
void lcd_send_data(unsigned char, unsigned char);
void lcd_send_str(unsigned char *, unsigned char);

void lcd_clear_display(unsigned char);
void lcd_create_char(unsigned char, unsigned char code*, unsigned char);


void delay_ms(unsigned char);

#endif //end LCD_I2C_H