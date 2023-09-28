/////*
///  * File:   main.c
///  * Author: VISHNU C S
///  * Program to measure weight using Atmega-328p and HX711 Load cell amplifier
///  * Created on 24 September, 2023, 4:01 PM
///  */

 //-----------------------------------------------------------------------------
#define F_CPU 16000000UL
// Frequency of Microcintroller Ocsillators

#include <avr/io.h>
#include <util/delay.h>  // To set delay
#include <stdio.h>  // To use " sprintf "

#define HX_DT PD2
// Define Port D2 as HX_DT (o/p of HX711)
#define HX_SCK PD3
// Define Port D3 as HX_SCK (clock of HX711)
//------------------------------------------------------------------------------

float weight;  // To store weight in grams                     
long tare;  // To store tare/offset value
long read_data;  // To store data read from HX711
const float cal_val= -460.451; // Scale factor of Load cell
char W[5];  // To store converted weight in char type (For printing in LCD)
int LCD_Address = 0x27;  // I2C slave address

// Function declaration
unsigned long Readcount(void);  // For fetching data from HX711
void HX_INIT();  // To initialize HX711  
void HX_RET();  // To Reset HX711
void cal();  // To calibrate Tare
void I2C_init();  // To initialize I2C Communication
void I2C_start();  // To start I2C Communication
void I2C_stop();  // To stop I2C Communication
void I2C_write(char data); // To assign data to the data register and to transmit the data.
void LCD_cmd(int cmd);  // To send command to the LCD
void LCD_write(char wdata);  // To send single character to LCD
void LCD_init();  // TO initialize LCD
void LCD_wstr(const char *str);  // To write string to a LCD
//------------------------------------------------------------------------------

// Main Function
int main(void)
{
    HX_INIT();  // Initialize HX711
    I2C_init();  // Initialize I2C communication
    LCD_init();  // Initialize LCD
    cal();  // Calibrate tare
    while (1)
    {
        read_data = Readcount(); // read data from HX711 
        weight=( ((read_data - tare) / cal_val) - 2*( (read_data - tare) / cal_val) );  // convert HX711 data into Weight in grams
        LCD_cmd(0x01);  // Clear LCD
        LCD_wstr("WEIGHT: ");  // Print " WEIGHT " in LCD
        if( (weight > -5) & (weight < 2) )
            LCD_wstr("0.00g     ");
        else if(weight < 1000)   // To print in grams if weight<1000
        {
            sprintf(W, "%.2f", weight); // To store Weight in to Char type in variable 'W'
            LCD_wstr(W);
            LCD_wstr("g     ");
        }
        else if(weight > 999)  // To print weight in "Kg" if weight >= 1000
        {
            weight/=1000;
            sprintf(W, "%.2f", weight);
            LCD_wstr(W);
            LCD_wstr("Kg     ");
        }
        HX_RET();  // Reset HX711 to fetch new data
    }
}
//------------------------------------------------------------------------------

// Function definition
// Function for fetching data from HX711
unsigned long Readcount(void)
{
    unsigned long count;
    unsigned char i;
    PORTD |= (1 << HX_DT); // Set Port D2
    PORTD &= (~(1 << HX_SCK)); // Clear Port D3 
    count = 0; // Initialize the count variable to zero.
    while( (PIND & (1 << HX_DT)) ); // check bit HX_DT of  Port D in the PIND register to go low (END OF CONVERSION)
    for(i=0; i<24; i++) // CLOCK to read 24 bits of data from the sensor:
    {
        PORTD |= (1 << HX_SCK); // Set the bit HX_SCK in the PORTD register to high to signal the sensor to provide the next bit.
        count = count << 1; // Shift the current value of count one bit to the left to make space for the next bit.
        PORTD &= (~(1 << HX_SCK)); // Clear the HX_SCK bit to signal that data should be read.
        if( (PIND & (1 << PD2)) ) // Check the value of a bit (PD2) in the PIND register and if the bit is high, count++
            count++;
    }
    PORTD |= (1 << HX_SCK); // Set the HX_SCK bit
    count = count ^ 0x800000; // XOR the count with 0x800000
    PORTD &= (~(1 << HX_SCK)); // Clear the HX_SCK bit
    return(count); // Return the final count value
}
//------------------------------------------------------------------------------

// Function to initialize HX711
void HX_INIT()
{
    DDRD &= ~(1 << HX_DT);  // Set Port D2 as input (HX711 data line)
    DDRD |= (1 << HX_SCK);  // Set Port D3 as output (HX711 clock line)
    PORTD &= ~(1 << HX_SCK); // Set HX711 clock line to low(Normal Working Mode)
    _delay_ms(100);
    
}
//------------------------------------------------------------------------------

// Function to RESET HX711
void HX_RET() // TO reset HX711 apply low to high clk and stay high for more than 60us to POWER DOWN
{
    PORTD &= (~(1 << HX_SCK)); // Set HX711 clk to low
    _delay_ms(2);
    PORTD |=(1<<HX_SCK); // Set HX711 clk to hight
    _delay_ms(2);   
    PORTD &= (~(1 << HX_SCK));  // Send low to HX711 SCK to POWER ON
}
//------------------------------------------------------------------------------

