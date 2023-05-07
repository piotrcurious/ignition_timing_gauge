
// Include the Adafruit OLED library
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define the OLED display size and pins
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C // See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

// Create an object for the OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Define the analog input pin for setting the expected timing value
#define ANALOG_PIN A0

// Define the digital input pins for the cam shaft sensor and the ignition module signals
#define CAM_PIN 2
#define IGN_PIN 3

// Define some constants for the timing calculations and display
#define MAX_DWELL 10 // Maximum dwell time in milliseconds
#define MAX_IGNITION 40 // Maximum ignition time in milliseconds
#define MAX_ANGLE 60 // Maximum ignition angle in degrees
#define SCALE_FACTOR 2 // Scale factor for converting milliseconds to pixels
#define BASELINE_Y 16 // Y coordinate of the baseline on the display
#define EXPECTED_Y 8 // Y coordinate of the expected timing line on the display

// Declare some global variables for storing the timing values and states
unsigned long dwellStart = 0; // Timestamp of the dwell start
unsigned long dwellEnd = 0; // Timestamp of the dwell end
unsigned long ignitionStart = 0; // Timestamp of the ignition start
unsigned long ignitionEnd = 0; // Timestamp of the ignition end
unsigned long dwellTime = 0; // Dwell time in milliseconds
unsigned long ignitionTime = 0; // Ignition time in milliseconds
int ignitionAngle = 0; // Ignition angle in degrees
int expectedAngle = 0; // Expected angle in degrees
int camState = LOW; // Current state of the cam shaft sensor signal
int ignState = LOW; // Current state of the ignition module signal

// Declare some functions for handling the interrupts and updating the display
void camISR(); // Interrupt service routine for the cam shaft sensor signal
void ignISR(); // Interrupt service routine for the ignition module signal
void updateDisplay(); // Function for updating the OLED display

void setup() {
  // Initialize serial communication for debugging purposes
  Serial.begin(9600);

  // Initialize the OLED display and clear it
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  display.clearDisplay();

  // Set up the analog input pin and read the initial expected angle value
  pinMode(ANALOG_PIN, INPUT);
  expectedAngle = map(analogRead(ANALOG_PIN), 0, 1023, -MAX_ANGLE, MAX_ANGLE);

  // Set up the digital input pins and attach the interrupt handlers
  pinMode(CAM_PIN, INPUT);
  pinMode(IGN_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(CAM_PIN), camISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(IGN_PIN), ignISR, CHANGE);
}


// Function for updating the OLED display
void updateDisplay() {
  // Clear the display
  display.clearDisplay();

  // Draw the baseline and the expected timing line
  display.drawLine(0, BASELINE_Y, SCREEN_WIDTH - 1, BASELINE_Y, WHITE);
  display.drawLine(0, EXPECTED_Y, SCREEN_WIDTH - 1, EXPECTED_Y, WHITE);

  // Draw the dwell and ignition bars
  display.fillRect(SCREEN_WIDTH - dwellTime * SCALE_FACTOR, BASELINE_Y - dwellTime * SCALE_FACTOR / 2, dwellTime * SCALE_FACTOR, dwellTime * SCALE_FACTOR / 2, WHITE);
  display.fillRect(SCREEN_WIDTH - ignitionTime * SCALE_FACTOR, BASELINE_Y + ignitionTime * SCALE_FACTOR / 2, ignitionTime * SCALE_FACTOR, ignitionTime * SCALE_FACTOR / 2, WHITE);

  // Draw the ignition angle text
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, BASELINE_Y + 2);
  display.print("Ignition angle: ");
  display.print(ignitionAngle);
  display.print(" deg");

  // Display the buffer
  display.display();
}


// Interrupt service routine for the ignition module signal
void ignISR() {
  // Read the current state of the ignition module signal
  ignState = digitalRead(IGN_PIN);

  // If the signal goes from low to high, it means the dwell has started
  if (ignState == HIGH) {
    // Record the dwell start timestamp
    dwellStart = micros();
  }

  // If the signal goes from high to low, it means the dwell has ended and the ignition has started
  if (ignState == LOW) {
    // Record the dwell end and ignition start timestamps
    dwellEnd = micros();
    ignitionStart = micros();

    // Calculate the dwell time in milliseconds
    dwellTime = (dwellEnd - dwellStart) / 1000;

    // Constrain the dwell time to the maximum value
    dwellTime = constrain(dwellTime, 0, MAX_DWELL);
  }

  // If the signal goes from low to high again, it means the ignition has ended
  if (ignState == HIGH) {
    // Record the ignition end timestamp
    ignitionEnd = micros();

    // Calculate the ignition time in milliseconds
    ignitionTime = (ignitionEnd - ignitionStart) / 1000;

    // Constrain the ignition time to the maximum value
    ignitionTime = constrain(ignitionTime, 0, MAX_IGNITION);

    // Calculate the ignition angle in degrees
    ignitionAngle = map(ignitionTime, 0, MAX_IGNITION, -MAX_ANGLE, MAX_ANGLE);
  }
}

// Declare a global variable for storing the update flag
bool updateFlag = false; // Flag to indicate whether the display needs to be updated

// Interrupt service routine for the cam shaft sensor signal
void camISR() {
  // Read the current state of the cam shaft sensor signal
  camState = digitalRead(CAM_PIN);

  // If the signal goes from low to high, it means a new cycle has started
  if (camState == HIGH) {
    // Set the update flag to true
    updateFlag = true;

    // Reset the dwell and ignition timestamps and values
    dwellStart = 0;
    dwellEnd = 0;
    ignitionStart = 0;
    ignitionEnd = 0;
    dwellTime = 0;
    ignitionTime = 0;
    ignitionAngle = 0;

    // Read the current expected angle value from the analog input pin
    expectedAngle = map(analogRead(ANALOG_PIN), 0, 1023, -MAX_ANGLE, MAX_ANGLE);
  }
}

void loop() {
  // Check if the update flag is true
  if (updateFlag) {
    // Update the display with the previous cycle's timing values
    updateDisplay();

    // Set the update flag to false
    updateFlag = false;
  }
}
