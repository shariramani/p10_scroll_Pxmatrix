
#include "config.h"

// File paths
const char* CONFIG_PATH = "/feeds.json";
const char* SETTINGS_PATH = "/settings.json";

// Default RSS feeds
const std::vector<RSSFeed> DEFAULT_FEEDS = {
  {"NDTV", "https://feeds.feedburner.com/ndtvnews-top-stories", true},
  {"The Hindu", "https://www.thehindu.com/news/national/feeder/default.rss", true},
  {"Economic Times", "https://economictimes.indiatimes.com/rssfeedsdefault.cms", true},
  {"BBC World", "http://feeds.bbci.co.uk/news/world/rss.xml", true},
  {"Reuters", "https://feeds.reuters.com/reuters/topNews", true}
};

bool initializeSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return false;
  }
  
  Serial.println("SPIFFS initialized successfully");
  
  // Check available space
  size_t totalBytes = SPIFFS.totalBytes();
  size_t usedBytes = SPIFFS.usedBytes();
  Serial.printf("SPIFFS: %d/%d bytes used\n", usedBytes, totalBytes);
  
  return true;
}

bool loadFeedsFromFile() {
  File file = SPIFFS.open(CONFIG_PATH, "r");
  if (!file) {
    Serial.println("Feeds config file not found - will use defaults");
    return false;
  }
  
  DynamicJsonDocument doc(JSON_BUFFER_SIZE);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.printf("Failed to parse feeds config: %s\n", error.c_str());
    return false;
  }
  
  feeds.clear();
  JsonArray array = doc.as<JsonArray>();
  
  for (JsonObject obj : array) {
    RSSFeed feed;
    feed.name = obj["name"].as<String>();
    feed.url = obj["url"].as<String>();
    feed.enabled = obj["enabled"].as<bool>();
    feeds.push_back(feed);
  }
  
  Serial.printf("Loaded %d RSS feeds from config\n", feeds.size());
  return true;
}

bool saveFeedsToFile() {
  File file = SPIFFS.open(CONFIG_PATH, "w");
  if (!file) {
    Serial.println("Failed to open feeds config for writing");
    return false;
  }
  
  DynamicJsonDocument doc(JSON_BUFFER_SIZE);
  JsonArray array = doc.to<JsonArray>();
  
  for (const auto& feed : feeds) {
    JsonObject obj = array.createNestedObject();
    obj["name"] = feed.name;
    obj["url"] = feed.url;
    obj["enabled"] = feed.enabled;
  }
  
  size_t bytesWritten = serializeJson(doc, file);
  file.close();
  
  Serial.printf("Saved %d feeds to config (%d bytes)\n", feeds.size(), bytesWritten);
  return bytesWritten > 0;
}

bool loadSettings() {
  File file = SPIFFS.open(SETTINGS_PATH, "r");
  if (!file) {
    Serial.println("Settings file not found - using defaults");
    return false;
  }
  
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, file);
  file.close();
  
  if (error) {
    Serial.printf("Failed to parse settings: %s\n", error.c_str());
    return false;
  }
  
  settings.fetchInterval = doc["fetchInterval"] | 300;
  settings.maxNewsAgeHours = doc["maxNewsAgeHours"] | 24;
  settings.tzRegion = doc["tzRegion"] | "Asia/Kolkata";
  settings.maxHeadlinesPerFeed = doc["maxHeadlinesPerFeed"] | MAX_HEADLINES_PER_FEED;
  
  Serial.println("Settings loaded successfully");
  return true;
}

bool saveSettings() {
  File file = SPIFFS.open(SETTINGS_PATH, "w");
  if (!file) {
    Serial.println("Failed to open settings file for writing");
    return false;
  }
  
  DynamicJsonDocument doc(1024);
  doc["fetchInterval"] = settings.fetchInterval;
  doc["maxNewsAgeHours"] = settings.maxNewsAgeHours;
  doc["tzRegion"] = settings.tzRegion;
  doc["maxHeadlinesPerFeed"] = settings.maxHeadlinesPerFeed;
  
  size_t bytesWritten = serializeJson(doc, file);
  file.close();
  
  Serial.printf("Settings saved (%d bytes)\n", bytesWritten);
  return bytesWritten > 0;
}

void loadConfiguration() {
  Serial.println("Loading configuration...");
  
  if (!loadFeedsFromFile()) {
    Serial.println("Using default feeds");
    initializeDefaultFeeds();
  }
  
  if (!loadSettings()) {
    Serial.println("Using default settings");
  }
  
  Serial.printf("Configuration loaded: %d feeds, %d sec interval\n", 
                feeds.size(), settings.fetchInterval);
}

void initializeDefaultFeeds() {
  feeds.clear();
  feeds = DEFAULT_FEEDS;
  Serial.printf("Initialized %d default feeds\n", feeds.size());
}

String sanitizeString(const String& input) {
  String result = input;
  result.replace("<![CDATA[", "");
  result.replace("]]>", "");
  result.replace("&amp;", "&");
  result.replace("&lt;", "<");
  result.replace("&gt;", ">");
  result.replace("&quot;", "\"");
  result.replace("&apos;", "'");
  result.trim();
  return result;
}

void logMemoryUsage(const String& context) {
  Serial.printf("[%s] Free heap: %d bytes\n", context.c_str(), ESP.getFreeHeap());
}
