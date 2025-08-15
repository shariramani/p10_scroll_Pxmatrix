#ifndef P10_DRIVER_H
#define P10_DRIVER_H

#include <Arduino.h>
#include "p10_display.h"

// Function declarations for P10 hardware control
void initializeP10Hardware();
void clearDisplayBuffer();
void setDisplayBrightness(uint8_t brightness);
void setPanelType(PanelType type);
void setFontType(FontType font);
void setAnimationType(AnimationType animation);

// Hardware variables
extern MatrixPanel_I2S_DMA *dma_display;

#endif