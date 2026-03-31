
#include "Arduino.h"
#include <iostream>
#include <string>
#include <sstream>

extern void setup();
extern void loop();

extern void set_mocked_micros(unsigned long us);
extern void trigger_pin_change(uint8_t pin, uint8_t new_val);
extern void setAnalogValue(uint8_t pin, int val);

int main() {
    setup();
    std::string line;
    while (std::getline(std::cin, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string cmd;
        ss >> cmd;
        if (cmd == "STEP") {
            loop();
        } else if (cmd == "TIME") {
            unsigned long us;
            ss >> us;
            set_mocked_micros(us);
        } else if (cmd == "PIN") {
            int pin, val;
            ss >> pin >> val;
            trigger_pin_change(pin, val);
        } else if (cmd == "ANALOG") {
            int pin, val;
            ss >> pin >> val;
            setAnalogValue(pin, val);
        } else if (cmd == "EXIT") {
            break;
        }
    }
    return 0;
}
