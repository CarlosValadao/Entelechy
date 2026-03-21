#include "lcd16x2_i2c.h"

void i2c_start(void)
{
    LCD16x2_SDA = 1;
    _nop_(); _nop_();
    LCD16x2_SCL = 1;
    _nop_(); _nop_();
    LCD16x2_SDA = 0;
    _nop_(); _nop_();
}

void i2c_stop(void)
{
    LCD16x2_SCL = 0;
    LCD16x2_SDA = 0;
    LCD16x2_SCL = 1;
    LCD16x2_SDA = 1;
}

void i2c_ACK(void)
{
    LCD16x2_SCL = 0;
    LCD16x2_SDA = 1;
    LCD16x2_SCL = 1;
    while(LCD16x2_SDA); // Aguarda o escravo puxar LCD16x2_SDA para baixo (ACK)
}

void i2c_write(unsigned char dat)
{
    unsigned char i;
    for(i = 0; i < 8; i++)
    {
        LCD16x2_SCL = 0;
        // Verifica bit a bit, do MSB para o LSB
        LCD16x2_SDA = (dat & (0x80 >> i)) ? 1 : 0;
        LCD16x2_SCL = 1;
    }
}

void lcd_send_cmd(unsigned char cmd, unsigned char lcd_addr)
{
    unsigned char cmd_l, cmd_u;
    cmd_l = (cmd << 4) & 0xf0;
    cmd_u = (cmd & 0xf0);

    i2c_start();            
    i2c_write(lcd_addr);
    i2c_ACK();
    
    // Envia parte alta (Upper nibble)
    i2c_write(cmd_u | 0x0C); // BL EN RW RS -> 1 1 0 0
    i2c_ACK();
    delay_ms(1);
    i2c_write(cmd_u | 0x08); // Pulse Enable: 1 0 0 0
    i2c_ACK();
    delay_ms(10);
    
    // Envia parte baixa (Lower nibble)
    i2c_write(cmd_l | 0x0C); // 1 1 0 0
    i2c_ACK();
    delay_ms(1);
    i2c_write(cmd_l | 0x08);
    i2c_ACK();
    delay_ms(10);
    
    i2c_stop();
}

void lcd_send_data(unsigned char dataw, unsigned char lcd_addr)
{
    unsigned char dataw_l, dataw_u;
    dataw_l = (dataw << 4) & 0xf0;
    dataw_u = (dataw & 0xf0);

    i2c_start();
    i2c_write(lcd_addr);
    i2c_ACK();
    
    // Envia parte alta
    i2c_write(dataw_u | 0x0D); // BL EN RW RS -> 1 1 0 1
    i2c_ACK();
    delay_ms(1);
    i2c_write(dataw_u | 0x09); // 1 0 0 1
    i2c_ACK();
    delay_ms(10);
    
    // Envia parte baixa
    i2c_write(dataw_l | 0x0D);
    i2c_ACK();
    delay_ms(1);
    i2c_write(dataw_l | 0x09);
    i2c_ACK();
    delay_ms(10);
    
    i2c_stop();
}

void lcd_send_str(unsigned char *p, unsigned char lcd_addr)
{
    while(*p != '\0')
    {
        lcd_send_data(*p++, lcd_addr);
    }
}

void delay_ms(unsigned char n)
{
    unsigned char m;
    for(; n > 0; n--)
    {
        for(m = 121; m > 0; m--);
        _nop_(); _nop_();
        _nop_(); _nop_();
        _nop_(); _nop_();
    }
}

void lcd_init(unsigned char lcd_addr)
{
    lcd_send_cmd(0x02, lcd_addr); // Return home
    lcd_send_cmd(0x28, lcd_addr); // 4 bit mode
    lcd_send_cmd(0x0C, lcd_addr); // Display On, cursor off
    lcd_send_cmd(0x06, lcd_addr); // Increment Cursor (shift cursor to right)
    lcd_send_cmd(0x01, lcd_addr); // Clear display
}

void lcd_clear_display(unsigned char lcd_addr) {
    lcd_send_cmd(0x01, lcd_addr);
}


// Carrega um caractere customizado na CGRAM
// slot: posição 0 a 7
// pattern: array de 8 bytes (linhas do caractere)
void lcd_create_char(unsigned char slot, unsigned char code *pattern, unsigned char lcd_addr)
{
    unsigned char i;
    // Endereço da CGRAM = 0x40 | (slot * 8)
    lcd_send_cmd(0x40 | (slot << 3), lcd_addr);
    for(i = 0; i < 8; i++)
    {
        lcd_send_data(pattern[i], lcd_addr);
    }
}