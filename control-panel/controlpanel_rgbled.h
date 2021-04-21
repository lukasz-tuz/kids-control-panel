#include <math.h>

namespace rgbled
{
// From esphome's color.h
#define ALWAYS_INLINE __attribute__((always_inline))

    inline static uint8_t scale8(uint8_t i, uint8_t scale)
    {
        return (uint16_t(i) * (1 + uint16_t(scale))) / 256;
    }
    inline static uint8_t scale(uint8_t i, uint8_t scale, uint8_t max_value = 255) { return (max_value * i / scale); }

    /**
     * @brief From esphome's color.h. Defines RBG or HSV colors, it's transformations and operations.
     * 
     */
    struct Color
    {
        union
        {
            struct
            {
                union
                {
                    uint8_t r;
                    uint8_t red;
                };
                union
                {
                    uint8_t g;
                    uint8_t green;
                };
                union
                {
                    uint8_t b;
                    uint8_t blue;
                };
                union
                {
                    uint8_t w;
                    uint8_t white;
                };
            };
            uint8_t raw[4];
            uint32_t raw_32;
        };
        enum ColorOrder : uint8_t
        {
            COLOR_ORDER_RGB = 0,
            COLOR_ORDER_BGR = 1,
            COLOR_ORDER_GRB = 2
        };
        enum ColorBitness : uint8_t
        {
            COLOR_BITNESS_888 = 0,
            COLOR_BITNESS_565 = 1,
            COLOR_BITNESS_332 = 2
        };
        inline Color() ALWAYS_INLINE : r(0), g(0), b(0), w(0) {} // NOLINT
        inline Color(float red, float green, float blue) ALWAYS_INLINE : r(uint8_t(red * 255)),
                                                                         g(uint8_t(green * 255)),
                                                                         b(uint8_t(blue * 255)),
                                                                         w(0) {}
        inline Color(float red, float green, float blue, float white) ALWAYS_INLINE : r(uint8_t(red * 255)),
                                                                                      g(uint8_t(green * 255)),
                                                                                      b(uint8_t(blue * 255)),
                                                                                      w(uint8_t(white * 255)) {}
        inline Color(uint32_t colorcode) ALWAYS_INLINE : r((colorcode >> 16) & 0xFF),
                                                         g((colorcode >> 8) & 0xFF),
                                                         b((colorcode >> 0) & 0xFF),
                                                         w((colorcode >> 24) & 0xFF) {}
        inline bool is_on() ALWAYS_INLINE { return this->raw_32 != 0; }
        inline Color &operator=(const Color &rhs) ALWAYS_INLINE
        {
            this->r = rhs.r;
            this->g = rhs.g;
            this->b = rhs.b;
            this->w = rhs.w;
            return *this;
        }
        inline Color &operator=(uint32_t colorcode) ALWAYS_INLINE
        {
            this->w = (colorcode >> 24) & 0xFF;
            this->r = (colorcode >> 16) & 0xFF;
            this->g = (colorcode >> 8) & 0xFF;
            this->b = (colorcode >> 0) & 0xFF;
            return *this;
        }
        inline uint8_t &operator[](uint8_t x) ALWAYS_INLINE { return this->raw[x]; }
        inline Color operator*(uint8_t scale) const ALWAYS_INLINE
        {
            return Color(scale8(this->red, scale), scale8(this->green, scale), scale8(this->blue, scale),
                         scale8(this->white, scale));
        }
        inline Color &operator*=(uint8_t scale) ALWAYS_INLINE
        {
            this->red = scale8(this->red, scale);
            this->green = scale8(this->green, scale);
            this->blue = scale8(this->blue, scale);
            this->white = scale8(this->white, scale);
            return *this;
        }
        inline Color operator*(const Color &scale) const ALWAYS_INLINE
        {
            return Color(scale8(this->red, scale.red), scale8(this->green, scale.green),
                         scale8(this->blue, scale.blue), scale8(this->white, scale.white));
        }
        inline Color &operator*=(const Color &scale) ALWAYS_INLINE
        {
            this->red = scale8(this->red, scale.red);
            this->green = scale8(this->green, scale.green);
            this->blue = scale8(this->blue, scale.blue);
            this->white = scale8(this->white, scale.white);
            return *this;
        }
        inline Color operator+(const Color &add) const ALWAYS_INLINE
        {
            Color ret;
            if (uint8_t(add.r + this->r) < this->r)
                ret.r = 255;
            else
                ret.r = this->r + add.r;
            if (uint8_t(add.g + this->g) < this->g)
                ret.g = 255;
            else
                ret.g = this->g + add.g;
            if (uint8_t(add.b + this->b) < this->b)
                ret.b = 255;
            else
                ret.b = this->b + add.b;
            if (uint8_t(add.w + this->w) < this->w)
                ret.w = 255;
            else
                ret.w = this->w + add.w;
            return ret;
        }
        inline Color &operator+=(const Color &add) ALWAYS_INLINE { return *this = (*this) + add; }
        inline Color operator+(uint8_t add) const ALWAYS_INLINE { return (*this) + Color(add, add, add, add); }
        inline Color &operator+=(uint8_t add) ALWAYS_INLINE { return *this = (*this) + add; }
        inline Color operator-(const Color &subtract) const ALWAYS_INLINE
        {
            Color ret;
            if (subtract.r > this->r)
                ret.r = 0;
            else
                ret.r = this->r - subtract.r;
            if (subtract.g > this->g)
                ret.g = 0;
            else
                ret.g = this->g - subtract.g;
            if (subtract.b > this->b)
                ret.b = 0;
            else
                ret.b = this->b - subtract.b;
            if (subtract.w > this->w)
                ret.w = 0;
            else
                ret.w = this->w - subtract.w;
            return ret;
        }
        inline Color &operator-=(const Color &subtract) ALWAYS_INLINE { return *this = (*this) - subtract; }
        inline Color operator-(uint8_t subtract) const ALWAYS_INLINE
        {
            return (*this) - Color(subtract, subtract, subtract, subtract);
        }
        inline Color &operator-=(uint8_t subtract) ALWAYS_INLINE { return *this = (*this) - subtract; }
        Color fade_to_white(uint8_t amnt) { return Color(1, 1, 1, 1) - (*this * amnt); }
        Color fade_to_black(uint8_t amnt) { return *this * amnt; }
        Color lighten(uint8_t delta) { return *this + delta; }
        Color darken(uint8_t delta) { return *this - delta; }
        uint8_t to_332(ColorOrder color_order = ColorOrder::COLOR_ORDER_RGB) const
        {
            uint16_t red_color, green_color, blue_color;

            red_color = scale8(this->red, ((1 << 3) - 1));
            green_color = scale8(this->green, ((1 << 3) - 1));
            blue_color = scale8(this->blue, (1 << 2) - 1);

            switch (color_order)
            {
            case COLOR_ORDER_RGB:
                return red_color << 5 | green_color << 2 | blue_color;
            case COLOR_ORDER_BGR:
                return blue_color << 6 | green_color << 3 | red_color;
            case COLOR_ORDER_GRB:
                return green_color << 5 | red_color << 2 | blue_color;
            }
            return 0;
        }
        uint16_t to_565(ColorOrder color_order = ColorOrder::COLOR_ORDER_RGB) const
        {
            uint16_t red_color, green_color, blue_color;

            red_color = scale8(this->red, ((1 << 5) - 1));
            green_color = scale8(this->green, ((1 << 6) - 1));
            blue_color = scale8(this->blue, (1 << 5) - 1);

            switch (color_order)
            {
            case COLOR_ORDER_RGB:
                return red_color << 11 | green_color << 5 | blue_color;
            case COLOR_ORDER_BGR:
                return blue_color << 11 | green_color << 5 | red_color;
            case COLOR_ORDER_GRB:
                return green_color << 10 | red_color << 5 | blue_color;
            }
            return 0;
        }
        uint32_t to_rgb_565() const
        {
            uint32_t color565 =
                (scale8(this->red, 31) << 11) | (scale8(this->green, 63) << 5) | (scale8(this->blue, 31) << 0);
            return color565;
        }
        uint32_t to_bgr_565() const
        {
            uint32_t color565 =
                (scale8(this->blue, 31) << 11) | (scale8(this->green, 63) << 5) | (scale8(this->red, 31) << 0);
            return color565;
        }
        uint32_t to_grayscale4() const
        {
            uint32_t gs4 = scale8(this->white, 15);
            return gs4;
        }
    };

