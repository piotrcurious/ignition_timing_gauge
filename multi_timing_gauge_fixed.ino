
// Arduino code for graphical ignition timing indicator
// Visualizes ignition timing in relation to cam shaft sensor.
// Created by BingAI, fixed by Jules.

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
volatile unsigned long ign_rise = 0;
volatile unsigned long ign_fall = 0;

volatile float dwell = 0;
volatile float ign = 0;
volatile float timing = 0;

int mode = 0;
int pot_value = 0;
float expected_timing = 0;
unsigned long last_btn_press = 0;

void cam_isr();
void ign_isr();

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

void draw_gauge() {
  display.clearDisplay();
  switch (mode) {
    case 0: {
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0,0);
      display.print("Dwell: "); display.print(dwell); display.print("ms");
      display.setCursor(64,0);
      display.print("Ign: "); display.print(ign); display.print("ms");

      int dwell_bar = map(dwell * 100, 0, 2000, 0, SCREEN_HEIGHT - 8);
      int ign_bar = map(ign * 100, 0, 2000, 0, SCREEN_HEIGHT - 8);
      dwell_bar = constrain(dwell_bar, 0, SCREEN_HEIGHT - 8);
      ign_bar = constrain(ign_bar, 0, SCREEN_HEIGHT - 8);
      display.fillRect(32, 8, 16, dwell_bar, SSD1306_WHITE);
      display.fillRect(80, 8, 16, ign_bar, SSD1306_WHITE);
      break;
    }
    case 1:
    case 2: {
      // Timing visualization
      int x_center = 64;
      int dot_x = x_center + map(timing * 10, -400, 100, -60, 60);
      dot_x = constrain(dot_x, 0, 127);
      display.drawLine(0, 24, 127, 24, SSD1306_WHITE);
      display.fillCircle(dot_x, 24, 3, SSD1306_WHITE);

      if (mode == 2) {
        int exp_x = x_center + map(expected_timing * 10, -400, 100, -60, 60);
        exp_x = constrain(exp_x, 0, 127);
        for(int y=0; y<32; y+=4) display.drawLine(exp_x, y, exp_x, y+2, SSD1306_WHITE);
      }
      display.setCursor(0,0);
      display.print("Timing: "); display.print(timing);
      break;
    }
  }
  display.display();
}

void loop() {
  pot_value = analogRead(POT_PIN);
  expected_timing = map(pot_value, 0, 1023, -40, -10);

  if (digitalRead(BTN_PIN) == LOW && millis() - last_btn_press > DEBOUNCE_TIME) {
    mode = (mode + 1) % 3;
    last_btn_press = millis();
  }
  draw_gauge();
  delay(10);
}

void cam_isr() {
  if (digitalRead(CAM_PIN) == HIGH) {
    cam_rise = micros();
  } else {
    cam_fall = micros();
    // Assuming 4-1 wheel, this logic needs to be more robust in real life
    // but for the gauge we use the last ignition vs cam relationship
    if (ign_rise > 0) {
        // Simple timing calculation: time between cam sync and ignition
        // This is a placeholder for actual engine position logic
        timing = (float)(cam_fall - ign_rise) * 0.036f;
    }
  }
}

void ign_isr() {
  if (digitalRead(IGN_PIN) == HIGH) {
    ign_rise = micros();
    if (ign_fall > 0) dwell = (ign_rise - ign_fall) / 1000.0f;
  } else {
    ign_fall = micros();
    if (ign_rise > 0) ign = (ign_fall - ign_rise) / 1000.0f;
  }
}
