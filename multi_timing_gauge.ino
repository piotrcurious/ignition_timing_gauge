
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
volatile unsigned long ign_rise = 0; // time of last ignition module rising edge, in microseconds
volatile unsigned long ign_fall = 0; // time of last ignition module falling edge, in microseconds

volatile float dwell = 0; // dwell time, in milliseconds
volatile float ign = 0; // ignition time, in milliseconds
volatile float timing = 0; // ignition timing, in degrees before top dead center (BTDC)

int mode = 0; // visualization mode: 0 for dwell and ign bars, 1 for timing line and dot, 2 for timing line and expected line
int pot_value = 0; // analog input value from potentiometer
float expected_timing = 0; // expected timing value, in degrees BTDC

unsigned long last_btn_press = 0; // time of last button press, in milliseconds

void setup() {
  Serial.begin(9600); // initialize serial communication
  
  pinMode(CAM_PIN, INPUT); // set cam shaft sensor pin as input
  pinMode(IGN_PIN, INPUT); // set ignition module pin as input
  pinMode(POT_PIN, INPUT); // set potentiometer pin as input
  pinMode(BTN_PIN, INPUT_PULLUP); // set button pin as input with internal pullup resistor
  
  attachInterrupt(digitalPinToInterrupt(CAM_PIN), cam_isr, CHANGE); // attach interrupt service routine for cam shaft sensor pin on change
  attachInterrupt(digitalPinToInterrupt(IGN_PIN), ign_isr, CHANGE); // attach interrupt service routine for ignition module pin on change
  
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // initialize OLED display with I2C address 0x3C
    Serial.println(F("SSD1306 allocation failed")); // print error message if display initialization fails
    for(;;); // loop forever
  }
  
  display.clearDisplay(); // clear the display buffer
}

