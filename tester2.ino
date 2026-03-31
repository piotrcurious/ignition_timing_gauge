
// Define the analog input pins for controlling the cam sensor frequency and the ignition timing
#define CAM_FREQ_PIN A0
#define IGN_TIMING_PIN A1

// Define the digital output pins for generating the cam sensor and the ignition module signals
#define CAM_OUT_PIN 2
#define IGN_OUT_PIN 3

// Define some constants for the signal generation and timing calculations
#define MIN_FREQ 10 // Minimum cam sensor frequency in Hz
#define MAX_FREQ 100 // Maximum cam sensor frequency in Hz
#define MIN_DWELL 1 // Minimum dwell time in milliseconds
#define MAX_DWELL 10 // Maximum dwell time in milliseconds
#define MIN_IGNITION 0 // Minimum ignition time in milliseconds
#define MAX_IGNITION 40 // Maximum ignition time in milliseconds
#define MIN_ANGLE -60 // Minimum ignition angle in degrees
#define MAX_ANGLE 60 // Maximum ignition angle in degrees

// Declare a lookup table for the dwell time based on the cam sensor frequency
const int dwellTable[10] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1}; // Dwell time in milliseconds for each frequency range

// Declare some global variables for storing the signal values and states
int camFreq = 0; // Cam sensor frequency in Hz
int ignTiming = 0; // Ignition timing in milliseconds
int dwellTime = 0; // Dwell time in milliseconds
int ignAngle = 0; // Ignition angle in degrees
unsigned long camPeriod = 0; // Cam sensor period in microseconds
unsigned long camHalfPeriod = 0; // Cam sensor half period in microseconds
unsigned long ignDelay = 0; // Ignition delay in microseconds
unsigned long camTimer = 0; // Timer for the cam sensor signal
unsigned long ignTimer = 0; // Timer for the ignition module signal
int camState = LOW; // Current state of the cam sensor signal
int ignState = LOW; // Current state of the ignition module signal
int camTransitions = 0;

void setup() {
  // Initialize serial communication for debugging purposes
  Serial.begin(9600);

  // Set up the digital output pins and set them to low initially
  pinMode(CAM_OUT_PIN, OUTPUT);
  pinMode(IGN_OUT_PIN, OUTPUT);
  digitalWrite(CAM_OUT_PIN, LOW);
  digitalWrite(IGN_OUT_PIN, LOW);

  camTimer = micros();
  ignTimer = micros();
}


void loop() {
  // Read the current values for the cam sensor frequency and the ignition timing from the analog input pins
  camFreq = map(analogRead(CAM_FREQ_PIN), 0, 1023, MIN_FREQ, MAX_FREQ);
  ignTiming = map(analogRead(IGN_TIMING_PIN), 0, 1023, MIN_IGNITION, MAX_IGNITION);

  // Calculate the cam sensor period and half period in microseconds
  camPeriod = 1000000 / camFreq;
  camHalfPeriod = camPeriod / 2;

  // Calculate the dwell time from the lookup table based on the cam sensor frequency
  dwellTime = dwellTable[constrain(map(camFreq, MIN_FREQ, MAX_FREQ, 0, 9), 0, 9)];

  // Calculate the ignition angle from the ignition timing in degrees
  ignAngle = map(ignTiming, MIN_IGNITION, MAX_IGNITION, MIN_ANGLE, MAX_ANGLE);

  // Calculate the ignition delay in microseconds from the ignition angle and the cam sensor period
  ignDelay = map(ignAngle, MIN_ANGLE, MAX_ANGLE, -camHalfPeriod, camHalfPeriod);

  // Get the current timestamp in microseconds
  unsigned long currentTime = micros();

  // Check if it's time to toggle the cam sensor signal
  if (currentTime - camTimer >= camHalfPeriod) {
    // Toggle the cam sensor signal state
    camState = !camState;
    digitalWrite(CAM_OUT_PIN, camState);
    camTimer = currentTime;

    // Increment the number of transitions
    camTransitions++;

    // 4-1 cam sensor: 8 transitions per cycle (4 pulses), but one is missing.
    // Let's say transitions 6 and 7 are missing.
    if (camTransitions >= 8) {
      camTransitions = 0;
    }
    
    // Simulate missing pulse on 4th pulse (transitions 6, 7)
    if (camTransitions == 6 || camTransitions == 7) {
        digitalWrite(CAM_OUT_PIN, LOW);
        camState = LOW;
    }
  }

  // Ignition logic synchronized with cam (e.g. dwell starts on transition 4, ends on transition 5)
  if (camTransitions == 4 && ignState == LOW) {
      ignState = HIGH;
      digitalWrite(IGN_OUT_PIN, HIGH);
  } else if (camTransitions == 5 && ignState == HIGH) {
      ignState = LOW;
      digitalWrite(IGN_OUT_PIN, LOW);
  }
}
