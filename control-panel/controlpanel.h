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

/*
PINOUTS: ARDUINO UNO
                                      +-----+
         +----[PWR]-------------------| USB |--+
         |                            +-----+  |
         |         GND/RST2  [ ][ ]            |
         |       MOSI2/SCK2  [ ][ ]  A5/SCL[ ] |   C5 
         |          5V/MISO2 [ ][ ]  A4/SDA[ ] |   C4 
         |                             AREF[ ] |
         |                              GND[ ] |
         | [ ]N/C                    SCK/13[L] |   B5
         | [ ]IOREF                 MISO/12[L] |   .
         | [ ]RST                   MOSI/11[M]~|   .
         | [ ]3V3    +---+               10[M]~|   .
         | [ ]5v    -| A |-               9[S]~|   .
         | [ ]GND   -| R |-               8[S] |   B0
         | [ ]GND   -| D |-                    |
         | [ ]Vin   -| U |-               7[T] |   D7
         |          -| I |-               6[T]~|   .
         | [T]A0    -| N |-               5[L]~|   .
         | [T]A1    -| O |-               4[L] |   .
         | [T]A2     +---+           INT1/3[E]~|   .
         | [T]A3                     INT0/2[E] |   .
         | [T*]A4/SDA  RST SCK MISO     TX>1[ ] |   .
         | [ ]A5/SCL  [ ] [ ] [ ]      RX<0[ ] |   D0
         |            [ ] [ ] [ ]              |
         |  UNO_R3    GND MOSI 5V  ____________/
          \_______________________/
   
   http://busyducks.com/ascii-art-arduinos

   *) A4 shared between LCD_RST and I2C SDA
*/

// LED bar pins (L)
#define LED_BAR_Y (4)
#define LED_BAR_O (5)
#define LED_BAR_R (12)
#define LED_BAR_B (13)

// Rotary encoder pins (E)
#define ENCODER_PIN_A (2)
#define ENCODER_PIN_B (3)

// 7seg Display pins (S)
#define DISP_CLK_PIN (8)
#define DISP_DIO_PIN (9)

// LED Matrix pins (M)
#define LED_MATRIX_CLK_PIN (10)
#define LED_MATRIX_DIO_PIN (11)

// RGB LED pins - PWM required! (R)
#undef RGB_LED_CONNECTED
#define RBG_RED (9)
#define RGB_GRN (10)
#define RGB_BLU (11)

// TFT LCD (T)
#undef LCD_TFT_CONNECTED
const int XP = (7), XM = A1, YP = A2, YM = (6); //240x320 ID=0x6809

/*
   GPIO Exander A
*/
// Alphanumeric keypad on GPIOA
const byte rows = 4;
const byte cols = 4;
char alphanumeric[rows][cols] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte colPins[rows] = {3, 2, 1, 0};
byte rowPins[cols] = {7, 6, 5, 4};

// 4x4 keypad on GPIOA
char matrix[rows][cols] = {
    {'0', '1', '2', '3'},
    {'4', '5', '6', '7'},
    {'8', '9', 'A', 'B'},
    {'C', 'D', 'E', 'F'}};
byte matrixRowPins[rows] = {11, 10, 9, 8};
byte matrixColPins[cols] = {15, 14, 13, 12};

/*
    GPIO Expander B
*/
// Pushbuttons on GPIOB
const byte buttonsRows = 1;
const byte buttonsCols = 4;
char buttons[buttonsRows][buttonsCols] = {'X', 'Y', 'Z', '@'};
byte buttonRowPins[buttonsRows] = {0};
byte buttonColPins[buttonsCols] = {1, 2, 3, 4};

// Pins for enable/disable touch buttons on gpioB
#define ENCODER_SWITCH (5)
#define JOYSTICK_SWITCH (6)

#define ENABLE_A_PIN (8)
#define ENABLE_B_PIN (9) 

/*
    ADC Expander A
*/
// Joystick pins on ADC
#define JOYSTICK_X (0)
#define JOYSTICK_Y (1)

/*
    CONFIGURATION SETTINGS
*/
// GPIO expander addresses
#define GPIOA_ADDR (0)
#define GPIOB_ADDR (1)

// 7-seg display size - 4-digit
#define DISPLAY_SIZE (4)

// LED matrix size - 8x8
#define LED_MATRIX_SIZE (8)

// TFT LCD
// Values hand calibrated for specific TFT LCD shield
const int TS_LEFT = 915, TS_RT = 143, TS_TOP = 129, TS_BOT = 931;

#define MINPRESSURE (200)
#define MAXPRESSURE (1000)

// Assign human-readable names to some common 16-bit color values:
#define BLACK (0x0000)
#define BLUE (0x001F)
#define RED (0xF800)
#define GREEN (0x07E0)
#define CYAN (0x07FF)
#define MAGENTA (0xF81F)
#define YELLOW (0xFFE0)
#define WHITE (0xFFFF)

// Control modes for encoder/joystick
#define DEFAULT_MODE (0)
#define RGB_LED_MODE (0)
#define LED_BAR_MODE (1)
#define LED_MATRIX_MODE (2)
#define LCD_MODE (3)
#define LAST_MODE (4)
