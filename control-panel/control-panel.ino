
#include <Arduino.h>
#include <Encoder.h>
#include <Adafruit_MCP23017.h>
#include <TM1637Display.h>

#include "controlpanel.h"

#ifdef LCD_TFT_CONNECTED
#include <MCUFRIEND_kbv.h>
MCUFRIEND_kbv tft; // hard-wired for UNO shields anyway.
#include <TouchScreen.h>
#endif

#ifdef RGB_LED_CONNECTED
#include "controlpanel_rgbled.h"
#endif

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

#ifdef RGB_LED_CONNECTED
using namespace rgbled;
#endif
using namespace joystick;

/*
Instantiate objects for control panel's internals.

GPIO Expanders go first.
*/
auto gpiosA = new Adafruit_MCP23017();
auto gpiosB = new Adafruit_MCP23017();
Adafruit_PCF8591 adc = Adafruit_PCF8591();

// /* Keypad panel control objects */
PanelKeypad alpha_keypad = PanelKeypad(gpiosA, makeKeymap(alphanumeric), rowPins, colPins, rows, cols);
PanelKeypad matrix_keypad = PanelKeypad(gpiosA, makeKeymap(matrix), matrixRowPins, matrixColPins, rows, cols);
PanelKeypad button_keypad = PanelKeypad(gpiosB, makeKeymap(buttons), buttonRowPins, buttonColPins, buttonsRows, buttonsCols);

// /* Rotary encoder */
Encoder knob = Encoder(ENCODER_PIN_A, ENCODER_PIN_B);

// /* Joystick */
Joystick joy = Joystick(&adc, JOYSTICK_X, JOYSTICK_Y);

// /* Displays */
TM1637Display seven_seg = TM1637Display(DISP_CLK_PIN, DISP_DIO_PIN);
uint8_t seven_seg_brightness = 3;

TM1637Display led_matrix = TM1637Display(LED_MATRIX_CLK_PIN, LED_MATRIX_DIO_PIN);
uint8_t led_matrix_brightness = 3;

// /* Common wrappers for hardware-specific objects */
Display char_disp = Display(DISPLAY_SIZE);
Display matrix_disp = Display(LED_MATRIX_SIZE);

#ifdef RGB_LED_CONNECTED
// /* The RGB LED module */
RgbLed rgb_led = RgbLed(RBG_RED, RGB_GRN, RGB_BLU);
#endif

/* Custom LED bar module */
LedBar led_bar = LedBar(LED_BAR_Y, LED_BAR_O, LED_BAR_R, LED_BAR_B);

#ifdef LCD_TFT_CONNECTED
/* The TFT LCD display shield with touch*/
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

/**
 * @brief Global variable for storing coordinates for point pressed on screen.
 *
 */
TSPoint tp;

/**
 * @brief TFT LCD shield control variables.
 *
 */
int16_t BOXSIZE; // Size of color picker boxes, calculated in setup()
int16_t PENRADIUS = 2;
uint16_t ID, oldcolor, currentcolor;
uint8_t Orientation = 0; // PORTRAIT
#endif

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

#ifdef RGB_LED_CONNECTED
/**
 * @brief Selected color for RGB LED module.
 *
 */
Color stored_rgb_led = Color(0, 0, 0);
#endif

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

#ifdef LCD_TFT_CONNECTED
void setup_tft()
{
  /* Setup TFT LCD screen */
  uint16_t tmp;

  tft.reset();
  ID = tft.readID();
  tft.begin(ID);
  tft.setRotation(Orientation);
  tft.fillScreen(BLACK);

  BOXSIZE = tft.width() / 6;
  tft.fillScreen(BLACK);
  tft.fillRect(0, 0, BOXSIZE, BOXSIZE, RED);
  tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, YELLOW);
  tft.fillRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, GREEN);
  tft.fillRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, CYAN);
  tft.fillRect(BOXSIZE * 4, 0, BOXSIZE, BOXSIZE, BLUE);
  tft.fillRect(BOXSIZE * 5, 0, BOXSIZE, BOXSIZE, MAGENTA);

  tft.drawRect(0, 0, BOXSIZE, BOXSIZE, WHITE);
  currentcolor = RED;
  delay(1000);
}