// Function to calibrate tare/offset
void cal()
{
    LCD_wstr("  CALIBRATING  ");
    _delay_ms(1500);
    LCD_cmd(0x01);
    LCD_wstr(" REMOVE  WEIGHT ");
    _delay_ms(3000);
    LCD_cmd(0x01);
    LCD_wstr("  PLEASE  WAIT  ");
    long temp = 0;
    tare = 0;
    for(int i=0; i<100; i++) // For calibrating tare (calculated for 100 times)
    {
        temp = Readcount(); // HX711 data without any weight on LOAD cell
        tare += temp;
    }
    tare /= 100;  // Average of calculated tare
    LCD_cmd(0x01);
    LCD_wstr("    FINISHED    ");
    _delay_ms(2000);
    LCD_cmd(0x01);
}
//------------------------------------------------------------------------------

// Function to initialize I2C Communication
void I2C_init() 
{
    // TWSR (Two wire Status Register)
    // TWPS1 = 0 = TWSP0 = 0.Sets the prescaler value to 1
    TWSR = 0x00;
    // Two Wire Control Register
    // TWEN: TWI Enable Bit.The TWEN bit enables TWI operation and activates the TWI interface.
    TWCR = (1 << TWEN);
    // TWI Bit Rate Register
    // Set bit rate register for 100 kHz
    // SCL Frequency =  CPU Clock Frequency/16+2(TWBR) X (Prescaler Value)
    TWBR = ((F_CPU / 100000)-16) / 2;
}
//------------------------------------------------------------------------------

// Function to start I2C Communication
void I2C_start()
{
    // TWCR - (Two wire control register)
    // TWSTA: TWI START Condition Bit
    // TWEN: TWI Enable Bit
    // TWINT: TWI Interrupt Flag (1 = Clear the interrupt flag to initiate operation of the TWI module.)
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTA);
    while ( (TWCR & (1 << TWINT)) == 0 );
}
//------------------------------------------------------------------------------

// Function to stop I2C Communication
void I2C_stop()
{
    //Bit 4_TWSTO: TWI STOP Condition Bit.
    //Bit 7_TWINT: TWI Interrupt Flag.
    //Bit 2_TWEN: TWI Enable Bit.TWEN bit enables TWI operation and activates the TWI interface.
    TWCR = (1 << TWINT) | (1 << TWEN) | (1 << TWSTO);
}
//------------------------------------------------------------------------------

// Function to assign data to the data register and to transmit the data.
void I2C_write(char data)
{
    //TWDR: TWI Data Register
    // TWCR: TWI Control Register
    // Bit 7: TWINT: TWI Interrupt Flag.
    // Bit 2: TWEN: TWI Enable Bit.TWEN bit enables TWI operation and activates the TWI interface
    TWDR = data;
    TWCR = (1 << TWINT) | (1 << TWEN);
    while ( (TWCR & (1 << TWINT)) == 0 );
}
//------------------------------------------------------------------------------

// Function to send command to LCD
void LCD_cmd(int cmd)
{
    int temp, tcmd;
    temp = (cmd & 0xF0);  // Masking 4bit MSB
    tcmd = (temp | 0x08) & (~0x01);  // Backlight ON(P3 =1) and Register Select (RS) = 0 for command mode.
    I2C_start();
    I2C_write(LCD_Address << 1);
    I2C_write(tcmd | 0x04);  // EN(Enable bit, P2) = 1.
    _delay_ms(2);
    I2C_write(tcmd & ~0x04);  //EN(Enable bit, P2) = 0.
    I2C_stop();

    temp = (cmd << 4);  //Shifting the 4 LSB bit to MSB
    tcmd = (temp | 0x08) & (~0x01);  // Backlight ON(P3 =1) and Register Select (RS) = 0 for command mode.
    I2C_start();
    I2C_write(LCD_Address << 1);
    I2C_write(tcmd | 0x04);  //EN(Enable bit, P2) = 1.
    _delay_ms(2);
    I2C_write(tcmd & ~0x04);  //EN(Enable bit, P2) = 0.
    I2C_stop();
}
//------------------------------------------------------------------------------

// Function to send initialize LCD
void LCD_init()
{
    LCD_cmd(0x02);  // To Return Home In LCD
    LCD_cmd(0x28);  // To set LCD in 4-bit MODE
    LCD_cmd(0x0C);  // Display ON, Cursor OFF
    LCD_cmd(0x01);  // To Clear LCD
    LCD_cmd(0x80);  // To Force cursor to beginning of first line
}
//------------------------------------------------------------------------------

// Function to send single character to LCD
void LCD_write(char wdata)
{
    char temp2, tdata;
    temp2 = (wdata & 0xF0);  //Masking the MSB 4 bits
    tdata = temp2 | 0x08 | 0x01;  //Backlight ON(P3 =1) and Register Select (RS) = 1 for data mode.
    I2C_start();
    I2C_write(LCD_Address << 1);
    I2C_write(tdata | 0x04);  //EN(Enable bit, P2) = 1.
    _delay_ms(2);
    I2C_write(tdata & ~0x04);  //EN(Enable bit, P2) = 0.
    I2C_stop();
    
    temp2 = (wdata << 4);   //Shifting the 4 LSB bit to MSB.
    tdata = temp2 | 0x08 | 0x01;  //Backlight ON(P3 =1) and Register Select (RS) = 1 for data mode.
    I2C_start();
    I2C_write(LCD_Address << 1);
    I2C_write(tdata | 0x04);  //EN(Enable bit, P2) = 1.
    _delay_ms(2);
    I2C_write(tdata & ~0x04);  //EN(Enable bit, P2) = 1.
    I2C_stop();
}
//------------------------------------------------------------------------------

// Function to send string to LCD
void LCD_wstr(const char *str)
{
    while(*str !='\0')
    {
        LCD_write(*str);
        str++;
    }
}
//------------------------------------------------------------------------------
