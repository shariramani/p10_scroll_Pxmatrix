#include "p10_renderer.h"
#include "p10_display.h"
#include "p10_driver.h"

// Font size configurations
struct FontConfig {
  uint8_t width;
  uint8_t height;
  uint8_t spacing;
};

const FontConfig fontConfigs[] = {
  {4, 6, 1},   // FONT_SMALL
  {6, 8, 1},   // FONT_MEDIUM  
  {8, 12, 2},  // FONT_LARGE
  {6, 8, 1}    // FONT_CUSTOM
};

int calculateTextWidth(const String& text) {
  FontConfig config = fontConfigs[displaySettings.fontType];
  return text.length() * (config.width + config.spacing);
}

void drawText(const String& text, int x, int y) {
  if (!dma_display) return;
  
  FontConfig config = fontConfigs[displaySettings.fontType];
  uint16_t color = displaySettings.textColor;
  
  // Adjust color for mono panels
  if (displaySettings.panelType == PANEL_MONO) {
    color = 0xF800; // Red for mono panels
  }
  
  // Set font size based on font type
  switch (displaySettings.fontType) {
    case FONT_SMALL:
      dma_display->setTextSize(1);
      break;
    case FONT_MEDIUM:
      dma_display->setTextSize(1);
      break;
    case FONT_LARGE:
      dma_display->setTextSize(2);
      break;
    case FONT_CUSTOM:
      dma_display->setTextSize(1);
      break;
  }
  
  dma_display->setTextColor(color);
  dma_display->setCursor(x, y);
  dma_display->print(text);
  
  Serial.printf("Drew text: '%s' at (%d,%d) with color 0x%04X\n", text.c_str(), x, y, color);
}

void drawTextWithAnimation(const String& text, int x, int y) {
  if (!dma_display || !displaySettings.animationEnabled) {
    drawText(text, x, y);
    return;
  }
  
  unsigned long currentTime = millis();
  
  switch (displaySettings.animationType) {
    case ANIM_FADE:
      drawFadeAnimation(text, x, y);
      break;
    case ANIM_BLINK:
      drawBlinkAnimation(text, x, y);
      break;
    case ANIM_RAINBOW:
      drawRainbowAnimation(text, x, y);
      break;
    default:
      drawText(text, x, y);
      break;
  }
}

void drawFadeAnimation(const String& text, int x, int y) {
  unsigned long currentTime = millis();
  
  if (currentTime - displaySettings.lastAnimationTime > 100) {
    displaySettings.animationStep = (displaySettings.animationStep + 1) % 100;
    displaySettings.lastAnimationTime = currentTime;
    
    // Calculate fade brightness
    uint8_t brightness = (sin(displaySettings.animationStep * 0.0628) + 1) * 127; // 0.0628 = 2*PI/100
    
    if (dma_display) {
      dma_display->setBrightness8(brightness);
      drawText(text, x, y);
    }
  }
}

void drawBlinkAnimation(const String& text, int x, int y) {
  unsigned long currentTime = millis();
  
  if (currentTime - displaySettings.lastAnimationTime > 500) {
    displaySettings.animationStep = (displaySettings.animationStep + 1) % 2;
    displaySettings.lastAnimationTime = currentTime;
  }
  
  if (displaySettings.animationStep == 0) {
    drawText(text, x, y);
  } else {
    // Don't draw text (blink off)
  }
}

void drawRainbowAnimation(const String& text, int x, int y) {
  if (!dma_display) return;
  
  unsigned long currentTime = millis();
  
  if (currentTime - displaySettings.lastAnimationTime > 50) {
    displaySettings.animationStep = (displaySettings.animationStep + 1) % 360;
    displaySettings.lastAnimationTime = currentTime;
  }
  
  FontConfig config = fontConfigs[displaySettings.fontType];
  
  // Set font size
  switch (displaySettings.fontType) {
    case FONT_LARGE:
      dma_display->setTextSize(2);
      break;
    default:
      dma_display->setTextSize(1);
      break;
  }
  
  dma_display->setCursor(x, y);
  
  // Draw each character with different rainbow color
  for (int i = 0; i < text.length(); i++) {
    uint16_t hue = (displaySettings.animationStep + i * 30) % 360;
    uint16_t color = HSVtoRGB565(hue, 255, 255);
    
    dma_display->setTextColor(color);
    dma_display->print(text.charAt(i));
  }
}

void shiftDisplayLeft() {
  // Not needed with new library - handled by scrolling logic
}

// Convert HSV to RGB565
uint16_t HSVtoRGB565(uint16_t hue, uint8_t sat, uint8_t val) {
  uint8_t r, g, b;
  
  if (sat == 0) {
    r = g = b = val;
  } else {
    uint16_t region = hue / 43;
    uint16_t remainder = (hue - (region * 43)) * 6;
    
    uint8_t p = (val * (255 - sat)) >> 8;
    uint8_t q = (val * (255 - ((sat * remainder) >> 8))) >> 8;
    uint8_t t = (val * (255 - ((sat * (255 - remainder)) >> 8))) >> 8;
    
    switch (region) {
      case 0: r = val; g = t; b = p; break;
      case 1: r = q; g = val; b = p; break;
      case 2: r = p; g = val; b = t; break;
      case 3: r = p; g = q; b = val; break;
      case 4: r = t; g = p; b = val; break;
      default: r = val; g = p; b = q; break;
    }
  }
  
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}