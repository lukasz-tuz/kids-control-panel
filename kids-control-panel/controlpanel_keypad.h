#include <Key.h>
#include <Keypad.h>
#include <Adafruit_MCP23017.h>

class PanelKeypad : public Keypad
{
public:
    PanelKeypad(Adafruit_MCP23017 *gpio, char *userKeymap, byte *row, byte *col, byte numRows, byte numCols)
        : Keypad(userKeymap, row, col, numRows, numCols)
    {
        PanelKeypad::gpio = gpio;
    }

    void pin_mode(byte pinNum, byte mode)
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

        if (PanelKeypad::gpio != NULL)
        {
            PanelKeypad::gpio->pinMode(pinNum, mode_internal);
            if (mode == INPUT_PULLUP)
                PanelKeypad::gpio->pullUp(pinNum, 1);
            else
                PanelKeypad::gpio->pullUp(pinNum, 0);
        }
    }

    void pin_write(byte pinNum, boolean level)
    {
        if (PanelKeypad::gpio != NULL)
        {
            PanelKeypad::gpio->digitalWrite(pinNum, level);
        }
    }

    int pin_read(byte pinNum)
    {
        if (PanelKeypad::gpio != NULL)
        {
            return PanelKeypad::gpio->digitalRead(pinNum);
        }
        else
        {
            return -1;
        }
    }

private:
    Adafruit_MCP23017 *gpio = NULL;
};