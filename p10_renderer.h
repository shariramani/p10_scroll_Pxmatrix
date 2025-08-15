#ifndef P10_RENDERER_H
#define P10_RENDERER_H

#include <Arduino.h>
#include "p10_display.h"

// Font and rendering functions
int calculateTextWidth(const String& text);
void drawText(const String& text, int x, int y);
void drawTextWithAnimation(const String& text, int x, int y);
void drawFadeAnimation(const String& text, int x, int y);
void drawBlinkAnimation(const String& text, int x, int y);
void drawRainbowAnimation(const String& text, int x, int y);
void shiftDisplayLeft();
uint16_t HSVtoRGB565(uint16_t hue, uint8_t sat, uint8_t val);

#endif