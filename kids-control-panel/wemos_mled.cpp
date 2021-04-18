#include "wemos_mled.h"

MLED::MLED(byte dataPin, byte clockPin)
{
    this->dataPin = dataPin;
    this->clockPin = clockPin;

    intensity = 7;
    pinMode(dataPin, OUTPUT);
    pinMode(clockPin, OUTPUT);

    digitalWrite(dataPin, HIGH);
    digitalWrite(clockPin, HIGH);
}

void MLED::display()
{
    for (uint8_t i = 0; i < 8; i++)
    {
        sendData(i, disBuffer[i]);

        digitalWrite(dataPin, LOW);
        digitalWrite(clockPin, LOW);
        digitalWrite(clockPin, HIGH);
        digitalWrite(dataPin, HIGH);
    }

    sendCommand(0x88 | (intensity));
}

void MLED::clear()
{
    for (uint8_t i = 0; i < 8; i++)
    {
        disBuffer[i] = 0x00;
    }
    MLED::display();
}

void MLED::dot(uint8_t x, uint8_t y, bool draw)
{
    x &= 0x07;
    y &= 0x07;

    if (draw)
    {
        disBuffer[y] |= (1 << x);
    }
    else
    {
        disBuffer[y] &= ~(1 << x);
    }
}

void MLED::setBrightness(uint8_t _intensity, bool on)
{
    if (_intensity > 7)
        _intensity = 7;
    MLED::intensity = _intensity;
}

void MLED::setSegments(uint8_t segments[], uint8_t length, uint8_t pos = 0)
{
    if (length > 8)
        length = 8;
    
    memcpy(segments, MLED::disBuffer, length);
    MLED::display();
}

void MLED::sendCommand(byte cmd)
{
    digitalWrite(dataPin, LOW);
    send(cmd);
    digitalWrite(dataPin, HIGH);
}

void MLED::sendData(byte address, byte data)
{
    sendCommand(0x44);
    digitalWrite(dataPin, LOW);
    send(0xC0 | address);
    send(data);
    digitalWrite(dataPin, HIGH);
}

void MLED::send(byte data)
{
    for (int i = 0; i < 8; i++)
    {
        digitalWrite(clockPin, LOW);
        digitalWrite(dataPin, data & 1 ? HIGH : LOW);
        data >>= 1;
        digitalWrite(clockPin, HIGH);
    }
}