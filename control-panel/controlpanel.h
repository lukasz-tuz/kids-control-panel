#include <Adafruit_MCP23017.h>

/**
 * @brief Wrapper for configuring GPIO pins behind MCP23017 I2C IO expander.
 * 
 * @param gpios Pointer to Adafruit_MCP23017 instance associated with pins being configured.
 * @param pins {Array} Array of pin numbers _on expander instance defined by *gpios_ to configure.
 * @param num_pins Length of pins array, in bytes.
 * @param mode Mode (input/input_pullup/output) to which pins are to be configured.
 */
inline void _configurePins(Adafruit_MCP23017 *gpios, byte *pins, byte num_pins, byte mode)
{
    uint8_t mode_internal;
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
    for (byte i = 0; i < num_pins; i++)
    {
        gpios->pinMode(pins[i], mode_internal);
        if (mode == INPUT_PULLUP)
            gpios->pullUp(pins[i], 1);
        else
            gpios->pullUp(pins[i], 0);
    }
}

/**
 * @brief Wrapper for configuring a GPIO pin on MCP23017 I2C IO expander.
 * 
 * @param gpios Pointer to Adafruit_MCP23017 instance associated with pins being configured.
 * @param pin Pin numbers _on expander instance defined by *gpios_ to configure.
 * @param mode Mode (input/input_pullup/output) to which pins are to be configured.
 */
inline void _configurePins(Adafruit_MCP23017 *gpios, byte pin, byte mode)
{
    uint8_t mode_internal;
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
    gpios->pinMode(pin, mode_internal);
    if (mode == INPUT_PULLUP)
        gpios->pullUp(pin, 1);
    else
        gpios->pullUp(pin, 0);
}

/**
 * @brief Configures pin modes for an array of native Arduino GPIOs.
 * 
 * @param pins {Array} Array of pin numbers to configure.
 * @param num_pins Length of pins array, in bytes.
 * @param mode Mode (input/input_pullup/output) to which pins are to be configured.
 */
inline void _configurePins(byte *pins, byte num_pins, byte mode)
{
    for (byte i = 0; i < num_pins; i++)
    {
        pinMode(pins[i], mode);
    }
}

/**
 * @brief Wrapper for Arduino's native pinMode function.
 * 
 * @param pin Pin number to configure.
 * @param mode Mode to which configure selected pin.
 */
inline void _configurePins(byte pin, byte mode)
{
    pinMode(pin, mode);
}

// GPIO expanders
#define GPIOA_ADDR 0
#define GPIOB_ADDR 4

// Pins for power on/off switches

// Pins for enable/disable touch buttons
byte enable_a_pin = 8;
byte enable_b_pin = 9;

// Alphanumeric keypad on GPIOA
const byte rows = 4;
const byte cols = 4;
char alphanumeric[rows][cols] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte colPins[rows] = {4, 5, 6, 7};
byte rowPins[cols] = {0, 1, 2, 3};

// 4x4 keypad on GPIOA
char matrix[rows][cols] = {
    {'0', '1', '2', '3'},
    {'4', '5', '6', '7'},
    {'8', '9', 'A', 'B'},
    {'C', 'D', 'E', 'F'}};
byte matrixRowPins[rows] = {11, 10, 9, 8};
byte matrixColPins[cols] = {15, 14, 13, 12};

// Pushbuttons on GPIOB
const byte buttonsRows = 1;
const byte buttonsCols = 4;
char buttons[buttonsRows][buttonsCols] = {'X', 'Y', 'Z', '@'};
byte buttonRowPins[buttonsRows] = {0};
byte buttonColPins[buttonsCols] = {1, 2, 3, 4};

// Displays

// 7-seg 4-digit display
#define DISPLAY_SIZE 4
#define DISP_CLK_PIN 7
#define DISP_DIO_PIN 8

// 8x8 LED matrix
#define LED_MATRIX_SIZE 8
#define LED_MATRIX_CLK_PIN 5
#define LED_MATRIX_DIO_PIN 4

// RGB LED
#define RBG_RED 9
#define RGB_GRN 10
#define RGB_BLU 11

// LED bar
#define LED_BAR_Y 0
#define LED_BAR_O 1
#define LED_BAR_R 12
#define LED_BAR_B 13

// Joystick
#define JOYSTICK_SWITCH 6 // on gpiosB
#define JOYSTICK_X A1
#define JOYSTICK_Y A0

// Rotary Encoder on GPIOB
#define ENCODER_SWITCH 5
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3

#define DEFAULT_MODE 0
#define RGB_LED_MODE 0
#define LED_BAR_MODE 1
#define LED_MATRIX_MODE 2
#define LCD_MODE 3
#define LAST_MODE 4
