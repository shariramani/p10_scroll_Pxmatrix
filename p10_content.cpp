
#include "p10_content.h"
#include "p10_display.h"
#include "p10_renderer.h"

void updateDisplayContent() {
  static unsigned long lastContentUpdate = 0;
  static int contentIndex = 0;
  
  // Update content every 5 seconds
  if (millis() - lastContentUpdate > 5000) {
    
    // Find next enabled content
    int attempts = 0;
    do {
      contentIndex = (contentIndex + 1) % scrollContents.size();
      attempts++;
    } while (!scrollContents[contentIndex].enabled && attempts < scrollContents.size());
    
    if (scrollContents[contentIndex].enabled) {
      String newContent = "";
      
      switch (scrollContents[contentIndex].type) {
        case CONTENT_TIME:
          newContent = generateTimeContent();
          break;
        case CONTENT_DATE:
          newContent = generateDateContent();
          break;
        case CONTENT_RSS_FEEDS:
          newContent = generateRSSContent();
          break;
        case CONTENT_QUOTE_OF_DAY:
          newContent = loadQuoteOfDay();
          break;
        case CONTENT_FUN_FACTS:
          newContent = loadFunFact();
          break;
        case CONTENT_CUSTOM_TEXT:
          newContent = scrollContents[contentIndex].content;
          break;
      }
      
      if (newContent != displaySettings.currentContent) {
        displaySettings.currentContent = newContent;
        displaySettings.scrollPosition = 0;
        
        Serial.printf("Display content updated: %s\n", newContent.c_str());
      }
    }
    
    lastContentUpdate = millis();
  }
  
  // Handle scrolling
  if (displaySettings.scrollEnabled) {
    scrollText();
  }
}

void scrollText() {
  if (millis() - displaySettings.lastScrollTime < displaySettings.scrollSpeed) {
    return;
  }
  
  if (displaySettings.currentContent.length() == 0) {
    return;
  }
  
  // Calculate text width properly
  int textWidth = calculateTextWidth(displaySettings.currentContent);
  bool needsScrolling = textWidth > DISPLAY_WIDTH;
  
  Serial.printf("Content: '%s', Width: %d, Display: %d, Needs scrolling: %s\n", 
                displaySettings.currentContent.c_str(), textWidth, DISPLAY_WIDTH, needsScrolling ? "YES" : "NO");
  
  if (dma_display) {
    dma_display->clearScreen();
  }
  
  if (needsScrolling) {
    // Handle different scroll directions
    switch (displaySettings.scrollDirection) {
      case 0: // Left
        displaySettings.scrollPosition++;
        if (displaySettings.scrollPosition >= textWidth + DISPLAY_WIDTH) {
          displaySettings.scrollPosition = 0;
        }
        break;
      case 1: // Right
        displaySettings.scrollPosition--;
        if (displaySettings.scrollPosition <= -textWidth - DISPLAY_WIDTH) {
          displaySettings.scrollPosition = DISPLAY_WIDTH;
        }
        break;
      case 2: // Up
        displaySettings.scrollPosition++;
        if (displaySettings.scrollPosition >= DISPLAY_HEIGHT + 10) {
          displaySettings.scrollPosition = -10;
        }
        break;
      case 3: // Down
        displaySettings.scrollPosition--;
        if (displaySettings.scrollPosition <= -DISPLAY_HEIGHT - 10) {
          displaySettings.scrollPosition = DISPLAY_HEIGHT;
        }
        break;
    }
    
    int drawX, drawY;
    
    if (displaySettings.scrollDirection <= 1) {
      // Horizontal scrolling
      drawX = (displaySettings.scrollDirection == 0) ? 
              (DISPLAY_WIDTH - displaySettings.scrollPosition) : displaySettings.scrollPosition;
      drawY = (DISPLAY_HEIGHT - 8) / 2; // Vertically centered
    } else {
      // Vertical scrolling
      drawX = (DISPLAY_WIDTH - textWidth) / 2;
      if (drawX < 0) drawX = 0;
      drawY = (displaySettings.scrollDirection == 2) ? 
              (DISPLAY_HEIGHT - displaySettings.scrollPosition) : displaySettings.scrollPosition;
    }
    
    drawTextWithAnimation(displaySettings.currentContent, drawX, drawY);
    displaySettings.lastScrollTime = millis();
    
  } else {
    // Static text that fits
    static unsigned long lastStaticUpdate = 0;
    if (millis() - lastStaticUpdate > 1000) { // Update every second for time
      int startX = (DISPLAY_WIDTH - textWidth) / 2;
      if (startX < 0) startX = 0;
      int startY = (DISPLAY_HEIGHT - 8) / 2; // Vertically centered
      
      drawTextWithAnimation(displaySettings.currentContent, startX, startY);
      lastStaticUpdate = millis();
      Serial.printf("Static display: '%s' centered at (%d,%d)\n", displaySettings.currentContent.c_str(), startX, startY);
    }
  }
}

void setScrollSpeed(uint8_t speed) {
  displaySettings.scrollSpeed = constrain(speed, 30, 200);
  Serial.printf("Scroll speed set to: %d ms\n", displaySettings.scrollSpeed);
}

void setScrollDirection(uint8_t direction) {
  displaySettings.scrollDirection = constrain(direction, 0, 3);
  displaySettings.scrollPosition = 0;
  Serial.printf("Scroll direction set to: %d\n", displaySettings.scrollDirection);
}

void addScrollContent(ContentType type, const String& content) {
  ScrollContent newContent(type, content, true);
  newContent.content = content;
  scrollContents.push_back(newContent);
  Serial.printf("Added scroll content: %s\n", content.c_str());
}

void addRSSHeadline(const String& headline) {
  allRSSHeadlines.push_back(headline);
  
  if (allRSSHeadlines.size() > 20) {
    allRSSHeadlines.erase(allRSSHeadlines.begin());
  }
  
  Serial.printf("Added RSS headline: %s (Total: %d)\n", headline.c_str(), allRSSHeadlines.size());
}

void clearRSSHeadlines() {
  allRSSHeadlines.clear();
  Serial.println("Cleared all RSS headlines");
}

String generateTimeContent() {
  return getCurrentTimeString();
}

String generateDateContent() {
  return getCurrentDateString();
}

String generateRSSContent() {
  static int headlineIndex = 0;
  
  if (allRSSHeadlines.size() > 0) {
    String headline = allRSSHeadlines[headlineIndex];
    headlineIndex = (headlineIndex + 1) % allRSSHeadlines.size();
    return headline;
  }
  
  return "No RSS headlines available";
}

String loadQuoteOfDay() {
  File file = SPIFFS.open("/quotes.txt", "r");
  if (!file) {
    return "Believe you can and you're halfway there.";
  }
  
  std::vector<String> quotes;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      quotes.push_back(line);
    }
  }
  file.close();
  
  if (quotes.size() > 0) {
    int randomIndex = random(0, quotes.size());
    return quotes[randomIndex];
  }
  
  return "No quotes available";
}

String loadFunFact() {
  File file = SPIFFS.open("/facts.txt", "r");
  if (!file) {
    return "The ESP32 has built-in Wi-Fi and Bluetooth!";
  }
  
  std::vector<String> facts;
  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      facts.push_back(line);
    }
  }
  file.close();
  
  if (facts.size() > 0) {
    int randomIndex = random(0, facts.size());
    return facts[randomIndex];
  }
  
  return "No facts available";
}
