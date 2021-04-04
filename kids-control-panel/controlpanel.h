namespace controlpanel
{
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

        void begin(uint8_t y, uint8_t o, uint8_t g, uint8_t b)
        {
            LedBar::pins[yellow] = y;
            LedBar::pins[orange] = o;
            LedBar::pins[green] = g;
            LedBar::pins[blue] = b;

            for (uint8_t nPin = 0; nPin < LedBar::numPins; nPin++)
            {
                pinMode(LedBar::pins[nPin], OUTPUT);
                digitalWrite(LedBar::pins[nPin], 0);
            }
        }

        void off()
        {
            for (uint8_t nPin = 0; nPin < LedBar::numPins; nPin++)
            {
                digitalWrite(LedBar::pins[nPin], 0);
            }
        }

        void color(uint8_t c, bool on)
        {
            if (c < max_color)
            {
                digitalWrite(LedBar::pins[c], (uint8_t)on);
            }
        }

    private:
        static const uint8_t numPins = 4;
        uint8_t pins[numPins] = {0};
    };

    

} // namespace controlpanel
