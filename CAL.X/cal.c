#define F_CPU 16000000

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>


#define HX_DT PD2
#define HX_SCK PD3


long tare;
long temp;
float temp_cal;
float avg_cal;
int ref_w = 167;
char out[8];

// Function declaration
float cal();
unsigned long Readcount(void);
void HX_RET();
void HX_INIT();
void I2C_init();
void I2C_start();
void I2C_stop();
void I2C_write(char data);
void LCD_cmd(int cmd);
void LCD_write(char wdata);
void LCD_init();
void LCD_wstr(const char *str);

int LCD_Address = 0x27;


int main(void)
{
    HX_INIT();
    I2C_init();
    LCD_init();
    avg_cal=0;
    LCD_wstr("  CALIBRATING  ");
    for(int i=1; i<6;i++)
    {
        temp_cal = cal();
        avg_cal+=temp_cal;
        char x[8];
        char y[4];
        sprintf(x, "%.3f", temp_cal);
        sprintf(y, "%d", i);
        LCD_cmd(0x01);
        LCD_wstr(y);
        LCD_wstr("-Scale Factor");
        LCD_cmd(0xC0);
        LCD_wstr(x);
        _delay_ms(5000);
    }
    avg_cal/=5;
    sprintf(out, "%.3f", avg_cal);
    LCD_cmd(0x01);
    LCD_wstr("Avg Scale Factor");
    LCD_cmd(0xC0);
    LCD_wstr(out);
    
    return 0;
}


// Function definition
unsigned long Readcount(void)
{
    unsigned long count;
    unsigned char i;
    PORTD |= (1 << HX_DT);
    PORTD &= (~(1 << HX_SCK));
    count = 0;
    while( (PIND & (1 << HX_DT)) );
    for(i=0; i<24; i++)
    {
        PORTD |= (1 << HX_SCK);
        count = count << 1;
        PORTD &= (~(1 << HX_SCK));
        if( (PIND & (1 << PD2)) )
            count++;
    }
    PORTD |= (1 << HX_SCK);
    count = count ^ 0x800000;
    PORTD &= (~(1 << HX_SCK));
    return(count);
}


void HX_INIT()
{
    DDRD &= ~(1 << HX_DT);
    DDRD |= (1 << HX_SCK);
    PORTD &= ~(1 << HX_SCK);
    _delay_ms(100);
    
}


void HX_RET()
{
    PORTD &= (~(1 << HX_SCK));
    _delay_ms(2);
    PORTD |=(1<<HX_SCK);
    _delay_ms(2);
    PORTD &= (~(1 << HX_SCK));
}


void I2C_init()
{
    TWSR = 0x00;
    TWCR = (1 << TWEN);
    TWBR = ((F_CPU / 100000)-16) / 2;
}


void I2C_start()
{
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);
    while ( (TWCR & (1 << TWINT)) == 0 );
}


void I2C_stop()
{
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}


void I2C_write(char data)
{
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while ( (TWCR & (1 << TWINT)) == 0 );
}


void LCD_cmd(int cmd)
{
    int temp, tcmd;
    temp = (cmd & 0xF0);
    tcmd = (temp | 0x08) & (~0x01);
    I2C_start();
    I2C_write(LCD_Address << 1);
    I2C_write(tcmd | 0x04);
    _delay_ms(2);
    I2C_write(tcmd & ~0x04);
    I2C_stop();

    temp = (cmd << 4);
    tcmd = (temp | 0x08) & (~0x01);
    I2C_start();
    I2C_write(LCD_Address << 1);
    I2C_write(tcmd | 0x04);
    _delay_ms(2);
    I2C_write(tcmd & ~0x04);
    I2C_stop();
}


void LCD_init()
{
    LCD_cmd(0x02);
    LCD_cmd(0x28);
    LCD_cmd(0x0C);
    LCD_cmd(0x01);
    LCD_cmd(0x80);
}


void LCD_write(char wdata)
{
    char temp2, tdata;
    temp2 = (wdata & 0xF0);
    tdata = temp2 | 0x08 | 0x01;
    I2C_start();
    I2C_write(LCD_Address << 1);
    I2C_write(tdata | 0x04);
    _delay_ms(2);
    I2C_write(tdata & ~0x04);
    I2C_stop();
    
    temp2 = (wdata << 4);
    tdata = temp2 | 0x08 | 0x01;
    I2C_start();
    I2C_write(LCD_Address << 1);
    I2C_write(tdata | 0x04);
    _delay_ms(2);
    I2C_write(tdata & ~0x04);
    I2C_stop();
}


void LCD_wstr(const char *str)
{
    while(*str !='\0')
    {
        LCD_write(*str);
        str++;
    }
}



float cal()
{
  float cal_val;
  LCD_cmd(0x01);
  LCD_wstr("Remove weight");
  _delay_ms(5000);
  LCD_cmd(0x01);
  LCD_wstr("     ------     ");
  
  temp=0;
  tare=0;
  for(int i=0; i<100; i++)
  {
    temp=Readcount();
    tare+= temp;
  }
  tare/=100;
  
  LCD_cmd(0x01);
  LCD_wstr("Place 167g");
  _delay_ms(5000);
  LCD_cmd(0x01);
  LCD_wstr("     ------     ");
  
  temp=0;
  cal_val=0;
  for(int i=0; i<100; i++)
  {
    temp=Readcount();
    cal_val+= (tare-temp);
  }
  cal_val/=100;
  cal_val/=ref_w;
  
  return cal_val;
}
