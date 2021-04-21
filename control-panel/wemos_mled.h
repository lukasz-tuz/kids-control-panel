#ifndef __WEMOS_Matrix_LED_H
#define __WEMOS_Matrix_LED_H


#if ARDUINO >= 100
 #include "Arduino.h"
#else
 #include "WProgram.h"
#endif

class MLED
{
    public:
        MLED(byte clockPin=5, byte dataPin=7);
        void display();
        void clear();
        void dot(uint8_t x, uint8_t y, bool draw=1);
        void setBrightness(uint8_t intensity, bool on);
        void setSegments(uint8_t segments[], uint8_t length, uint8_t pos = 0);
        
        uint8_t intensity;
        

    protected:
        void sendCommand(byte led);
        void sendData(byte add, byte data);
        void send(byte data);

        uint8_t disBuffer[8]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        byte dataPin;
        byte clockPin;

};


#endif
