
// Fully functional Arduino code for graphical ignition timing indicator
// Visualizes ignition timing in relation to cam shaft sensor.
// Handles 4-1 wheel signal.
// Created by Jules.

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

volatile unsigned long cam_rise = 0;
volatile unsigned long cam_fall = 0;
volatile unsigned long last_cam_rise = 0;
volatile unsigned long ign_rise = 0;
volatile unsigned long ign_fall = 0;
volatile float rpm = 0;

volatile float dwell = 0;
volatile float ign = 0;
volatile float timing = 0;
volatile float deg_per_us = 0.0036f;

int mode = 0;
int pot_value = 0;
float expected_timing = 0;
unsigned long last_btn_press = 0;

void cam_isr() {
  if (digitalRead(CAM_PIN) == HIGH) {
      unsigned long now = micros();
      if (last_cam_rise > 0) {
          unsigned long period = now - last_cam_rise;
          if (period > 0) {
              rpm = 60000000.0f / (period * 4.0f);
              deg_per_us = 360.0f / (period * 4.0f);
          }
      }
      last_cam_rise = now;
      cam_rise = now;
  } else {
      cam_fall = micros();
      if (ign_rise > 0 && cam_fall > ign_rise) {
          timing = (float)(cam_fall - ign_rise) * deg_per_us;
      }
  }
}

void ign_isr() {
  if (digitalRead(IGN_PIN) == HIGH) {
    ign_rise = micros();
    if (ign_fall > 0) dwell = (float)(ign_rise - ign_fall) / 1000.0f;
  } else {
    ign_fall = micros();
    if (ign_rise > 0) ign = (float)(ign_fall - ign_rise) / 1000.0f;
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(CAM_PIN, INPUT);
  pinMode(IGN_PIN, INPUT);
  pinMode(POT_PIN, INPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(CAM_PIN), cam_isr, CHANGE);
  attachInterrupt(digitalPinToInterrupt(IGN_PIN), ign_isr, CHANGE);

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    for(;;);
  }
  display.clearDisplay();
}

void loop() {
  pot_value = analogRead(POT_PIN);
  expected_timing = map(pot_value, 0, 1023, 10, 40); // 10 to 40 deg BTDC

  if (digitalRead(BTN_PIN) == LOW && millis() - last_btn_press > DEBOUNCE_TIME) {
    mode = (mode + 1) % 3;
    last_btn_press = millis();
    display.clearDisplay();
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  if (mode == 0) {
    display.setCursor(0,0);
    display.print("Dwell: "); display.print(dwell); display.print("ms");
    display.setCursor(0,10);
    display.print("Ign:   "); display.print(ign); display.print("ms");

    int d_w = (int)dwell * 10;
    int i_w = (int)ign * 10;
    display.fillRect(0, 20, constrain(d_w, 0, 127), 4, SSD1306_WHITE);
    display.fillRect(0, 26, constrain(i_w, 0, 127), 4, SSD1306_WHITE);
  } else {
    display.setCursor(0,0);
    display.print("Timing: "); display.print(timing); display.print(" deg");
    display.setCursor(80,0);
    display.print("RPM:"); display.print((int)rpm);

    int x_center = 64;
    int dot_x = x_center + (int)(timing - 25) * 2;
    dot_x = constrain(dot_x, 0, 127);

    display.drawLine(0, 22, 127, 22, SSD1306_WHITE);
    display.fillCircle(dot_x, 22, 3, SSD1306_WHITE);

    if (mode == 2) {
        int exp_x = x_center + (int)(expected_timing - 25) * 2;
        exp_x = constrain(exp_x, 0, 127);
        for(int y=10; y<32; y+=4) display.drawLine(exp_x, y, exp_x, y+2, SSD1306_WHITE);
    }
  }

  display.display();
  delay(30);
}
