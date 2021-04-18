
class LedBar
{
public:
    typedef enum ledColor
    {
        blue = 0,
        orange,
        red,
        yellow,
        max_color
    };

    explicit LedBar(uint8_t y, uint8_t o, uint8_t r, uint8_t b)
    {
        begin(y, o, r, b);
    }
    explicit LedBar(Adafruit_MCP23017 *gpios, uint8_t y, uint8_t o, uint8_t r, uint8_t b)
    {
        LedBar::gpios = gpios;
        begin(y, o, r, b);
    }

    void begin(uint8_t y, uint8_t o, uint8_t r, uint8_t b)
    {
        LedBar::pins[yellow] = y;
        LedBar::pins[orange] = o;
        LedBar::pins[red] = r;
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

    void move_bar(int direction)
    {
        // make sure that direction is either 1 or -1
        direction = direction / abs(direction);
        LedBar::leds[led_ptr] = !LedBar::leds[led_ptr];
        LedBar::color(led_ptr, LedBar::leds[led_ptr]);
        if (direction > 0)
        {
            ++led_ptr %= max_color;
        }
        else if (direction < 0)
        {
            if (led_ptr == 0)
                led_ptr = max_color;
            led_ptr--;
        }
    }

private:
    Adafruit_MCP23017 *gpios = NULL;
    static const uint8_t numPins = 4;
    uint8_t pins[numPins] = {0};
    bool leds[numPins] = {0};

    uint8_t led_ptr = 0;

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