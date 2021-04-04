#include <Arduino.h>
#include <Key.h>
#include <Keypad.h>

#include <Adafruit_MCP23017.h>
#include "controlpanel.h"

using namespace controlpanel;

#define ENABLE_A_PIN 1
#define ENABLE_B_PIN 2

auto led_bar = new LedBar();
const byte rows = 4; //four rows
const byte cols = 4; //three columns
char keys[rows][cols] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}};
byte rowPins[rows] = {13, 12, 11, 10}; //connect to the row pinouts of the keypad
byte colPins[cols] = {9, 8, 7, 6};     //connect to the column pinouts of the keypad
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, cols);

void setup()
{
  Wire.begin();
  Serial.begin(115200);
  while (!Serial)
    ; // Leonardo: wait for serial monitor

  // Initialize LED bar pins
  led_bar->begin(2, 4, 3, 5);

  //pinMode(ENABLE_A_PIN, INPUT);
  //pinMode(ENABLE_B_PIN, INPUT);
}

void loop()
{
  //uint8_t enable_a = digitalRead(ENABLE_A_PIN);
  //uint8_t enable_b = digitalRead(ENABLE_B_PIN);

  uint8_t enable_a = 1;
  uint8_t enable_b = 1;

  if (enable_a and enable_b)
  {
    char key = keypad.getKey();

    if (key != NO_KEY)
    {
      Serial.println(key);
    }
    
    for (uint8_t c = 0; c < led_bar->max_color; c++)
    {
      led_bar->color(c, true);
      delay(1000);
    }
    for (uint8_t c = 0; c < led_bar->max_color; c++)
    {
      led_bar->color(c, false);
      delay(1000);
    }
  }
  else
  {
    led_bar->off();
  }
}
