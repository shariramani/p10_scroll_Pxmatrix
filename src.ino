
/*
 * ESP32-C6 RSS News Scroller and Clock with Web Configuration and OTA
 * 
 * Features:
 * - Wi-Fi Manager with captive portal
 * - RSS feed fetching and parsing
 * - Web-based configuration
 * - NTP/RTC time synchronization
 * - OTA firmware updates
 * - P10 LED display support with scrolling
 */

#include "config.h"
#include "wifi_manager.h"
#include "rss_handler.h"
#include "web_server.h"
#include "time_manager.h"
#include "p10_display.h"

// Global variables
AsyncWebServer server(80);
Preferences preferences;
RTC_DS3231 rtc;
Settings settings;
std::vector<RSSFeed> feeds;
unsigned long lastFetchTime = 0;
bool hasInternet = false;

void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP32-C6 RSS News Scroller Starting ===");
  
  // Initialize SPIFFS
  if (!initializeSPIFFS()) {
    Serial.println("CRITICAL: SPIFFS initialization failed!");
    return;
  }
  
  // Initialize preferences
  preferences.begin("wifi", false);
  
  // Connect to WiFi
  hasInternet = connectToWiFi();
  
  // Initialize time
  initializeTime();
  
  // Load configuration
  loadConfiguration();
  
  // Initialize P10 display
  initializeP10Display();
  
  // Setup web server
  setupWebServer();
  
  // Initialize default feeds if none exist
  if (feeds.empty()) {
    initializeDefaultFeeds();
    saveFeedsToFile();
  }
  
  Serial.println("=== Setup Complete ===");
  Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  
  if (hasInternet) {
    Serial.println("Device ready - RSS feeds will be fetched every " + String(settings.fetchInterval) + " seconds");
  } else {
    Serial.println("No internet connection - only local features available");
  }
  
  Serial.println("P10 Display: Scrolling enabled with configurable content");
}

void loop() {
  static unsigned long lastHeapCheck = 0;
  unsigned long currentTime = millis();
  
  // Fetch RSS feeds periodically
  if (hasInternet && (currentTime - lastFetchTime > settings.fetchInterval * 1000)) {
    lastFetchTime = currentTime;
    Serial.println("\n--- Starting RSS Feed Fetch Cycle ---");
    fetchAllRSSFeeds();
    Serial.println("--- RSS Feed Fetch Cycle Complete ---\n");
  }
  
  // Update P10 display content and handle scrolling
  updateDisplayContent();
  
  // Check heap memory every 30 seconds
  if (currentTime - lastHeapCheck > 30000) {
    lastHeapCheck = currentTime;
    Serial.printf("Free heap: %d bytes\n", ESP.getFreeHeap());
  }
  
  // Add small delay to prevent watchdog issues
  delay(50); // Reduced delay for smoother scrolling
}
