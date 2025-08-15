
#ifndef P10_CONTENT_H
#define P10_CONTENT_H

#include <Arduino.h>
#include "p10_display.h"

// Content management functions
void updateDisplayContent();
void scrollText();
void setScrollSpeed(uint8_t speed);
void setScrollDirection(uint8_t direction);
void addScrollContent(ContentType type, const String& content);
void addRSSHeadline(const String& headline);
void clearRSSHeadlines();

// Content generation functions
String generateTimeContent();
String generateDateContent();
String generateRSSContent();
String loadQuoteOfDay();
String loadFunFact();

#endif
