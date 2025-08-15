
#ifndef P10_DISPLAY_H
#define P10_DISPLAY_H

#include "config.h"
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>

// Display dimensions (configurable)
#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define PANEL_RES_X 64
#define PANEL_RES_Y 32
#define PANEL_CHAIN 1

// HUB75 pin definitions for ESP32 (38-pin)
#define R1_PIN 25
#define G1_PIN 26
#define B1_PIN 27
#define R2_PIN 14
#define G2_PIN 12
#define B2_PIN 13
#define A_PIN 23
#define B_PIN 19
#define C_PIN 5
#define D_PIN 17
#define E_PIN -1
#define LAT_PIN 4
#define OE_PIN 15
#define CLK_PIN 16

// Font types
enum FontType {
  FONT_SMALL = 0,
  FONT_MEDIUM = 1,
  FONT_LARGE = 2,
  FONT_CUSTOM = 3
};

// Animation types
enum AnimationType {
  ANIM_NONE = 0,
  ANIM_SCROLL_LEFT = 1,
  ANIM_SCROLL_RIGHT = 2,
  ANIM_SCROLL_UP = 3,
  ANIM_SCROLL_DOWN = 4,
  ANIM_FADE = 5,
  ANIM_BLINK = 6,
  ANIM_RAINBOW = 7
};

// Panel types
enum PanelType {
  PANEL_MONO = 0,
  PANEL_RGB = 1
};

// P10 Display Configuration
struct DisplaySettings {
  uint8_t brightness = 50;      // 0-100
  uint8_t scrollSpeed = 80;     // milliseconds delay
  uint8_t scrollDirection = 0;  // 0=left, 1=right, 2=up, 3=down
  PanelType panelType = PANEL_RGB; // Panel type
  FontType fontType = FONT_MEDIUM; // Font selection
  AnimationType animationType = ANIM_SCROLL_LEFT; // Animation type
  uint16_t textColor = 0xFFFF;  // White for RGB panels
  uint16_t backgroundColor = 0; // Black background
  uint16_t secondaryColor = 0xF800; // Red for effects
  String currentContent = "";
  unsigned long lastScrollTime = 0;
  unsigned long lastAnimationTime = 0;
  int scrollPosition = 0;
  int animationStep = 0;
  bool scrollEnabled = true;
  bool animationEnabled = true;
};

// Content types for scrolling
enum ContentType {
  CONTENT_RSS_FEEDS = 0,
  CONTENT_TIME = 1,
  CONTENT_DATE = 2,
  CONTENT_QUOTE_OF_DAY = 3,
  CONTENT_FUN_FACTS = 4,
  CONTENT_CUSTOM_TEXT = 5
};

struct ScrollContent {
  ContentType type;
  String name;
  bool enabled;
  String content;
  
  ScrollContent(ContentType t, const String& n, bool e = true) 
    : type(t), name(n), enabled(e) {}
};

// Global display variables
extern DisplaySettings displaySettings;
extern std::vector<ScrollContent> scrollContents;
extern std::vector<String> allRSSHeadlines;
extern MatrixPanel_I2S_DMA *dma_display;

// Main P10 display functions
void initializeP10Display();
void setDisplayBrightness(uint8_t brightness);
void setPanelType(PanelType type);
void setFontType(FontType font);
void setAnimationType(AnimationType animation);

// Include module headers for their function declarations
#include "p10_driver.h"
#include "p10_renderer.h"
#include "p10_content.h"
#include "p10_settings.h"

#endif
