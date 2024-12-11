#pragma once
#include <Arduino.h>
#include <SPI.h>

class ET1616{
    uint8_t brightness = 0x8F;
    uint8_t data[4] = {0,0,0,0};
    
    public:
    ET1616( int8_t sck, int8_t mosi, int8_t ss){
        SPI.begin(sck, -1, mosi, ss); 
        pinMode(SPI.pinSS(), OUTPUT);
        digitalWrite(SPI.pinSS(), HIGH);     
    }

    void setBytes() {   
        SPI.beginTransaction(SPISettings(450000, LSBFIRST, SPI_MODE3));

        digitalWrite(SPI.pinSS(), LOW); 
        SPI.transfer(0x00); // 0X display mode: mode 3? not in datasheet. 0 == 4 digits 7 segments
        digitalWrite(SPI.pinSS(), HIGH); 

        digitalWrite(SPI.pinSS(), LOW); 
        SPI.transfer(brightness); // 8X display control:  on, full brightness
        digitalWrite(SPI.pinSS(), HIGH); 

        digitalWrite(SPI.pinSS(), LOW); 
        SPI.transfer(0x40); //4X data settings: normal, autoinc, write data
        digitalWrite(SPI.pinSS(), HIGH); 

        digitalWrite(SPI.pinSS(), LOW); 
        SPI.transfer(0xC0); // start address 0
        for(int i = 0; i < 4; i++){
            SPI.transfer(data[i]);
            SPI.transfer(0x00); // padding
        }
        digitalWrite(SPI.pinSS(), HIGH); 

        SPI.endTransaction();
    }

    void setHex(const char* hexString){
        byte bytes[4];
        if(strlen(hexString) > 8)
            return;
        int i = 0;
        while(*hexString && hexString[1]){
            data[i++] = char2int(*hexString)*16 + char2int(hexString[1]);
            hexString += 2;
        }

        setBytes();
    }

    int char2int(char input){
        if(input >= '0' && input <= '9')
            return input - '0';
        if(input >= 'A' && input <= 'F')
            return input - 'A' + 10;
        if(input >= 'a' && input <= 'f')
            return input - 'a' + 10;
        throw std::invalid_argument("Invalid input string");
    }

    void setBrightness(uint8_t level){ // 0 (off) to 8
        if(level > 8)
            level = 8;

        brightness = level == 0 ? 0x80 : 0x88 | (level - 1);    

        setBytes();
    }

    void setNumber(uint8_t val){
        const byte segBytes[] = {0x7E, 0x18, 0x6D, 0x5D, 0x1B, 0x57, 0x77, 0x1C, 0x7F, 0x5F};

        if(val > 99){
            data[3] = 0; data[2] = 0; // blank
            setBytes();
            return;
        }
        uint8_t unit = val % 10;
        uint8_t tens = val / 10;
        data[3] = segBytes[unit];
        data[2] = tens == 0 ? 0x00 : segBytes[tens];

        setBytes();
    }

    void setLeds(const char* leds){
        data[1] = 0;

        while(char ch = *leds++)                                                                                                                
            data[1] |= 0x01 << ch - '0';                                                                                              
        
        setBytes();
    }
};