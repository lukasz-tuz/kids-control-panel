class LedBar
{
public:
    explicit LedBar(uint8_t y, uint8_t o, uint8_t r, uint8_t b)
    {
        begin(y, o, r, b);
    }

    void begin(uint8_t y, uint8_t o, uint8_t r, uint8_t b)
    {
        LedBar::pins[0] = y;
        LedBar::pins[1] = o;
        LedBar::pins[2] = r;
        LedBar::pins[3] = b;

        for (uint8_t nPin = 0; nPin < LedBar::numColors; nPin++)
        {
            pinMode(LedBar::pins[nPin], OUTPUT);
            digitalWrite(LedBar::pins[nPin], 0);
        }
    }

    /**
     * @brief When sharing physical pins with other devices
     * this method needs to be called to restore pin modes
     * required by Led bar.
     */
    void reset_pins()
    {
        for (uint8_t nPin = 0; nPin < LedBar::numColors; nPin++)
        {
            pinMode(LedBar::pins[nPin], OUTPUT);
            digitalWrite(LedBar::pins[nPin], LedBar::leds[nPin]);
        }
    }

    void off()
    {
        for (uint8_t nPin = 0; nPin < LedBar::numColors; nPin++)
        {
            LedBar::leds[nPin] = false;
            digitalWrite(LedBar::pins[nPin], 0);
        }
    }

    void color(uint8_t c, bool on)
    {
        if (c < numColors)
        {
            LedBar::leds[c] = on;
            digitalWrite(LedBar::pins[c], (uint8_t)on);
        }
    }

    void move_bar(int direction)
    {
        // make sure that direction is either 1 or -1
        if (direction != 0)
        {
            direction = direction / abs(direction);
            LedBar::leds[led_ptr] = !LedBar::leds[led_ptr];
            LedBar::color(led_ptr, LedBar::leds[led_ptr]);
            if (direction > 0)
            {
                ++led_ptr %= numColors;
            }
            else if (direction < 0)
            {
                if (led_ptr == 0)
                    led_ptr = numColors;
                led_ptr--;
            }
        }
    }

private:
    static const uint8_t numColors = 4;
    uint8_t pins[numColors] = {0};
    bool leds[numColors] = {0};

    uint8_t led_ptr = 0;
};