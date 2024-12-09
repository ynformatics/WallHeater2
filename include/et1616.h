#pragma once
#include <Arduino.h>
#include <SPI.h>

class ET1616{
    //SPIClass* spi;
    public:
    ET1616( int8_t sck, int8_t mosi, int8_t ss)
    {
        //spi = new HSPI HSPIClass(0);
        //spi->end();
        SPI.begin(sck, -1, mosi, ss); 
        // pinMode(spi->pinSS(), OUTPUT);
        // digitalWrite(spi->pinSS(), HIGH); 
         pinMode(ss, OUTPUT);
        digitalWrite(ss, HIGH); 
    }

    void test()
    {
        SPI.beginTransaction(SPISettings(450000, LSBFIRST, SPI_MODE3));
        digitalWrite(SPI.pinSS(), LOW); 
        SPI.transfer(0x03); // 0X display mode: mode 3? not in datasheet. 0 == 4 digits 7 segments
        digitalWrite(SPI.pinSS(), HIGH); 

        digitalWrite(SPI.pinSS(), LOW); 
        SPI.transfer(0x8F); // 8X display control:  on, full brightness
        digitalWrite(SPI.pinSS(), HIGH); 

        digitalWrite(SPI.pinSS(), LOW); 
        SPI.transfer(0x40); //4X data settings: normal, autoinc, write data
        digitalWrite(SPI.pinSS(), HIGH); 


        digitalWrite(SPI.pinSS(), LOW); 
        SPI.transfer(0xC0); // CX address setting:  0

        SPI.transfer(0x80);
        SPI.transfer(0x01);
        SPI.transfer(0x00);
        SPI.transfer(0x00);
        SPI.transfer(0x57);
        SPI.transfer(0xAE);
        SPI.transfer(0x00);
       
        digitalWrite(SPI.pinSS(), HIGH); 
        SPI.endTransaction();
    }
    void setDisplayData(byte* segData, int length)
    {   
        SPI.beginTransaction(SPISettings(450000, LSBFIRST, SPI_MODE3));
        digitalWrite(SPI.pinSS(), LOW); 
        SPI.transfer(0xC0); // address 0, auto increment
        for(int i = 0; i < length; i++)
        {
            SPI.transfer(segData[i]);
            SPI.transfer(0x00); // padding
        }
        digitalWrite(SPI.pinSS(), HIGH); 
        SPI.endTransaction();
    }

    void setDisplayData(const char* hexString)
    {
        byte bytes[4];
        if(strlen(hexString) > 8)
            return;
        int i = 0;
        while(*hexString && hexString[1])
        {
            bytes[i++] = char2int(*hexString)*16 + char2int(hexString[1]);
            hexString += 2;
        }

        setDisplayData(bytes, 4);

    }
    int char2int(char input)
    {
        if(input >= '0' && input <= '9')
            return input - '0';
        if(input >= 'A' && input <= 'F')
            return input - 'A' + 10;
        if(input >= 'a' && input <= 'f')
            return input - 'a' + 10;
        throw std::invalid_argument("Invalid input string");
    }
    void setBrightness(uint8_t level) // 0 (off) to 8
    {
        if(level > 8)
         return;
        SPI.beginTransaction(SPISettings(450000, LSBFIRST, SPI_MODE3));
        digitalWrite(SPI.pinSS(), LOW); 

        if(level == 0)
            SPI.transfer(0x80); 
        else
            SPI.transfer(0x88 | (level - 1)); // 000 to 111
      
        digitalWrite(SPI.pinSS(), HIGH); 
        SPI.endTransaction();
    }
};