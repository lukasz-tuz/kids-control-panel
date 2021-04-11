#include <Encoder.h>
#include <Adafruit_MCP23017.h>

class PanelEncoder : Encoder
{
public:
    PanelEncoder(Adafruit_MCP23017 *gpio, uint8_t pin1, uint8_t pin2)
        : Encoder(pin1, pin2)
    {
        PanelEncoder::gpio = gpio;
    }

private:
    Adafruit_MCP23017 *gpio = NULL;

    void pinMode(byte pinNum, byte mode)
    {
        byte mode_internal = INPUT;

        switch (mode)
        {
        case OUTPUT:
            mode_internal = OUTPUT;
            break;
        case INPUT:
        case INPUT_PULLUP:
            mode_internal = INPUT;
            break;
        default:
            mode_internal = INPUT;
            break;
        }
        if (PanelEncoder::gpio != NULL)
        {
            PanelEncoder::gpio->pinMode(pinNum, mode_internal);
            if (mode == INPUT_PULLUP)
                PanelEncoder::gpio->pullUp(pinNum, 1);
            else
                PanelEncoder::gpio->pullUp(pinNum, 0);
        }
    }

    void digitalWrite(byte pinNum, boolean level)
    {
        if (PanelEncoder::gpio != NULL)
        {
            PanelEncoder::gpio->digitalWrite(pinNum, level);
        }
    }

    int digitalRead(byte pinNum)
    {
        if (PanelEncoder::gpio != NULL)
        {
            return PanelEncoder::gpio->digitalRead(pinNum);
        }
        else
        {
            return -1;
        }
    }
};