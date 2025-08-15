
#include "p10_display.h"
#include "p10_driver.h"
#include "p10_renderer.h"
#include "p10_content.h"
#include "p10_settings.h"

// Global display variables
DisplaySettings displaySettings;
std::vector<ScrollContent> scrollContents;
std::vector<String> allRSSHeadlines;

void initializeP10Display() {
  // Initialize hardware
  initializeP10Hardware();
  
  // Load display settings
  loadDisplaySettings();
  
  // Initialize scroll contents if empty
  initializeDefaultScrollContents();
  
  Serial.printf("P10 Display initialized - Brightness: %d, Speed: %d\n",
                displaySettings.brightness, displaySettings.scrollSpeed);
  
  // Display initial test message
  if (dma_display) {
    dma_display->clearScreen();
    drawText("MATRIX READY", 1, (DISPLAY_HEIGHT - 8) / 2);
  }
}
