
#ifndef ADAFRUIT_GFX_H
#define ADAFRUIT_GFX_H

#include <stdint.h>
#include <string.h>

#define SSD1306_WHITE 1
#define WHITE 1
#define SSD1306_BLACK 0
#define BLACK 0

class Adafruit_GFX {
public:
    int16_t width, height;
    Adafruit_GFX(int16_t w, int16_t h) : width(w), height(h), _textSize(1), _textColor(WHITE), _cursorX(0), _cursorY(0) {}
    virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;

    void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
        // Simple Bresenham line algorithm mock
        int16_t dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
        int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
        int16_t err = dx + dy, e2;
        for (;;) {
            drawPixel(x0, y0, color);
            if (x0 == x1 && y0 == y1) break;
            e2 = 2 * err;
            if (e2 >= dy) { err += dy; x0 += sx; }
            if (e2 <= dx) { err += dx; y0 += sy; }
        }
    }

    void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
        for (int16_t i = x; i < x + w; i++) {
            for (int16_t j = y; j < y + h; j++) {
                drawPixel(i, j, color);
            }
        }
    }

    void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
        // Simple circle fill
        for (int16_t x = x0 - r; x <= x0 + r; x++) {
            for (int16_t y = y0 - r; y <= y0 + r; y++) {
                if ((x - x0) * (x - x0) + (y - y0) * (y - y0) <= r * r) {
                    drawPixel(x, y, color);
                }
            }
        }
    }

    void setCursor(int16_t x, int16_t y) { _cursorX = x; _cursorY = y; }
    void setTextSize(uint8_t s) { _textSize = s; }
    void setTextColor(uint16_t c) { _textColor = c; }

    // Mock print/println for text on screen
    void print(const char* s) { /* Mock drawing text is complex, just log it */ }
    void print(int n) { /* Same here */ }
    void print(float n) { /* Same here */ }
    void println(const char* s) { /* Same here */ }
    void println(int n) { /* Same here */ }
    void println(float n) { /* Same here */ }
    void println() { /* Same here */ }

    // Mock drawBitmap
    void drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color) {
      // Just a stub
    }

    // Some .ino files have very broken drawBitmap/drawPattern calls.
    // Let's provide a catch-all for weird signatures
    template<typename... Args>
    void drawBitmap(Args... args) {}

protected:
    uint8_t _textSize;
    uint16_t _textColor;
    int16_t _cursorX, _cursorY;
};

#endif
