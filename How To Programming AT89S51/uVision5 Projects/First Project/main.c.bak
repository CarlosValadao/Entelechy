#include <reg51.h>
#include <intrins.h>

// Protótipos de Funções
void i2c_start(void);
void i2c_stop(void);
void i2c_ACK(void);
void i2c_write(unsigned char);
void i2c_DevWrite(unsigned char);
void lcd_send_cmd(unsigned char);
void lcd_send_data(unsigned char);
void lcd_send_str(unsigned char *);
void lcd_slave(unsigned char);
void delay_ms(unsigned int);

// Variáveis Globais
unsigned char slave1 = 0x4E;
unsigned char slave_add;

// Definição de Pinos
sbit scl = P0^6;
sbit sda = P0^7;

void i2c_start(void)
{
    sda = 1;
    _nop_(); _nop_();
    scl = 1;
    _nop_(); _nop_();
    sda = 0;
    _nop_(); _nop_();
}

void i2c_stop(void)
{
    scl = 0;
    sda = 0;
    scl = 1;
    sda = 1;
}

void lcd_slave(unsigned char slave)
{
    slave_add = slave;
}

void i2c_ACK(void)
{
    scl = 0;
    sda = 1;
    scl = 1;
    while(sda); // Aguarda o escravo puxar SDA para baixo (ACK)
}

void i2c_write(unsigned char dat)
{
    unsigned char i;
    for(i = 0; i < 8; i++)
    {
        scl = 0;
        // Verifica bit a bit, do MSB para o LSB
        sda = (dat & (0x80 >> i)) ? 1 : 0;
        scl = 1;
    }
}

void lcd_send_cmd(unsigned char cmd)
{
    unsigned char cmd_l, cmd_u;
    cmd_l = (cmd << 4) & 0xf0;
    cmd_u = (cmd & 0xf0);

    i2c_start();            
    i2c_write(slave_add);
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

void lcd_send_data(unsigned char dataw)
{
    unsigned char dataw_l, dataw_u;
    dataw_l = (dataw << 4) & 0xf0;
    dataw_u = (dataw & 0xf0);

    i2c_start();
    i2c_write(slave_add);
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

void lcd_send_str(unsigned char *p)
{
    while(*p != '\0')
    {
        lcd_send_data(*p++);
    }
}

void delay_ms(unsigned int n)
{
    unsigned int m;
    for(; n > 0; n--)
    {
        for(m = 121; m > 0; m--);
        _nop_(); _nop_();
        _nop_(); _nop_();
        _nop_(); _nop_();
    }
}

void lcd_init()
{
    lcd_send_cmd(0x02); // Return home
    lcd_send_cmd(0x28); // 4 bit mode
    lcd_send_cmd(0x0C); // Display On, cursor off
    lcd_send_cmd(0x06); // Increment Cursor (shift cursor to right)
    lcd_send_cmd(0x01); // Clear display
}

void main(void)
{
    lcd_slave(slave1);
    lcd_init();
    
    lcd_send_cmd(0x80); // Linha 1
    lcd_send_str("      UEFS       ");
    
    lcd_send_cmd(0xC0); // Linha 2
    lcd_send_str("Entelechy | LEDS");

    while(1)
    {
        // Loop infinito
    }
}