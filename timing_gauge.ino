
// Include the Adafruit graphics and OLED libraries
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Define the OLED display width and height
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// Create an object for the OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT);

// Define the pins for the camshaft sensor and ignition module signals
#define CAM_PIN 2 // The camshaft sensor signal is connected to pin 2
#define IGN_PIN 3 // The ignition module signal is connected to pin 3

// Define some variables for the timing calculation
volatile unsigned long camTime = 0; // The time of the last camshaft sensor pulse in microseconds
volatile unsigned long ignTime = 0; // The time of the last ignition module pulse in microseconds
volatile int timing = 0; // The ignition timing in degrees

// Define some constants for the timing calculation
#define DEG_PER_MICROSECOND 0.006 // The degrees per microsecond for a 4-1 camshaft sensor
#define MAX_TIMING 60 // The maximum timing in degrees to display
#define MIN_TIMING -60 // The minimum timing in degrees to display

// Define some constants for the OLED display
#define BAR_WIDTH 2 // The width of each bar in pixels
#define BAR_SPACING 1 // The spacing between each bar in pixels
#define BAR_COLOR WHITE // The color of the bars
#define BAR_OFFSET 16 // The offset of the bars from the center of the screen in pixels
#define TEXT_COLOR WHITE // The color of the text
#define TEXT_SIZE 1 // The size of the text

// This function is called when a camshaft sensor pulse is detected
void camISR() {
  camTime = micros(); // Record the current time in microseconds
}

// This function is called when an ignition module pulse is detected
void ignISR() {
  ignTime = micros(); // Record the current time in microseconds
  timing = (ignTime - camTime) * DEG_PER_MICROSECOND; // Calculate the ignition timing in degrees
}

void setup() {
  // Initialize serial communication for debugging
  Serial.begin(9600);

  // Initialize the OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Initialize with I2C address 0x3C
  display.clearDisplay(); // Clear the display buffer

  // Attach interrupt handlers for the camshaft sensor and ignition module signals
  attachInterrupt(digitalPinToInterrupt(CAM_PIN), camISR, RISING); // Trigger on rising edge of camshaft sensor signal
  attachInterrupt(digitalPinToInterrupt(IGN_PIN), ignISR, RISING); // Trigger on rising edge of ignition module signal

}

void loop() {
  // Print the timing value for debugging
  Serial.println(timing);

  // Draw the timing indicator on the OLED display
  display.clearDisplay(); // Clear the display buffer

  // Draw a horizontal line at the center of the screen
  display.drawLine(0, SCREEN_HEIGHT / 2, SCREEN_WIDTH - 1, SCREEN_HEIGHT / 2, TEXT_COLOR);

  // Draw a vertical line at the center of the screen
  display.drawLine(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT - 1, TEXT_COLOR);

  // Draw a bar representing the timing value
  int barHeight = map(abs(timing), MIN_TIMING, MAX_TIMING, BAR_OFFSET, SCREEN_HEIGHT / 2 - BAR_OFFSET); // Map the timing value to a bar height within limits
  int barX = SCREEN_WIDTH / 2 + (timing > 0 ? BAR_SPACING : -BAR_WIDTH - BAR_SPACING); // Calculate the x position of the bar depending on the sign of timing value
  int barY = SCREEN_HEIGHT / 2 - barHeight; // Calculate the y position of the bar from the center of the screen

  // Draw the bar on the display
  display.fillRect(barX, barY, BAR_WIDTH, barHeight, BAR_COLOR);

  // Print the timing value on the display
  display.setTextSize(TEXT_SIZE);
  display.setTextColor(TEXT_COLOR);
  display.setCursor(SCREEN_WIDTH / 2 + (timing > 0 ? BAR_WIDTH + 2 * BAR_SPACING : -BAR_WIDTH - 4 * BAR_SPACING - 3 * TEXT_SIZE * 6), SCREEN_HEIGHT / 2 - TEXT_SIZE * 4);
  display.print(timing);
  display.print(" deg");

  // Display the buffer on the screen
  display.display();
}


//Source: Conversation with Bing, 5/7/2023
//(1) Timing Light and Tachometer With Arduino : 6 Steps - Instructables. https://www.instructables.com/Timing-Light-and-Tachometer-With-Arduino/.
//(2) Arduino ignition timing controller - YouTube. https://www.youtube.com/watch?v=vK0NQlnKV9w.
//(3) (PDF) Ignition Timing and Fuel Injection Timing Control using Arduino .... https://www.researchgate.net/publication/348110266_Ignition_Timing_and_Fuel_Injection_Timing_Control_using_Arduino_and_Control_Drivers.
//(4) Camshaft position sensor - function & troubleshooting | HELLA. https://www.hella.com/techworld/uk/Technical/Sensors-and-actuators/Camshaft-position-sensor-3899/.
//(5) Camshaft position sensor - function & troubleshooting | HELLA. https://www.hella.com/techworld/us/Technical/Sensors-and-actuators/Camshaft-position-sensor-3899/.
//(6) Crankshaft and camshaft position sensor comparison - Pico auto. https://www.picoauto.com/library/automotive-guided-tests/cam-and-crank-sensors/.
//(7) What Does the Ignition Control Module Do? Symptoms, Replacement Cost. https://www.carparts.com/blog/what-does-the-ignition-control-module-do-symptoms-replacement-cost/.
//(8) What is an Ignition Module? - crankSHIFT. http://www.crankshift.com/ignition-module/.
//(9) Ignition controller - EDIS (Ford) - reference and timing - Pico auto. https://www.picoauto.com/library/automotive-guided-tests/edis-unit-pip-saw-signals.
//(10) Symptoms Of Bad Ignition Control Module - Time To Replace It?. https://www.motorverso.com/symptoms-of-bad-ignition-control-module/.
