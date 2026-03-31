
// Arduino code for graphical ignition timing indicator
// using 4-1 cam shaft sensor signal and ignition module signal
// using 128x32 monochrome adafruit OLED module for display
// including dwell and ignition times in the visualization
// using analog input pin to allow setting a dotted line visualizing expected timing value
// using push button to cycle in-between three different visualization methods

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define CAM_PIN 2 // pin for cam shaft sensor signal
#define IGN_PIN 3 // pin for ignition module signal
#define POT_PIN A0 // pin for analog input to set expected timing value
#define BTN_PIN 4 // pin for push button to cycle visualization methods

#define DEBOUNCE_TIME 50 // debounce time for button press, in milliseconds

volatile unsigned long cam_rise = 0; // time of last cam shaft sensor rising edge, in microseconds
volatile unsigned long cam_fall = 0; // time of last cam shaft sensor falling edge, in microseconds
volatile unsigned long last_cam_rise = 0;
volatile unsigned long ign_rise = 0; // time of last ignition module rising edge, in microseconds
volatile unsigned long ign_fall = 0; // time of last ignition module falling edge, in microseconds

volatile float dwell = 0; // dwell time, in milliseconds
volatile float ign = 0; // ignition time, in milliseconds
volatile float timing = 0; // ignition timing, in degrees before top dead center (BTDC)
volatile float deg_per_us = 0.0036f; // default for 10Hz if not measured

int mode = 0; // visualization mode: 0 for dwell and ign bars, 1 for timing line and dot, 2 for timing line and expected line
int pot_value = 0; // analog input value from potentiometer
float expected_timing = 0; // expected timing value, in degrees BTDC

unsigned long last_btn_press = 0; // time of last button press, in milliseconds

void cam_isr() {
  if (digitalRead(CAM_PIN) == HIGH) {
    unsigned long now = micros();
    if (last_cam_rise > 0) {
        unsigned long period = now - last_cam_rise;
        if (period > 0) deg_per_us = 360.0f / (period * 4.0f);
    }
    last_cam_rise = now;
    cam_rise = now;
  } else {
    cam_fall = micros();
    if (ign_rise > 0) {
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
  expected_timing = map(pot_value, 0, 1023, 10, 40);
  
  if (digitalRead(BTN_PIN) == LOW && millis() - last_btn_press > DEBOUNCE_TIME) {
    mode = (mode + 1) % 3;
    last_btn_press = millis();
    display.clearDisplay();
  }
  
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  switch (mode) {
    case 0:
      display.setCursor(0,0);
      display.print("Dwell: "); display.print(dwell); display.print("ms");
      display.setCursor(64,0);
      display.print("Ign: "); display.print(ign); display.print("ms");

      display.fillRect(32, 8, 16, (int)constrain(dwell * 5, 0, 24), SSD1306_WHITE);
      display.fillRect(80, 8, 16, (int)constrain(ign * 5, 0, 24), SSD1306_WHITE);
      break;
      
    case 1:
    case 2:
      display.setCursor(0,0);
      display.print("Timing: "); display.print(timing);

      int dot_x = 64 + (int)(timing - 25) * 2;
      dot_x = constrain(dot_x, 0, 127);
      display.drawLine(0, 24, 127, 24, SSD1306_WHITE);
      display.fillCircle(dot_x, 24, 2, SSD1306_WHITE);

      if (mode == 2) {
        int exp_x = 64 + (int)(expected_timing - 25) * 2;
        exp_x = constrain(exp_x, 0, 127);
        for(int y=8; y<32; y+=4) display.drawLine(exp_x, y, exp_x, y+2, SSD1306_WHITE);
      }
      break;
  }
  display.display();
  delay(10);
}
