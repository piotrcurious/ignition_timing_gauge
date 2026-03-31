
#ifndef ARDUINO_H
#define ARDUINO_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <iostream>
#include <string>

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

#define RISING 0x01
#define FALLING 0x02
#define CHANGE 0x03

#define A0 14
#define A1 15
#define A2 16
#define A3 17
#define A4 18
#define A5 19

typedef bool boolean;
typedef uint8_t byte;

unsigned long micros();
unsigned long millis();
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int digitalRead(uint8_t pin);
int analogRead(uint8_t pin);
void attachInterrupt(uint8_t interrupt, void (*userFunc)(void), int mode);
void detachInterrupt(uint8_t interrupt);
uint8_t digitalPinToInterrupt(uint8_t pin);

void interrupts();
void noInterrupts();

long map(long x, long in_min, long in_max, long out_min, long out_max);

#define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))

class SerialMock {
public:
    void begin(unsigned long baud) {}
    void print(const char* s) { std::cout << s; }
    void print(int n) { std::cout << n; }
    void print(float n) { std::cout << n; }
    void print(unsigned long n) { std::cout << n; }
    void println(const char* s) { std::cout << s << std::endl; }
    void println(int n) { std::cout << n << std::endl; }
    void println(float n) { std::cout << n << std::endl; }
    void println(unsigned long n) { std::cout << n << std::endl; }
    void println() { std::cout << std::endl; }
};

extern SerialMock Serial;

#define F(x) x

#endif
