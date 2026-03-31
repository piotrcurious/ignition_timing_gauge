
#include "Arduino.h"
#include <chrono>
#include <vector>
#include <functional>

static auto start_time = std::chrono::steady_clock::now();
static unsigned long mocked_micros = 0;
static bool use_real_time = false;

void set_mocked_micros(unsigned long us) {
    mocked_micros = us;
    use_real_time = false;
}

unsigned long micros() {
    if (use_real_time) {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(now - start_time).count();
    }
    return mocked_micros;
}

unsigned long millis() {
    return micros() / 1000;
}

void delay(unsigned long ms) {
    if (use_real_time) {
        // Not really needed for our tests
    } else {
        mocked_micros += ms * 1000;
    }
}

void delayMicroseconds(unsigned int us) {
    if (use_real_time) {
        // Not really needed
    } else {
        mocked_micros += us;
    }
}

static uint8_t pin_modes[32] = {0};
static uint8_t pin_values[32] = {0};
static int analog_values[32] = {0};

void pinMode(uint8_t pin, uint8_t mode) {
    if (pin < 32) pin_modes[pin] = mode;
}

void digitalWrite(uint8_t pin, uint8_t val) {
    if (pin < 32) pin_values[pin] = val;
}

int digitalRead(uint8_t pin) {
    if (pin < 32) return pin_values[pin];
    return LOW;
}

int analogRead(uint8_t pin) {
    if (pin >= 14 && pin < 32) return analog_values[pin - 14];
    return 0;
}

void setAnalogValue(uint8_t pin, int val) {
    if (pin >= 14 && pin < 32) analog_values[pin - 14] = val;
}

struct Interrupt {
    void (*func)(void);
    int mode;
    int last_val;
};

static Interrupt interrupts_list[32] = {nullptr, 0, LOW};

void attachInterrupt(uint8_t interrupt, void (*userFunc)(void), int mode) {
    if (interrupt < 32) {
        interrupts_list[interrupt] = {userFunc, mode, digitalRead(interrupt)};
    }
}

void detachInterrupt(uint8_t interrupt) {
    if (interrupt < 32) interrupts_list[interrupt].func = nullptr;
}

uint8_t digitalPinToInterrupt(uint8_t pin) {
    return pin;
}

static bool interrupts_enabled = true;
void interrupts() { interrupts_enabled = true; }
void noInterrupts() { interrupts_enabled = false; }

void trigger_pin_change(uint8_t pin, uint8_t new_val) {
    uint8_t old_val = pin_values[pin];
    pin_values[pin] = new_val;
    if (interrupts_enabled && interrupts_list[pin].func) {
        bool trigger = false;
        if (interrupts_list[pin].mode == CHANGE && old_val != new_val) trigger = true;
        else if (interrupts_list[pin].mode == RISING && old_val == LOW && new_val == HIGH) trigger = true;
        else if (interrupts_list[pin].mode == FALLING && old_val == HIGH && new_val == LOW) trigger = true;

        if (trigger) {
            interrupts_list[pin].func();
        }
    }
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

SerialMock Serial;
