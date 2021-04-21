#include <Arduino.h>
#include <Encoder.h>
#include <Adafruit_MCP23017.h>
#include <TM1637Display.h>
#include "controlpanel.h"
#include "controlpanel_rgbled.h"
#include "controlpanel_display.h"
#include "controlpanel_keypad.h"
#include "controlpanel_ledbar.h"

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

auto gpiosA = new Adafruit_MCP23017();
auto gpiosB = new Adafruit_MCP23017();

PanelKeypad alpha_keypad = PanelKeypad(gpiosA, makeKeymap(alphanumeric), rowPins, colPins, rows, cols);
PanelKeypad matrix_keypad = PanelKeypad(gpiosA, makeKeymap(matrix), matrixRowPins, matrixColPins, rows, cols);
PanelKeypad button_keypad = PanelKeypad(gpiosB, makeKeymap(buttons), buttonRowPins, buttonColPins, buttonsRows, buttonsCols);

Encoder knob(ENCODER_PIN_A, ENCODER_PIN_B);

TM1637Display seven_seg = TM1637Display(DISP_CLK_PIN, DISP_DIO_PIN);
uint8_t seven_seg_brightness = 3;

TM1637Display led_matrix = TM1637Display(LED_MATRIX_CLK_PIN, LED_MATRIX_DIO_PIN);
uint8_t led_matrix_brightness = 3;

Display char_disp = Display(DISPLAY_SIZE);
Display matrix_disp = Display(LED_MATRIX_SIZE);

RgbLed rgb_led = RgbLed(RBG_RED, RGB_GRN, RGB_BLU);
//LedBar led_bar = LedBar(LED_BAR_Y, LED_BAR_O, LED_BAR_R, LED_BAR_B);

uint8_t current_mode = DEFAULT_MODE;
int last_knob_position = 0;

Color stored_rgb_led = Color(0, 0, 0);

uint8_t led_matrix_buffer[LED_MATRIX_SIZE] = {0};
uint8_t coord_x = 0;
uint8_t coord_y = 0;
bool coords_update = false;

void setup()
{
  Wire.begin();
  Serial.begin(115200);
  while (!Serial)
    ; // Leonardo: wait for serial monitor
  Serial.println("Initialzing...");

  _scanI2C();

  gpiosA->begin(GPIOA_ADDR, &Wire);
  gpiosB->begin(GPIOB_ADDR, &Wire);

  // configure pins for buttons/switches which are not part of
  // scanning keypad matrix.
  _configurePins(gpiosB, &joystick_switch, 1, INPUT_PULLUP);
  _configurePins(gpiosB, &encoder_switch, 1, INPUT_PULLUP);
  _configurePins(gpiosB, &enable_a_pin, 1, INPUT_PULLUP);
  _configurePins(gpiosB, &enable_b_pin, 1, INPUT_PULLUP);

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
    int joy_x = analogRead(JOYSTICK_X);
    int joy_y = analogRead(JOYSTICK_Y);
    byte joy_sw = gpiosB->digitalRead(joystick_switch);
    byte enc_sw = gpiosB->digitalRead(encoder_switch);
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
      float x = (((float)joy_x / 1024) - 0.5) * 2;
      float y = (((float)joy_y / 1024) - 0.5) * 2;

      if (abs(x) > 0.1 or abs(y) > 0.1)
      {
        // If joystick is not in neutral position, update LED's color
        // according to (x,y) coordinates.
        uint32_t rgb = rgb_led.rectToRGB(x, y);
        rgb_led.setColor(rgb);

        // Store selected color
        if (joy_sw == 0)
        {
          stored_rgb_led.raw_32 = rgb;
        }
      }
      else
      {
        // Update RGB LD - brightness
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
      float x = (((float)joy_x / 1024) - 0.5) * 2;
      float y = (((float)joy_y / 1024) - 0.5) * 2;

      if ((abs(x) > 0.6 or abs(y) > 0.6) and coords_update == false)
      {
        coords_update = true;

        if (abs(x) > 0.6)
        {
          int direction_x = (int)(x / abs(x));
          coord_x = wrap_around(coord_x, direction_x, LED_MATRIX_SIZE);
        }
        // y coordinate
        if (abs(y) > 0.6)
        {
          int direction_y = (int)(y / abs(y));
          coord_y = wrap_around(coord_y, direction_y, LED_MATRIX_SIZE);
        }

        led_matrix_buffer[coord_x] ^= (1 << coord_y);

        led_matrix.setSegments(led_matrix_buffer, LED_MATRIX_SIZE);
      }
      else if ((abs(x) < 0.1 and abs(y) < 0.1) and coords_update == true)
      {
        coords_update = false;
      }
    }
    else if (current_mode == LCD_MODE)
    {
    }

    if (character != NO_KEY)
    {
      char_disp.set_character(character);
      // Update 7seg display
      seven_seg.setSegments(char_disp.get_buffer(), DISPLAY_SIZE);
    }

    if (matrix_key != NO_KEY)
    {
      matrix_disp.set_character(matrix_key);
      // Update LED matrix
      led_matrix.setSegments(matrix_disp.get_buffer(), LED_MATRIX_SIZE);
    }
  }
}
