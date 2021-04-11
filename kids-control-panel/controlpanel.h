#include <RGB_LED.h>
#include "controlpanel_keypad.h"
#include "controlpanel_encoder.h"

namespace controlpanel
{
    // For debugging
    void _scanI2C()
    {
        uint8_t nDevices = 0;

        for (byte address = 1; address < 127; ++address)
        {
            // The i2c_scanner uses the return value of
            // the Write.endTransmisstion to see if
            // a device did acknowledge to the address.
            Wire.beginTransmission(address);
            byte error = Wire.endTransmission();

            if (error == 0)
            {
                Serial.print("I2C device found at address 0x");
                if (address < 16)
                {
                    Serial.print("0");
                }
                Serial.print(address, HEX);
                Serial.println("  !");

                ++nDevices;
            }
            else if (error == 4)
            {
                Serial.print("Unknown error at address 0x");
                if (address < 16)
                {
                    Serial.print("0");
                }
                Serial.println(address, HEX);
            }
        }
        if (nDevices == 0)
        {
            Serial.println("No I2C devices found\n");
        }
        else
        {
            Serial.println("done\n");
        }
    }

    typedef enum LedSegments
    {
        a = 0,
        b = 1,
        c = 2,
        d = 3,
        e = 4,
        f = 5,
        g = 6,
        dpx = 7
    };

    class LedBar
    {
    public:
        typedef enum ledColor
        {
            blue = 0,
            orange,
            green,
            yellow,
            max_color
        };

        explicit LedBar();
        explicit LedBar(Adafruit_MCP23017 *gpios)
        {
            LedBar::gpios = gpios;
        }

        void begin(uint8_t y, uint8_t o, uint8_t g, uint8_t b)
        {
            LedBar::pins[yellow] = y;
            LedBar::pins[orange] = o;
            LedBar::pins[green] = g;
            LedBar::pins[blue] = b;

            for (uint8_t nPin = 0; nPin < LedBar::numPins; nPin++)
            {
                LedBar::pinMode(LedBar::pins[nPin], OUTPUT);
                LedBar::digitalWrite(LedBar::pins[nPin], 0);
            }
        }

        void off()
        {
            for (uint8_t nPin = 0; nPin < LedBar::numPins; nPin++)
            {
                LedBar::digitalWrite(LedBar::pins[nPin], 0);
            }
        }

        void color(uint8_t c, bool on)
        {
            if (c < max_color)
            {
                LedBar::digitalWrite(LedBar::pins[c], (uint8_t)on);
            }
        }

    private:
        Adafruit_MCP23017 *gpios = NULL;
        static const uint8_t numPins = 4;
        uint8_t pins[numPins] = {0};

        void pinMode(uint8_t pin, uint8_t d)
        {
            if (LedBar::gpios != NULL)
            {
                gpios->pinMode(pin, d);
            }
            else
            {
                pinMode(pin, d);
            }
        }

        void digitalWrite(uint8_t pin, uint8_t d)
        {
            if (LedBar::gpios != NULL)
            {
                gpios->digitalWrite(pin, d);
            }
            else
            {
                digitalWrite(pin, d);
            }
        }
    };

} // namespace controlpanel
