
#ifndef ADAFRUIT_SSD1306_H
#define ADAFRUIT_SSD1306_H

#include "Arduino.h"
#include "Adafruit_GFX.h"

#define SSD1306_SWITCHCAPVCC 0x02
#define WHITE 1
#define BLACK 0
#define SSD1306_WHITE 1
#define SSD1306_BLACK 0

class Adafruit_SSD1306 : public Adafruit_GFX {
public:
    Adafruit_SSD1306(int w, int h, void* wire = nullptr, int reset = -1) : Adafruit_GFX(w, h) {
        buffer = (uint8_t*)malloc(w * h / 8);
        memset(buffer, 0, w * h / 8);
    }
    ~Adafruit_SSD1306() { free(buffer); }

    bool begin(uint8_t switchvcc = SSD1306_SWITCHCAPVCC, uint8_t i2caddr = 0x3C) { return true; }
    void clearDisplay() { memset(buffer, 0, width * height / 8); }
    void display() {
        // Dump buffer to stdout in a format Python can read
        std::cout << "DISPLAY_DUMP " << width << " " << height << " ";
        for (int i = 0; i < width * height / 8; ++i) {
            printf("%02x", buffer[i]);
        }
        std::cout << std::endl;
    }
    void drawPixel(int16_t x, int16_t y, uint16_t color) override {
        if (x < 0 || x >= width || y < 0 || y >= height) return;
        if (color) buffer[x + (y / 8) * width] |= (1 << (y & 7));
        else buffer[x + (y / 8) * width] &= ~(1 << (y & 7));
    }

    // Some .ino files use non-standard functions
    void drawPattern(uint16_t* pattern, uint8_t size, int16_t x, int16_t y1, int16_t y2) {
        // Simple mock for drawPattern
        for (int y = y1; y <= y2; y++) {
            if ((y - y1) % 4 == 0) drawPixel(x, y, WHITE);
        }
    }

private:
    uint8_t* buffer;
};

#endif