    inline uint8_t change_color(uint8_t *color, int delta)
    {
        uint8_t d = abs(delta);
        int c = (int)*color;
        if (delta > 0)
            *color = (uint8_t)min(c + d, 255);
        if (delta < 0)
            *color = (uint8_t)max(c - d, 0);
    }

    class RgbLed
    {
    private:
        /* data */
        uint8_t pin_red;
        uint8_t pin_green;
        uint8_t pin_blue;

    protected:
        Color color;
        void setRGB(void);
        uint32_t hsv2rgb(uint16_t hue, float saturation, float value);

    public:
        RgbLed(uint8_t pin_red, uint8_t pin_green, uint8_t pin_blue);
        ~RgbLed();

        void setColor(rgbled::Color color);
        void lighten(uint8_t amount);
        void darken(uint8_t amount);
        void changeBrightness(int delta);
        Color getColor(void);
        uint32_t rectToRGB(float x, float y);
    };

    RgbLed::RgbLed(uint8_t pin_red, uint8_t pin_green, uint8_t pin_blue)
    {
        RgbLed::pin_red = pin_red;
        RgbLed::pin_green = pin_green;
        RgbLed::pin_blue = pin_blue;

        pinMode(pin_red, OUTPUT);
        pinMode(pin_green, OUTPUT);
        pinMode(pin_blue, OUTPUT);

        RgbLed::color = Color{0};
    }