/**
 * @brief Loop for paint app displayed on TFT LCD shield. Also handles touch input.
 *
 */
void loop_tft()
{
  uint16_t xpos, ypos; //screen coordinates
  tp = ts.getPoint();  //tp.x, tp.y are ADC values

  // we have some minimum pressure we consider 'valid'
  // pressure of 0 means no pressing!
  if (tp.z > MINPRESSURE && tp.z < MAXPRESSURE)
  {
    // most mcufriend have touch (with icons) that extends below the TFT
    // screens without icons need to reserve a space for "erase"
    // scale the ADC values from ts.getPoint() to screen values e.g. 0-239
    //
    // Calibration is true for PORTRAIT. tp.y is always long dimension
    // map to your current pixel orientation
    switch (Orientation)
    {
    case 0:
      xpos = map(tp.x, TS_LEFT, TS_RT, 0, tft.width());
      ypos = map(tp.y, TS_TOP, TS_BOT, 0, tft.height());
      break;
    case 1:
      xpos = map(tp.y, TS_TOP, TS_BOT, 0, tft.width());
      ypos = map(tp.x, TS_RT, TS_LEFT, 0, tft.height());
      break;
    case 2:
      xpos = map(tp.x, TS_RT, TS_LEFT, 0, tft.width());
      ypos = map(tp.y, TS_BOT, TS_TOP, 0, tft.height());
      break;
    case 3:
      xpos = map(tp.y, TS_BOT, TS_TOP, 0, tft.width());
      ypos = map(tp.x, TS_LEFT, TS_RT, 0, tft.height());
      break;
    }

    // are we in top color box area ?
    if (ypos < BOXSIZE)
    { //draw white border on selected color box
      oldcolor = currentcolor;

      if (xpos < BOXSIZE)
      {
        currentcolor = RED;
        tft.drawRect(0, 0, BOXSIZE, BOXSIZE, WHITE);
      }
      else if (xpos < BOXSIZE * 2)
      {
        currentcolor = YELLOW;
        tft.drawRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, WHITE);
      }
      else if (xpos < BOXSIZE * 3)
      {
        currentcolor = GREEN;
        tft.drawRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, WHITE);
      }
      else if (xpos < BOXSIZE * 4)
      {
        currentcolor = CYAN;
        tft.drawRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, WHITE);
      }
      else if (xpos < BOXSIZE * 5)
      {
        currentcolor = BLUE;
        tft.drawRect(BOXSIZE * 4, 0, BOXSIZE, BOXSIZE, WHITE);
      }
      else if (xpos < BOXSIZE * 6)
      {
        currentcolor = MAGENTA;
        tft.drawRect(BOXSIZE * 5, 0, BOXSIZE, BOXSIZE, WHITE);
      }

      if (oldcolor != currentcolor)
      { //rub out the previous white border
        if (oldcolor == RED)
          tft.fillRect(0, 0, BOXSIZE, BOXSIZE, RED);
        if (oldcolor == YELLOW)
          tft.fillRect(BOXSIZE, 0, BOXSIZE, BOXSIZE, YELLOW);
        if (oldcolor == GREEN)
          tft.fillRect(BOXSIZE * 2, 0, BOXSIZE, BOXSIZE, GREEN);
        if (oldcolor == CYAN)
          tft.fillRect(BOXSIZE * 3, 0, BOXSIZE, BOXSIZE, CYAN);
        if (oldcolor == BLUE)
          tft.fillRect(BOXSIZE * 4, 0, BOXSIZE, BOXSIZE, BLUE);
        if (oldcolor == MAGENTA)
          tft.fillRect(BOXSIZE * 5, 0, BOXSIZE, BOXSIZE, MAGENTA);
      }
    }
    // are we in drawing area ?
    if (((ypos - PENRADIUS) > BOXSIZE) && ((ypos + PENRADIUS) < tft.height()))
    {
      tft.fillCircle(xpos, ypos, PENRADIUS, currentcolor);
    }
    // are we in erase area ?
    // Plain Touch panels use bottom 10 pixels e.g. > h - 10
    // Touch panels with icon area e.g. > h - 0
    if (ypos > tft.height() - 10)
    {
      // press the bottom of the screen to erase
      tft.fillRect(0, BOXSIZE, tft.width(), tft.height() - BOXSIZE, BLACK);
    }
  }
}
#endif