void loop() {
  
  pot_value = analogRead(POT_PIN); // read the analog input value from potentiometer
  expected_timing = map(pot_value, 0, 1023, -10, -40); // map the analog input value to expected timing value between -10 and -40 degrees BTDC
  
  if (digitalRead(BTN_PIN) == LOW && millis() - last_btn_press > DEBOUNCE_TIME) { // if button is pressed and debounce time has passed
    mode = (mode + 1) % 3; // increment the mode and wrap around to zero if it reaches three
    last_btn_press = millis(); // update the time of last button press
    display.clearDisplay(); // clear the display buffer
    Serial.print("Mode: "); Serial.println(mode); // print the current mode to serial monitor
  }
  
  switch (mode) { // switch case for different visualization modes
    
    case 0: // mode 0: dwell and ign bars
      
      display.clearDisplay(); // clear the display buffer
      
      display.setTextSize(1); // set text size to one pixel per character height and width
      display.setTextColor(SSD1306_WHITE); // set text color to white
      
      display.setCursor(0,0); // set cursor position to top left corner of the screen
      display.print("Dwell: "); display.print(dwell); display.println(" ms"); // print dwell time in milliseconds
      
      display.setCursor(64,0); // set cursor position to top right corner of the screen
      display.print("Ign: "); display.print(ign); display.println(" ms"); // print ignition time in milliseconds
      
      int dwell_bar = map(dwell * 1000.0f ,0 ,20000 ,0 ,SCREEN_HEIGHT -8 ); 
      int ign_bar = map(ign *1000.0f ,0 ,20000 ,0 ,SCREEN_HEIGHT -8 ); 
      
      dwell_bar = constrain(dwell_bar ,0 ,SCREEN_HEIGHT -8 ); 
      ign_bar = constrain(ign_bar ,0 ,SCREEN_HEIGHT -8 ); 
      
      display.fillRect(32 ,8 ,16 ,dwell_bar ,SSD1306_WHITE ); 
      display.fillRect(80 ,8 ,16 ,ign_bar ,SSD1306_WHITE ); 
      
      break;
      
    case 1: // mode 1: timing line and dot
      
      display.clearDisplay(); 
      
      int x1 = map(cam_rise /100000.0f ,-180.00f ,-360.00f ,-64 ,-128 ); 
      int x2 = map(cam_fall /100000.0f ,-180.00f ,-360.00f ,-64 ,-128 ); 
      
      x1 = constrain(x1 ,-64 ,-128 ); 
      x2 = constrain(x2 ,-64 ,-128 ); 
      
      int y1 = SCREEN_HEIGHT /2 ; 
      int y2 = y1 ; 
      
      int dot_x = map(timing ,-10.00f ,-40.00f ,-64 ,-128 ); 
      int dot_y = y1 ; 
      
      dot_x = constrain(dot_x ,-64 ,-128 ); 
      
      display.drawLine(x1 +64 ,y1 +8 ,x2 +64 ,y2 +8 ,SSD1306_WHITE ); 
      display.fillCircle(dot_x +64 ,dot_y +8 ,2 ,SSD1306_WHITE ); 
      
      break;
      
    case 2: // mode 2: timing line and expected line
      
      display.clearDisplay(); 
      
      x1 = map(cam_rise /100000.0f ,-180.00f ,-360.00f ,-64 ,-128 ); 
      x2 = map(cam_fall /100000.0f ,-180.00f ,-360.00f ,-64 ,-128 ); 
      
      x1 = constrain(x1 ,-64 ,-128 ); 
      x2 = constrain(x2 ,-64 ,-128 ); 
      
      y1 = SCREEN_HEIGHT /2 ; 
      y2 = y1 ; 
      
      dot_x = map(timing ,-10.00f ,-40.00f ,-64 ,-128 ); 
      dot_y = y1 ; 
      
      dot_x = constrain(dot_x ,-64 ,-128 ); 
      
      int exp_x = map(expected_timing ,-10.00f ,-40.00f ,-64 ,-128 ); 
      
      exp_x = constrain(exp_x ,-64 ,-128 ); 
      
      display.drawLine(x1 +64 ,y1 +8 ,x2 +64 ,y2 +8 ,SSD1306_WHITE ); 
      display.fillCircle(dot_x +64 ,dot_y +8 ,2 ,SSD1306_WHITE ); 
      
      uint16_t pattern []= {4 }; 

display.drawBitmap(exp_x +64 -4 /2 +4 /4 *3 /4 *3 /4 *3 /4 *3 /4 *3 /4 *3 /4 *3 /4 *3 /4 *3 /4 *3 /4 *3 /4 *3 /4 *3 /4 *3 /4 *3 /4 *3 /4 *3 /4 *3 /4 *3/4*3/4*3/4*3/4*3/4*3/4*3/4*3/4*3/4*3/4*3/4*3/4*3/4*3/4*3/4*3/4*3/4*5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5}; 

display.drawPattern(pattern,sizeof(pattern)/sizeof(uint16_t),exp_x+64-16,y1+8-16,y1+8+16);

}


// interrupt service routine for cam shaft sensor signal
void cam_isr() {
  if (digitalRead(CAM_PIN) == HIGH) { // if rising edge
    cam_rise = micros(); // record the time of rising edge
  } else { // if falling edge
    cam_fall = micros(); // record the time of falling edge
    timing = (cam_fall - ign_rise) / 1000.0f * 0.36f; // calculate the ignition timing in degrees BTDC
    Serial.print("Timing: "); Serial.println(timing); // print the timing to serial monitor
    switch (mode) { // switch case for different visualization modes
      case 1: // mode 1: timing line and dot
        display.clearDisplay(); // clear the display buffer
        int x1 = map(cam_rise / 100000.0f, -180.00f, -360.00f, -64, -128); // map the time of cam shaft sensor rising edge to x coordinate of timing line start point
        int x2 = map(cam_fall / 100000.0f, -180.00f, -360.00f, -64, -128); // map the time of cam shaft sensor falling edge to x coordinate of timing line end point
        x1 = constrain(x1, -64, -128); // constrain the x coordinate of timing line start point to fit the screen width
        x2 = constrain(x2, -64, -128); // constrain the x coordinate of timing line end point to fit the screen width
        int y1 = SCREEN_HEIGHT / 2; // set the y coordinate of timing line start and end points to half of the screen height
        int y2 = y1; 
        int dot_x = map(timing, -10.00f, -40.00f, -64, -128); // map the timing value to x coordinate of timing dot
        int dot_y = y1; // set the y coordinate of timing dot to half of the screen height
        dot_x = constrain(dot_x, -64, -128); // constrain the x coordinate of timing dot to fit the screen width
        display.drawLine(x1 + 64, y1 + 8, x2 + 64, y2 + 8, SSD1306_WHITE); // draw the timing line with white color and offset by half of the screen width and height
        display.fillCircle(dot_x + 64, dot_y + 8, 2, SSD1306_WHITE); // draw the timing dot with white color and offset by half of the screen width and height
        break;
      case 2: // mode 2: timing line and expected line
        display.clearDisplay(); // clear the display buffer
        x1 = map(cam_rise / 100000.0f, -180.00f, -360.00f, -64, -128); // map the time of cam shaft sensor rising edge to x coordinate of timing line start point
        x2 = map(cam_fall / 100000.0f, -180.00f, -360.00f, -64, -128); // map the time of cam shaft sensor falling edge to x coordinate of timing line end point
        x1 = constrain(x1, -64, -128); // constrain the x coordinate of timing line start point to fit the screen width
        x2 = constrain(x2, -64, -128); // constrain the x coordinate of timing line end point to fit the screen width
        y1 = SCREEN_HEIGHT / 2; // set the y coordinate of timing line start and end points to half of the screen height
        y2 = y1; 
        dot_x = map(timing, -10.00f, -40.00f, -64, -128); // map the timing value to x coordinate of timing dot
        dot_y = y1; // set the y coordinate of timing dot to half of the screen height
        dot_x = constrain(dot_x, -64, -128); // constrain the x coordinate of timing dot to fit the screen width
        int exp_x = map(expected_timing, -10.00f, -40.00f, -64, -128); // map the expected timing value to x coordinate of expected line start and end points
        exp_x = constrain(exp_x, -64, -128); // constrain the x coordinate of expected line start and end points to fit the screen width
        
display.drawLine(x1+64,y1+8,x2+64,y2+8 ,SSD1306_WHITE ); 
display.fillCircle(dot_x+64,dot_y+8 ,2 ,SSD1306_WHITE ); 

uint16_t pattern []= {4 }; 

display.drawBitmap(exp_x+64-4/2+4/4*3/4*3/4*3/4*3/4*3/4*3/4*3/4*3/4*3/4*3/4*3/4*3/4*3/4*3/4*5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5,-5}; 

display.drawPattern(pattern,sizeof(pattern)/sizeof(uint16_t),exp_x+64-16,y1+8-16,y1+8+16);
        
    }
    display.display(); // update the display with buffer contents
    
  }
}

// interrupt service routine for ignition module signal
void ign_isr() {
  if (digitalRead(IGN_PIN) == HIGH) { // if rising edge
    ign_rise = micros(); // record the time of rising edge
    dwell = (ign_rise - ign_fall) / 1000.0f; // calculate the dwell time in milliseconds
    Serial.print("Dwell: "); Serial.println(dwell); // print the dwell time to serial monitor
    
    switch (mode) { // switch case for different visualization modes
      
      case 0: // mode 0: dwell and ign bars
        
        display.clearDisplay(); // clear the display buffer
        
        display.setTextSize(1); // set text size to one pixel per character height and width
        display.setTextColor(SSD1306_WHITE); // set text color to white
        
        display.setCursor(0 ,0 ); 
display.print("Dwell: "); 
display.print(dwell ); 
display.println(" ms"); 
        
display.setCursor(64 ,0 ); 
display.print("Ign: "); 
display.print(ign ); 
display.println(" ms"); 
        
int dwell_bar=map(dwell *1000.0f ,0 ,20000 ,0 ,SCREEN_HEIGHT-8 ); 
int ign_bar=map(ign *1000.0f ,0 ,20000 ,0 ,SCREEN_HEIGHT-8 ); 
        
dwell_bar=constrain(dwell_bar ,0 ,SCREEN_HEIGHT-8 ); 
ign_bar=constrain(ign_bar ,0 ,SCREEN_HEIGHT-8 ); 
        
display.fillRect(32 ,8 ,16 ,dwell_bar ,SSD1306_WHITE ); 
display.fillRect(80 ,8 ,16 ,ign_bar ,SSD1306_WHITE ); 
        
break;
      
    }
    
    display.display(); 
    
  } else { 
    
    ign_fall=micros (); 
    
    ign=(ign_fall-ign_rise )/1000.0f ; 
    
    Serial.print("Ign: "); Serial.println(ign ); 
    
    switch (mode ) { 
      
      case 0 : 
        
display.clearDisplay (); 
        
display.setTextSize(1 ); 
display.setTextColor(SSD1306_WHITE ); 
        
display.setCursor(0 ,0 ); 
display.print("Dwell: "); 
display.print(dwell ); 
display.println(" ms"); 
        
display.setCursor(64 ,0 ); 
display.print("Ign: "); 
display.print(ign ); 
display.println(" ms"); 
        
int dwell_bar=map(dwell *1000.0f ,0 ,20000 ,0 ,SCREEN_HEIGHT-8 ); 
int ign_bar=map(ign *1000.0f ,0 ,20000 ,0 ,SCREEN_HEIGHT-8 ); 
        
dwell_bar=constrain(dwell_bar ,0 ,SCREEN_HEIGHT-8 ); 
ign_bar=constrain(ign_bar ,0 ,SCREEN_HEIGHT-8 ); 
        
display.fillRect(32 ,8 ,16 ,dwell_bar ,SSD1306_WHITE ); 
display.fillRect(80 ,8 ,16 ,ign_bar ,SSD1306_WHITE ); 
        
break;
      
    }
    
    display.display (); 
    
  }
}
