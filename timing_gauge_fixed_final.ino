
// Main multi-mode ignition timing gauge
// Features: Dwell/Ign times, BTDC timing, dynamic RPM, expected line.
// Supports 4-1 (missing tooth) and 4-equal cam wheels.
// Fixed and improved by Jules.

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define CAM_PIN 2
#define IGN_PIN 3
#define POT_PIN A0
#define BTN_PIN 4

#define DEBOUNCE_TIME 50
#define LONG_PRESS_TIME 1000

enum WheelType { WHEEL_4_1, WHEEL_4_EQUAL };
WheelType currentWheel = WHEEL_4_1;

volatile unsigned long cam_rise = 0;
volatile unsigned long cam_fall = 0;
volatile unsigned long last_cam_rise = 0;
volatile unsigned long last_valid_period = 0;
volatile unsigned long ign_rise = 0;
volatile unsigned long ign_fall = 0;
volatile float rpm = 0;
volatile float deg_per_us = 0.0036f;
volatile float dwell = 0;
volatile float ign = 0;
volatile float timing = 0;

int mode = 0;
int pot_value = 0;
float expected_timing = 0;
unsigned long last_btn_press = 0;
bool btn_prev_state = HIGH;

void cam_isr() {
  uint8_t state = digitalRead(CAM_PIN);
  unsigned long now = micros();
  if (state == HIGH) {
      if (last_cam_rise > 0) {
          unsigned long period = now - last_cam_rise;

          if (currentWheel == WHEEL_4_1) {
              // 4-1 wheel: 3 regular pulses, 1 missing.
              if (last_valid_period == 0 || period < last_valid_period * 1.5f) {
                  last_valid_period = period;
              }
              if (last_valid_period > 0) {
                  rpm = 60000000.0f / (last_valid_period * 4.0f);
                  deg_per_us = 360.0f / (last_valid_period * 4.0f);
              }
          } else {
              // 4 equal pulses
              last_valid_period = period;
              rpm = 60000000.0f / (period * 4.0f);
              deg_per_us = 360.0f / (period * 4.0f);
          }
      }
      last_cam_rise = now;
      cam_rise = now;
  } else {
      cam_fall = now;
      if (ign_rise > 0 && cam_fall > ign_rise) {
          timing = (float)(cam_fall - ign_rise) * deg_per_us;
      }
  }
}

void ign_isr() {
  uint8_t state = digitalRead(IGN_PIN);
  unsigned long now = micros();
  if (state == HIGH) {
    ign_rise = now;
    if (ign_fall > 0) dwell = (float)(ign_rise - ign_fall) / 1000.0f;
  } else {
    ign_fall = now;
    if (ign_rise > 0) ign = (float)(ign_fall - ign_rise) / 1000.0f;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(CAM_PIN, INPUT_PULLUP);
  pinMode(IGN_PIN, INPUT_PULLUP);
  pinMode(POT_PIN, INPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(CAM_PIN), cam_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(IGN_PIN), ign_isr, CHANGE);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;);
  }
  display.clearDisplay();
  display.display();
}

void loop() {
  pot_value = analogRead(POT_PIN);
  expected_timing = (float)map(pot_value, 0, 1023, 10, 40);

  bool btn_state = digitalRead(BTN_PIN);
  if (btn_state == LOW && btn_prev_state == HIGH) {
      last_btn_press = millis();
  } else if (btn_state == HIGH && btn_prev_state == LOW) {
      unsigned long duration = millis() - last_btn_press;
      if (duration > LONG_PRESS_TIME) {
          currentWheel = (currentWheel == WHEEL_4_1) ? WHEEL_4_EQUAL : WHEEL_4_1;
          display.clearDisplay();
          display.setCursor(0,10);
          display.print("Wheel: ");
          display.print(currentWheel == WHEEL_4_1 ? "4-1" : "4 Equal");
          display.display();
          delay(1000);
      } else if (duration > DEBOUNCE_TIME) {
          mode = (mode + 1) % 3;
      }
  }
  btn_prev_state = btn_state;

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if (mode == 0) {
    display.setCursor(0,0);
    display.print("Dwell: "); display.print(dwell); display.print("ms");
    display.setCursor(0,12);
    display.print("Ign:   "); display.print(ign); display.print("ms");
    display.setCursor(80,0);
    display.print(currentWheel == WHEEL_4_1 ? "4-1" : "4Eq");

    int d_w = (int)(dwell * 10.0f);
    int i_w = (int)(ign * 10.0f);
    display.fillRect(0, 24, (d_w > 127 ? 127 : (d_w < 0 ? 0 : d_w)), 3, SSD1306_WHITE);
    display.fillRect(0, 28, (i_w > 127 ? 127 : (i_w < 0 ? 0 : i_w)), 3, SSD1306_WHITE);
  } else {
    display.setCursor(0,0);
    display.print("Timing: "); display.print(timing); display.print(" deg");
    display.setCursor(0,10);
    display.print("RPM:    "); display.print((int)rpm);
    display.setCursor(80,10);
    display.print(currentWheel == WHEEL_4_1 ? "4-1" : "4Eq");

    int x_center = 64;
    int dot_x = x_center + (int)(timing - 25.0f) * 2;
    if (dot_x > 127) dot_x = 127; if (dot_x < 0) dot_x = 0;

    display.drawLine(0, 24, 127, 24, SSD1306_WHITE);
    display.fillCircle(dot_x, 24, 3, SSD1306_WHITE);

    if (mode == 2) {
        int exp_x = x_center + (int)(expected_timing - 25.0f) * 2;
        if (exp_x > 127) exp_x = 127; if (exp_x < 0) exp_x = 0;
        for(int y=16; y<32; y+=4) display.drawLine(exp_x, y, exp_x, y+2, SSD1306_WHITE);
    }
  }

  display.display();
  delay(50);
}