void setup()
{
  Wire.begin();
  Serial.begin(115200);
  while (!Serial)
    ; // Leonardo: wait for serial monitor
  Serial.println("Initialzing...");

#ifdef LCD_TFT_CONNECTED
  setup_tft();
#endif

  // I2C scan only called for debug purposes.
  _scanI2C();

  gpiosA->begin(GPIOA_ADDR, &Wire);
  gpiosB->begin(GPIOB_ADDR, &Wire);
  adc.begin();

  // Configure pins for buttons/switches which are not part of
  // scanning keypad matrix.
  _configurePins(gpiosB, JOYSTICK_SWITCH, INPUT_PULLUP);
  _configurePins(gpiosB, ENCODER_SWITCH, INPUT_PULLUP);
  _configurePins(gpiosB, ENABLE_A_PIN, INPUT_PULLUP);
  _configurePins(gpiosB, ENABLE_B_PIN, INPUT_PULLUP);

  // Initialize displays
  seven_seg.setBrightness(seven_seg_brightness, true);
  seven_seg.clear();

  led_matrix.setBrightness(led_matrix_brightness, true);
  led_matrix.clear();

  led_bar.off();

  rgb_led.setColor(Color(0.5, 0.5, 0.5));
}

/**
 * @brief Handler for LED matrix controls by encoder and joystick
 *
 */
void led_matrix_handler()
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

#ifdef RGB_LED_CONNECTED
/**
 * @brief Handler for RGB LED controls by encoder and joystick.
 *
 * @param joy_sw State of joystick switch.
 * @param delta State of Encoder knob
 */
void rgb_led_handler(byte joy_sw, int delta)
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
#endif

/**
 * @brief Main loop for control panel.
 *
 */
void loop()
{
  // Read state of Enable pins
  byte enable_a = gpiosB->digitalRead(ENABLE_A_PIN);
  byte enable_b = gpiosB->digitalRead(ENABLE_B_PIN);

  if (true)
  {
    // Process paint app first.
#ifdef LCD_TFT_CONNECTED
    // Initialize shared pins to SPI mode
    pinMode(MISO, INPUT_PULLUP);
    pinMode(MOSI, OUTPUT);
    // if sharing pins, you'll need to fix the directions of the touchscreen pins
    pinMode(XM, OUTPUT);
    pinMode(YP, OUTPUT);

    loop_tft();

    // Restore SPI pins to GPIO mode
    pinMode(MISO, OUTPUT); // pin 12, LED_BAR_R by default
    pinMode(MOSI, INPUT);  // pin 11, LED_MATRIX_DIO_PIN by default
                           // No need to restore A4 as both SPI and rest of device is OUTPUT
#endif

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

        // Do some initialization work for different modes.
        if (mode == LED_MATRIX_MODE)
        {
          led_matrix.clear();
        }
        else if (mode == LED_BAR_MODE)
        {
          matrix_disp.clear();
          led_bar.off();
        }
      }
    }

    if (current_mode == RGB_LED_MODE)
    {
#ifdef RGB_LED_CONNECTED
      rgb_led_handler(joy_sw, delta);
#endif
      led_bar.move_bar(1);
    }
    else if (current_mode == LED_BAR_MODE)
    {
      led_bar.move_bar(delta);
    }
    else if (current_mode == LED_MATRIX_MODE)
    {
      led_matrix_handler();
    }
    else if (current_mode == LCD_MODE)
    {
    }

    if (character != NO_KEY)
    {
      // Update 7seg display
      char_disp.set_character(character);
      seven_seg.setSegments(char_disp.get_buffer(), DISPLAY_SIZE);
      Serial.println(character);
    }

    if (matrix_key != NO_KEY)
    {
      // Update LED matrix
      matrix_disp.set_character(matrix_key);
      led_matrix.setSegments(matrix_disp.get_buffer(), LED_MATRIX_SIZE);
      Serial.println(matrix_key);
    }
  }
#ifdef LCD_TFT_CONNECTED
  else
  {
    tft.fillScreen(BLACK);
  }
#endif
}
