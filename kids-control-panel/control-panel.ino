#include <Arduino.h>
#include "controlpanel_keypad.h"
#include "controlpanel_encoder.h"

// GPIO expanders
#define GPIOA_ADDR 0
#define GPIOB_ADDR 4
auto gpiosA = new Adafruit_MCP23017();
auto gpiosB = new Adafruit_MCP23017();

void _configurePins(Adafruit_MCP23017 *gpios, byte *pins, byte num_pins, byte mode)
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

void _configurePins(byte *pins, byte num_pins, byte mode)
{
  for (byte i = 0; i < num_pins; i++)
  {
    pinMode(pins[i], mode);
  }
}


// Pins for power on/off switches

// Pins for enable/disable touch buttons
#define ENABLEA_PIN A2
#define ENABLEB_PIN NULL

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
PanelKeypad alpha_keypad = PanelKeypad(gpiosA, makeKeymap(alphanumeric), rowPins, colPins, rows, cols);

// 4x4 keypad on GPIOA
char matrix[rows][cols] = {
    {'0', '1', '2', '3'},
    {'4', '5', '6', '7'},
    {'8', '9', 'A', 'B'},
    {'C', 'D', 'E', 'F'}};
byte matrixRowPins[rows] = {11, 10, 9, 8};
byte matrixColPins[cols] = {15, 14, 13, 12};
PanelKeypad matrix_keypad = PanelKeypad(gpiosA, makeKeymap(matrix), matrixRowPins, matrixColPins, rows, cols);

// 7-seg 4-digit display

// 8x8 LED matrix

// RGB LED

// Joystick
#define JOYSTICK_X A1
#define JOYSTICK_Y A0
#define JOYSTICK_SW 2

// Rotary Encoder on GPIOB
byte encoder_switch = 7;
#define ENCODER_PIN_A 5
#define ENCODER_PIN_B 6
PanelEncoder knob(gpiosB, ENCODER_PIN_A, ENCODER_PIN_B);

// Pushbuttons on GPIOB
const byte buttonsRows = 1;
const byte buttonsCols = 4;
char buttons[buttonsRows][buttonsCols] = {'X', 'Y', 'Z', '@'};
byte buttonRowPins[buttonsRows] = {0};
byte buttonColPins[buttonsCols] = {1, 2, 3, 4};
PanelKeypad button_keypad = PanelKeypad(gpiosB, makeKeymap(buttons), buttonRowPins, buttonColPins, buttonsRows, buttonsCols);

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

  _configurePins(gpiosA, colPins, cols, INPUT_PULLUP);
  _configurePins(gpiosA, rowPins, rows, INPUT_PULLUP);
  _configurePins(gpiosA, matrixColPins, cols, INPUT_PULLUP);
  _configurePins(gpiosA, matrixRowPins, rows, INPUT_PULLUP);
  // _configurePins(gpiosB, buttonColPins, buttonsCols, INPUT_PULLUP);
  // _configurePins(gpiosB, buttonRowPins, buttonsRows, INPUT_PULLUP);
  pinMode(JOYSTICK_SW, INPUT_PULLUP);
  _configurePins(gpiosB, &encoder_switch, 1, INPUT_PULLUP);
}

void loop()
{
  // Read state of Enable pins
  byte enable_a = digitalRead(ENABLEA_PIN);

  if (true)
  {
    // Poll aplhanumeric keypad, key matrix, buttons, joystick, encoder
    char alphanumeric_key = alpha_keypad.getKey();
    char matrix_key = matrix_keypad.getKey();
    char pushbutton = button_keypad.getKey();
    // int joy_x = analogRead(JOYSTICK_X);
    // int joy_y = analogRead(JOYSTICK_Y);
    byte joy_sw = digitalRead(JOYSTICK_SW);
    byte enc_sw = gpiosB->digitalRead(encoder_switch);

    if (alphanumeric_key != NO_KEY)
    {
      Serial.print("Keypad key = ");
      Serial.println(alphanumeric_key);
    }
    if (matrix_key != NO_KEY)
    {
      Serial.print("Matrix key = ");
      Serial.println(matrix_key);
    }
    if (pushbutton != NO_KEY)
    {
      Serial.print("Pushbutton = ");
      Serial.println(pushbutton);
    }
    // Serial.print("X = ");
    // Serial.print(joy_x);
    // Serial.print(" Y = ");
    // Serial.print(joy_y);
    // Serial.print("SW1 = ");
    // Serial.print(joy_sw);
    // Serial.print(" SW2 = ");
    // Serial.println(enc_sw);

    // Update 7seg display

    // Update LED matrix

    // Update RGB LED
  }
  delay(10);
}
