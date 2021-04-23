
#include <Arduino.h>
#include <Encoder.h>
#include <Adafruit_MCP23017.h>
#include <TM1637Display.h>
#include "controlpanel.h"
#include "controlpanel_rgbled.h"
#include "controlpanel_display.h"
#include "controlpanel_keypad.h"
#include "controlpanel_ledbar.h"
#include "controlpanel_joystick.h"

/**
  From Arduino I2C examples.
  Scan I2C bus for devices, print addresses on Serial.
 */
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

/**
 * @brief Convert a character based control mode selection into
 * an encoded value.
 * 
 * @param b Character received from input device
 * @return uint8_t uint8 encoded control mode for the panel.
 */
uint8_t get_controls_mode(char b)
{
  uint8_t mode;

  switch (b)
  {
  case 'X':
    mode = RGB_LED_MODE;
    break;
  case 'Y':
    mode = LED_BAR_MODE;
    break;
  case 'Z':
    mode = LED_MATRIX_MODE;
    break;
  case '@':
    mode = LCD_MODE;
    break;
  default:
    mode = DEFAULT_MODE;
    break;
  }

  return mode;
}

/**
 * @brief Increment/decrement value with wrap around at value defined by size.
 * 
 * If incrementing (direction > 0), value will wrap around to zero when specified
 * size is reached.
 * 
 * If decrementing (direction < 0), value wraps around to size - 1 when zero is reached.
 * 
 * @param value Value to be modified.
 * @param direction Determines if value is to be incremented or decremented.
 * @param size Size at which value is to be wrapped.
 * @return uint8_t Modified value.
 */
uint8_t wrap_around(uint8_t value, int direction, uint8_t size)
{
  uint8_t v = value;

  if (direction > 0)
  {
    ++v %= size;
  }

  if (direction < 0)
  {
    if (v == 0)
    {
      v = size;
    }
    v--;
  }
  return v;
}

using namespace rgbled;
using namespace joystick;

/* 
Instantiate objects for control panel's internals.

GPIO Expanders go first.
*/
auto gpiosA = new Adafruit_MCP23017();
auto gpiosB = new Adafruit_MCP23017();
auto adc = new Adafruit_PCF8591();

/* Keypad panel control objects */
PanelKeypad alpha_keypad = PanelKeypad(gpiosA, makeKeymap(alphanumeric), rowPins, colPins, rows, cols);
PanelKeypad matrix_keypad = PanelKeypad(gpiosA, makeKeymap(matrix), matrixRowPins, matrixColPins, rows, cols);
PanelKeypad button_keypad = PanelKeypad(gpiosB, makeKeymap(buttons), buttonRowPins, buttonColPins, buttonsRows, buttonsCols);

/* Rotary encoder */
Encoder knob = Encoder(ENCODER_PIN_A, ENCODER_PIN_B);

/* Joystick */
Joystick joy = Joystick(adc, JOYSTICK_X, JOYSTICK_Y);

/* Displays */
TM1637Display seven_seg = TM1637Display(DISP_CLK_PIN, DISP_DIO_PIN);
uint8_t seven_seg_brightness = 3;

TM1637Display led_matrix = TM1637Display(LED_MATRIX_CLK_PIN, LED_MATRIX_DIO_PIN);
uint8_t led_matrix_brightness = 3;

/* Common wrappers for hardware-specific objects */
Display char_disp = Display(DISPLAY_SIZE);
Display matrix_disp = Display(LED_MATRIX_SIZE);

/* The RGB LED module */
RgbLed rgb_led = RgbLed(RBG_RED, RGB_GRN, RGB_BLU);

/* Custom LED bar module */
//LedBar led_bar = LedBar(LED_BAR_Y, LED_BAR_O, LED_BAR_R, LED_BAR_B);

/**
 * @brief Mode determines which modules are controlled by joystick and encoder.
 * Mode is swtiched on user input. @see pushbutton.
 * 
 */
uint8_t current_mode = DEFAULT_MODE;

/**
 * @brief Last recorded position of rotary encoder, used to determine rotation direction.
 * 
 */
int last_knob_position = 0;

/**
 * @brief Selected color for RGB LED module.
 * 
 */
Color stored_rgb_led = Color(0, 0, 0);

/**
 * @brief Internal display buffer for LED matrix module.
 * 
 */
uint8_t led_matrix_buffer[LED_MATRIX_SIZE] = {0};
uint8_t coord_x = 0;
uint8_t coord_y = 0;

/**
 * @brief When set, locks changes of coord_x/coord_y variables on subsequent calls for loop().
 * 
 */
bool coords_update = false;

