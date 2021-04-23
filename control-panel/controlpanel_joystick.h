#include <Adafruit_PCF8591.h>

namespace joystick
{
#define TILT_THRESHOLD 0.6
    /**
     * @brief 
     * 
     */
    struct Coordinates
    {
        float X;
        float Y;
    };

    /**
     * @brief 
     * 
     */
    class Joystick
    {
    public:
        Joystick(Adafruit_PCF8591 *adc, uint8_t pinx, uint8_t piny);
        Joystick(uint8_t pinx, uint8_t piny);

        float getX(bool update_coords = true);
        float getY(bool update_coords = true);

        bool isTilted(float threshold = TILT_THRESHOLD, bool update_coords = true);
        int getDirection(float axis_coords, bool update_coords = true);

    protected:
        Coordinates coords = {0};
        void read();

    private:
        uint8_t pinx = 0;
        uint8_t piny = 0;
        Adafruit_PCF8591 *adc = NULL;

        float read_pin(uint8_t pin);
    };

    /**
     * @brief Construct a new Joystick:: Joystick object
     * 
     * @param adc 
     * @param pinx 
     * @param piny 
     */
    Joystick::Joystick(Adafruit_PCF8591 *adc, uint8_t pinx, uint8_t piny)
    {
        Joystick::adc = adc;
        Joystick::pinx = pinx;
        Joystick::piny = piny;

        adc->begin();
    }

    /**
     * @brief Construct a new Joystick:: Joystick object
     * 
     * @param pinx 
     * @param piny 
     */
    Joystick::Joystick(uint8_t pinx, uint8_t piny)
    {
        Joystick::pinx = pinx;
        Joystick::piny = piny;
    }

    /**
     * @brief Returns value corresponding to X-axis tilt of the joystick.
     * 
     * @param update_coords bool When set to true, read current tilt values from ADC first.
     * @return float X-axis tilt in range [-1..1]
     */
    float Joystick::getX(bool update_coords)
    {
        if (update_coords)
            Joystick::read();

        return Joystick::coords.X;
    }

    /**
     * @brief Returns value corresponding to Y-axis tilt of the joystick.
     * 
     * @param update_coords bool When set to true, read current tilt values from ADC first.
     * @return float Y-axis tilt in range [-1..1]
     */
    float Joystick::getY(bool update_coords)
    {
        if (update_coords)
            Joystick::read();

        return Joystick::coords.Y;
    }

    /**
     * @brief Returns true if joystick is fully tilted in either direction.
     * 
     * @param threshold float Value of threshold determine if joystick is tilted or not.
     * @param update_coords bool When set to true, read current tilt values from ADC first.
     * @return true Returned when joystick's tilt exceeds 0.6 in any direction.
     * @return false Returned whtn joystick's tile is below 0.6.
     */
    bool Joystick::isTilted(float threshold = TILT_THRESHOLD, bool update_coords = true)
    {
        bool isTilted = false;

        if (update_coords)
            Joystick::read();

        if (abs(Joystick::coords.X) > threshold or abs(Joystick::coords.Y) > threshold)
        {
            isTilted = true;
        }
        return isTilted;
    }

    /**
     * @brief Returns tilt direction in axis indicated by input parameter.
     * 
     * Checks if tilt of selected coordinates (X/Y) exceeds threshold,
     * returns integer indicating direction: 1 for up/left, -1 for down/right.
     * 
     * @param axis_coords One of joystick's coordinate values, in range [-1..1]
     * @param update_coords bool When set to true, read current tilt values from ADC first.
     * @return int Tilt direction: 1 for up/left, -1 for down/right, 0 for neutral postiion.
     */
    int Joystick::getDirection(float axis_coords, bool update_coords = true)
    {
        int direction = 0;
        if (update_coords)
            Joystick::read();

        if (abs(axis_coords) > TILT_THRESHOLD)
        {
            direction = (int)(axis_coords / abs(axis_coords));
        }
        return direction;
    }

    /**
     * @brief Reads current coordinats from joystick's pins and stores them.
     * 
     */
    void Joystick::read()
    {
        Joystick::coords.X = Joystick::read_pin(Joystick::pinx);
        Joystick::coords.Y = Joystick::read_pin(Joystick::piny);
    }

    /**
     * @brief Performs read of specified pin number and coverts value to [-1..1] range.
     * 
     * @param pin Pin number to read. Must be an analog in (either An pin of Arduino or ADC input pin number).
     * @return Joystick coordinates as read from pin.
     */
    float Joystick::read_pin(uint8_t pin)
    {
        float value = 0;

        auto convert = [](int value, uint16_t size) {
            float x = (((float)value / size) - 0.5) * 2;
            return x;
        };

        if (Joystick::adc != NULL)
        {
            uint8_t raw_val = Joystick::adc->analogRead(pin);
            value = convert(raw_val, UINT8_MAX);
        }
        else
        {
            int raw_val = analogRead(pin);
            value = convert(raw_val, 1024);
        }
        return value;
    }
}