    RgbLed::~RgbLed()
    {
    }

    /** 
    * @brief Convert cartesian coordinates to RGB
    * 
    * @param {float} x - x coordinate in range -1 to 1
    * @param {float} y - y coordinate in range -1 to 1
    * @returns {Color} [red, green, blue] values in range 0 to 255
    */
    uint32_t RgbLed::rectToRGB(float x, float y)
    {
        float theta = 0;
        auto cval = [](float theta, float ro, float phase, float neg_phase) {
            float val = sin(0.666 * theta - phase);
            if (val < 0)
                val = sin(0.666 * theta - neg_phase);
            return val;
        };

        if (abs(x) > 0.1 or abs(y) > 0.1)
        {
            theta = atan2(y, x) + PI;
        }

        float ro = sqrt(x * x + y * y);
        float r = cval(theta, ro, -PI / 2, PI);
        float g = cval(theta, ro, 0, 3 * PI / 2);
        float b = cval(theta, ro, PI / 2, 5 * PI / 2);
        Color c = Color(r, g, b);

        return c.raw_32;
    }

    void RgbLed::setRGB(void)
    {
        analogWrite(RgbLed::pin_red, RgbLed::color.r);
        analogWrite(RgbLed::pin_green, RgbLed::color.g);
        analogWrite(RgbLed::pin_blue, RgbLed::color.b);
    }

    void RgbLed::setColor(Color color)
    {
        RgbLed::color = color;
        RgbLed::setRGB();
    }

    void RgbLed::changeBrightness(int delta)
    {
        change_color(&RgbLed::color.r, delta);
        change_color(&RgbLed::color.g, delta);
        change_color(&RgbLed::color.b, delta);
        RgbLed::setRGB();
    }

    Color RgbLed::getColor(void)
    {
        return RgbLed::color;
    }

    void RgbLed::lighten(uint8_t amount)
    {
        RgbLed::color += amount;
        RgbLed::setRGB();
    }

    void RgbLed::darken(uint8_t amount)
    {
        RgbLed::color -= amount;
        RgbLed::setRGB();
    }
}