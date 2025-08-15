
#ifndef CONFIG_H
#define CONFIG_H

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <TinyXML2.h>
#include <time.h>
#include <AsyncTCP.h>
#include <ESPmDNS.h>
#include <Update.h>
#include <Preferences.h>
#include <Wire.h>
#include "RTClib.h"
#include "tz_mappings.h"

// Configuration constants
#define AP_SSID "ESP32_Config"
#define AP_PASS "12345678"
#define HTTP_TIMEOUT 8000
#define MAX_HEADLINES_PER_FEED 10
#define JSON_BUFFER_SIZE 8192

// File paths
extern const char* CONFIG_PATH;
extern const char* SETTINGS_PATH;

// Data structures
struct RSSFeed {
  String name;
  String url;
  bool enabled;
  
  RSSFeed() : enabled(true) {}
  RSSFeed(const String& n, const String& u, bool e = true) 
    : name(n), url(u), enabled(e) {}
};

struct Settings {
  unsigned long fetchInterval = 300;  // 5 minutes default
  unsigned long maxNewsAgeHours = 24;  // 24 hours default
  String tzRegion = "Asia/Kolkata";
  int maxHeadlinesPerFeed = MAX_HEADLINES_PER_FEED;
  
  Settings() = default;
};

// Global variables (declared extern here, defined in main.ino)
extern AsyncWebServer server;
extern Preferences preferences;
extern RTC_DS3231 rtc;
extern Settings settings;
extern std::vector<RSSFeed> feeds;
extern unsigned long lastFetchTime;
extern bool hasInternet;

// Function declarations
bool initializeSPIFFS();
void loadConfiguration();
void saveConfiguration();  // Added this missing declaration
void initializeDefaultFeeds();
bool saveFeedsToFile();

// Time function declarations
String getCurrentTimeString();
String getCurrentDateString();
bool setManualTime(const String& datetime);
void applyTimezone();

// Utility functions
String sanitizeString(const String& input);
void logMemoryUsage(const String& context);

#endif
