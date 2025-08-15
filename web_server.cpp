#include "web_server.h"
#include "p10_display.h"
#include "wifi_manager.h"
#include "rss_handler.h"
#include <Update.h>

void setupWebServer() {
  Serial.println("Setting up web server...");
  
  // Setup WiFi routes
  setupWiFiRoutes();
  
  // Setup display routes
  setupDisplayRoutes();
  
  // Setup OTA update routes
  setupOTARoutes();
  
  // Serve static files from SPIFFS
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  
  // System status endpoint
  server.on("/status", HTTP_GET, [](AsyncWebServerRequest* request) {
    DynamicJsonDocument doc(512);
    doc["freeMemory"] = ESP.getFreeHeap();
    doc["wifi"] = WiFi.isConnected() ? "Connected (" + WiFi.localIP().toString() + ")" : "Disconnected";
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  // Time endpoints
  server.on("/time", HTTP_GET, [](AsyncWebServerRequest* request) {
    String timeStr = getCurrentTimeString();
    String dateStr = getCurrentDateString();
    String response = "{\"time\":\"" + timeStr + "\",\"date\":\"" + dateStr + "\"}";
    request->send(200, "application/json", response);
  });
  
  server.on("/settime", HTTP_POST, [](AsyncWebServerRequest* request) {
    if (request->hasParam("datetime", true)) {
      String datetime = request->getParam("datetime", true)->value();
      if (setManualTime(datetime)) {
        request->send(200, "text/plain", "Time updated successfully");
      } else {
        request->send(400, "text/plain", "Invalid datetime format");
      }
    } else {
      request->send(400, "text/plain", "Missing datetime parameter");
    }
  });
  
  // Settings endpoints
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest* request) {
    DynamicJsonDocument doc(1024);
    doc["fetchInterval"] = settings.fetchInterval;
    doc["maxNewsAgeHours"] = settings.maxNewsAgeHours;
    doc["tzRegion"] = settings.tzRegion;
    doc["maxHeadlinesPerFeed"] = settings.maxHeadlinesPerFeed;
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  server.on("/settings", HTTP_POST, [](AsyncWebServerRequest* request) {
    // Handle settings update
    request->send(200, "text/plain", "Settings updated");
  }, NULL, [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, data);
    
    settings.fetchInterval = doc["fetchInterval"] | settings.fetchInterval;
    settings.maxNewsAgeHours = doc["maxNewsAgeHours"] | settings.maxNewsAgeHours;
    settings.tzRegion = doc["tzRegion"] | settings.tzRegion;
    settings.maxHeadlinesPerFeed = doc["maxHeadlinesPerFeed"] | settings.maxHeadlinesPerFeed;
    
    loadConfiguration(); // Use loadConfiguration instead of saveConfiguration for now
    applyTimezone();
  });
  
  // RSS Feeds endpoints
  server.on("/feeds", HTTP_GET, [](AsyncWebServerRequest* request) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    JsonArray array = doc.to<JsonArray>();
    
    for (const auto& feed : feeds) {
      JsonObject obj = array.createNestedObject();
      obj["name"] = feed.name;
      obj["url"] = feed.url;
      obj["enabled"] = feed.enabled;
    }
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  server.on("/feeds", HTTP_POST, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", "Feeds updated");
  }, NULL, [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
    DynamicJsonDocument doc(JSON_BUFFER_SIZE);
    deserializeJson(doc, data);
    
    feeds.clear();
    JsonArray array = doc.as<JsonArray>();
    
    for (JsonObject obj : array) {
      RSSFeed feed;
      feed.name = obj["name"].as<String>();
      feed.url = obj["url"].as<String>();
      feed.enabled = obj["enabled"].as<bool>();
      feeds.push_back(feed);
    }
    
    saveFeedsToFile();
  });
  
  server.on("/feeds/reset", HTTP_POST, [](AsyncWebServerRequest* request) {
    initializeDefaultFeeds();
    saveFeedsToFile();
    request->send(200, "text/plain", "Feeds reset to default");
  });
  
  server.begin();
  Serial.println("Web server started");
}

void setupOTARoutes() {
  // OTA Update page
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest* request) {
    request->send(SPIFFS, "/update.html", "text/html");
  });
  
  // Handle firmware upload
  server.on("/update", HTTP_POST, [](AsyncWebServerRequest* request) {
    bool shouldReboot = !Update.hasError();
    AsyncWebServerResponse* response = request->beginResponse(200, "text/plain", 
      shouldReboot ? "Update Success! Rebooting..." : "Update Failed!");
    response->addHeader("Connection", "close");
    request->send(response);
    
    if (shouldReboot) {
      delay(1000);
      ESP.restart();
    }
  }, [](AsyncWebServerRequest* request, String filename, size_t index, uint8_t* data, size_t len, bool final) {
    // Handle file upload
    if (!index) {
      Serial.printf("Update Start: %s\n", filename.c_str());
      
      // Start update process
      if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
        Update.printError(Serial);
        return;
      }
    }
    
    // Write chunk to flash
    if (Update.write(data, len) != len) {
      Update.printError(Serial);
      return;
    }
    
    // Final chunk
    if (final) {
      if (Update.end(true)) {
        Serial.printf("Update Success: %uB\n", index + len);
      } else {
        Update.printError(Serial);
      }
    }
  });
}