void setup()
{
  Wire.begin();
  Serial.begin(115200);
  while (!Serial)
    ; // Leonardo: wait for serial monitor
  Serial.println("Initialzing...");

  // I2C scan only called for debug purposes.
  _scanI2C();

  gpiosA->begin(GPIOA_ADDR, &Wire);
  gpiosB->begin(GPIOB_ADDR, &Wire);
  adc->begin();

  // Configure pins for buttons/switches which are not part of
  // scanning keypad matrix.
  _configurePins(gpiosB, JOYSTICK_SWITCH, INPUT_PULLUP);
  _configurePins(gpiosB, ENCODER_SWITCH, INPUT_PULLUP);
  _configurePins(gpiosB, &enable_a_pin, 1, INPUT_PULLUP);
  _configurePins(gpiosB, &enable_b_pin, 1, INPUT_PULLUP);

  // Initialize displays
  seven_seg.setBrightness(seven_seg_brightness, true);
  seven_seg.clear();

  led_matrix.setBrightness(led_matrix_brightness, true);
  led_matrix.clear();

  Color c = Color(0.0f, 0.0f, 0.0f);
  rgb_led.setColor(c);

  // led_bar.off();
}

void loop()
{
  // Read state of Enable pins
  byte enable_a = gpiosB->digitalRead(enable_a_pin);

  if (enable_a)
  {
    // Poll aplhanumeric keypad, key matrix, buttons, joystick, encoder
    char character = alpha_keypad.getKey();
    char matrix_key = matrix_keypad.getKey();
    char pushbutton = button_keypad.getKey();
    byte joy_sw = gpiosB->digitalRead(JOYSTICK_SWITCH);
    byte enc_sw = gpiosB->digitalRead(ENCODER_SWITCH);
    int position = knob.read();
    int delta = position - last_knob_position;
    last_knob_position = position;

    if (pushbutton != NO_KEY)
    {
      uint8_t mode = get_controls_mode(pushbutton);
      if (mode != current_mode)
      {
        Serial.print("Switching mode to ");
        Serial.println(mode);
        current_mode = mode;
      }
    }

    if (current_mode == RGB_LED_MODE)
    {
      // Map raw values from joy_x/_y to [-1:1] range
      if (joy.isTilted(0.1))
      {
        // If joystick is not in neutral position, update LED's color
        // according to (x,y) coordinates.
        float joy_x = joy.getX(false);
        float joy_y = joy.getY(false);

        uint32_t rgb = rgb_led.rectToRGB(joy_x, joy_y);
        rgb_led.setColor(rgb);

        // Store selected color
        if (joy_sw == 0)
        {
          stored_rgb_led.raw_32 = rgb;
        }
      }
      else
      {
        // Update RGB LED's brightness
        if (delta != 0)
        {
          rgb_led.changeBrightness(delta);
          stored_rgb_led = rgb_led.getColor();
        }
        // Show stored color when joystick is in neutral position.
        rgb_led.setColor(stored_rgb_led);
      }
    }
    else if (current_mode == LED_BAR_MODE)
    {
      //led_bar.move_bar(delta);
    }
    else if (current_mode == LED_MATRIX_MODE)
    {
      /*
        If joystick is pushed in either direction, update display.
      */
      if (joy.isTilted(0.6) and coords_update == false)
      {
        // Lock update so that each push of joystick only moves LEDs by one
        coords_update = true;

        // don't update coordinates in getX/getY calls, just use ones already done.
        int direction_x = joy.getDirection(joy.getX(false), false);
        coord_x = wrap_around(coord_x, direction_x, LED_MATRIX_SIZE);
        int direction_y = joy.getDirection(joy.getY(false), false);
        coord_y = wrap_around(coord_y, direction_y, LED_MATRIX_SIZE);

        // XOR operation done so that LEDs can be switched on/off
        led_matrix_buffer[coord_x] ^= (1 << coord_y);
        led_matrix.setSegments(led_matrix_buffer, LED_MATRIX_SIZE);
      }
      else if (!joy.isTilted(0.1) and coords_update == true)
      {
        // Joystick in neutral position, release lock
        coords_update = false;
      }
    }
    else if (current_mode == LCD_MODE)
    {
    }

    if (character != NO_KEY)
    {
      // Update 7seg display
      char_disp.set_character(character);
      seven_seg.setSegments(char_disp.get_buffer(), DISPLAY_SIZE);
    }

    if (matrix_key != NO_KEY)
    {
      // Update LED matrix
      matrix_disp.set_character(matrix_key);
      led_matrix.setSegments(matrix_disp.get_buffer(), LED_MATRIX_SIZE);
    }
  }
}
