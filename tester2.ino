
// Refined signal generator
// Supports 4-1 and 4-equal cam wheel signals.
// Use BTN_PIN (4) to toggle wheel type.

#define CAM_FREQ_PIN A0
#define IGN_TIMING_PIN A1
#define BTN_PIN 4

#define CAM_OUT_PIN 2
#define IGN_OUT_PIN 3

#define MIN_FREQ 10
#define MAX_FREQ 100
#define DEBOUNCE_TIME 50

int camFreq = 10;
unsigned long camPeriod = 100000;
unsigned long camHalfPeriod = 50000;
unsigned long camTimer = 0;
int camState = LOW;
int camTransitions = 0;
int wheelType = 0; // 0: 4-1, 1: 4-equal

unsigned long last_btn_press = 0;
bool btn_prev_state = HIGH;

void setup() {
  Serial.begin(9600);
  pinMode(CAM_OUT_PIN, OUTPUT);
  pinMode(IGN_OUT_PIN, OUTPUT);
  pinMode(BTN_PIN, INPUT_PULLUP);
  camTimer = micros();
}

void loop() {
  camFreq = map(analogRead(CAM_FREQ_PIN), 0, 1023, MIN_FREQ, MAX_FREQ);
  camPeriod = 1000000 / camFreq;
  camHalfPeriod = camPeriod / 2;

  bool btn_state = digitalRead(BTN_PIN);
  if (btn_state == LOW && btn_prev_state == HIGH) {
      wheelType = !wheelType;
      Serial.print("Wheel type: "); Serial.println(wheelType ? "4-equal" : "4-1");
      delay(DEBOUNCE_TIME);
  }
  btn_prev_state = btn_state;

  unsigned long currentTime = micros();

  if (currentTime - camTimer >= camHalfPeriod) {
    camState = !camState;
    camTimer = currentTime;
    camTransitions++;
    if (camTransitions >= 8) camTransitions = 0;
    
    int actualCamState = camState;
    if (wheelType == 0) { // 4-1
        if (camTransitions == 6 || camTransitions == 7) actualCamState = LOW;
    }
    digitalWrite(CAM_OUT_PIN, actualCamState);
  }

  // Simple ignition pulse synchronized with cam
  if (camTransitions == 4) digitalWrite(IGN_OUT_PIN, HIGH);
  else if (camTransitions == 5) digitalWrite(IGN_OUT_PIN, LOW);
}