void setupDisplayRoutes() {
  // Display settings endpoint
  server.on("/display/settings", HTTP_GET, [](AsyncWebServerRequest* request) {
    DynamicJsonDocument doc(1024);
    doc["brightness"] = displaySettings.brightness;
    doc["scrollSpeed"] = displaySettings.scrollSpeed;
    doc["scrollDirection"] = displaySettings.scrollDirection;
    doc["panelType"] = static_cast<int>(displaySettings.panelType);
    doc["fontType"] = static_cast<int>(displaySettings.fontType);
    doc["animationType"] = static_cast<int>(displaySettings.animationType);
    doc["textColor"] = displaySettings.textColor;
    doc["backgroundColor"] = displaySettings.backgroundColor;
    doc["secondaryColor"] = displaySettings.secondaryColor;
    doc["scrollEnabled"] = displaySettings.scrollEnabled;
    doc["animationEnabled"] = displaySettings.animationEnabled;
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
  });
  
  server.on("/display/settings", HTTP_POST, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", "Display settings updated");
  }, NULL, [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, data);
    
    if (doc.containsKey("brightness")) {
      setDisplayBrightness(doc["brightness"]);
    }
    if (doc.containsKey("scrollSpeed")) {
      setScrollSpeed(doc["scrollSpeed"]);
    }
    if (doc.containsKey("scrollDirection")) {
      setScrollDirection(doc["scrollDirection"]);
    }
    if (doc.containsKey("panelType")) {
      setPanelType(static_cast<PanelType>(doc["panelType"].as<int>()));
    }
    if (doc.containsKey("fontType")) {
      setFontType(static_cast<FontType>(doc["fontType"].as<int>()));
    }
    if (doc.containsKey("animationType")) {
      setAnimationType(static_cast<AnimationType>(doc["animationType"].as<int>()));
    }
    if (doc.containsKey("animationEnabled")) {
      displaySettings.animationEnabled = doc["animationEnabled"];
    }
    
    saveDisplaySettings();
  });
  
  // Scroll content endpoint
  server.on("/display/content", HTTP_POST, [](AsyncWebServerRequest* request) {
    request->send(200, "text/plain", "Scroll content updated");
  }, NULL, [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, data);
    
    // Update scroll content enable/disable status
    for (auto& content : scrollContents) {
      switch (content.type) {
        case CONTENT_TIME:
          content.enabled = doc["time"] | content.enabled;
          break;
        case CONTENT_DATE:
          content.enabled = doc["date"] | content.enabled;
          break;
        case CONTENT_RSS_FEEDS:
          content.enabled = doc["rss"] | content.enabled;
          break;
        case CONTENT_QUOTE_OF_DAY:
          content.enabled = doc["quotes"] | content.enabled;
          break;
        case CONTENT_FUN_FACTS:
          content.enabled = doc["facts"] | content.enabled;
          break;
      }
    }
    
    saveDisplaySettings();
  });
  
  // Manual RSS fetch trigger
  server.on("/feeds/fetch", HTTP_POST, [](AsyncWebServerRequest* request) {
    Serial.println("RSS fetch requested via web interface");
    
    // Create a task to fetch RSS feeds to avoid blocking the web server
    xTaskCreate([](void* parameter) {
      Serial.println("Starting RSS fetch task...");
      fetchAllRSSFeeds();
      Serial.println("RSS fetch task completed");
      vTaskDelete(NULL);
    }, "RSS_Fetch_Task", 8192, NULL, 1, NULL);
    
    request->send(200, "text/plain", "RSS fetch started");
  });
}
