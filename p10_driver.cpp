#include "p10_driver.h"
#include "p10_display.h"

// Global matrix display object
MatrixPanel_I2S_DMA *dma_display = nullptr;

void initializeP10Hardware() {
  Serial.println("Initializing HUB75 LED Matrix Panel...");
  
  // Configure the matrix panel
  HUB75_I2S_CFG mxconfig(
    PANEL_RES_X,   // module width
    PANEL_RES_Y,   // module height
    PANEL_CHAIN    // Chain length
  );

  // Configure pins for ESP32 (38-pin)
  mxconfig.gpio.r1 = R1_PIN;
  mxconfig.gpio.g1 = G1_PIN;
  mxconfig.gpio.b1 = B1_PIN;
  mxconfig.gpio.r2 = R2_PIN;
  mxconfig.gpio.g2 = G2_PIN;
  mxconfig.gpio.b2 = B2_PIN;
  mxconfig.gpio.a = A_PIN;
  mxconfig.gpio.b = B_PIN;
  mxconfig.gpio.c = C_PIN;
  mxconfig.gpio.d = D_PIN;
  mxconfig.gpio.e = E_PIN;
  mxconfig.gpio.lat = LAT_PIN;
  mxconfig.gpio.oe = OE_PIN;
  mxconfig.gpio.clk = CLK_PIN;

  // Create the display object
  dma_display = new MatrixPanel_I2S_DMA(mxconfig);
  
  // Initialize the display
  if (!dma_display->begin()) {
    Serial.println("ERROR: Could not initialize matrix display!");
    return;
  }
  
  // Clear the display
  dma_display->clearScreen();
  
  // Set initial brightness
  dma_display->setBrightness8(displaySettings.brightness * 255 / 100);
  
  Serial.printf("HUB75 Matrix initialized - Resolution: %dx%d\n", PANEL_RES_X, PANEL_RES_Y);
  Serial.printf("Panel type: %s\n", displaySettings.panelType == PANEL_RGB ? "RGB" : "Mono");
}

void clearDisplayBuffer() {
  if (dma_display) {
    dma_display->clearScreen();
  }
}

void setDisplayBrightness(uint8_t brightness) {
  displaySettings.brightness = constrain(brightness, 0, 100);
  if (dma_display) {
    dma_display->setBrightness8(displaySettings.brightness * 255 / 100);
  }
  Serial.printf("Display brightness set to: %d%%\n", displaySettings.brightness);
}

void setPanelType(PanelType type) {
  displaySettings.panelType = type;
  
  // Adjust default colors based on panel type
  if (type == PANEL_MONO) {
    displaySettings.textColor = 0xF800; // Red for mono
    displaySettings.backgroundColor = 0;
    displaySettings.secondaryColor = 0x07E0; // Green
  } else {
    displaySettings.textColor = 0xFFFF; // White for RGB
    displaySettings.backgroundColor = 0;
    displaySettings.secondaryColor = 0xF800; // Red
  }
  
  Serial.printf("Panel type set to: %s\n", type == PANEL_RGB ? "RGB" : "Mono");
}

void setFontType(FontType font) {
  displaySettings.fontType = font;
  Serial.printf("Font type set to: %d\n", font);
}

void setAnimationType(AnimationType animation) {
  displaySettings.animationType = animation;
  displaySettings.animationStep = 0;
  displaySettings.lastAnimationTime = millis();
  Serial.printf("Animation type set to: %d\n", animation);
}