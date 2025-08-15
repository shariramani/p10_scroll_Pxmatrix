
#include "time_manager.h"

void initializeTime() {
  Serial.println("Initializing time system...");
  
  // Apply timezone first
  applyTimezone();
  
  if (hasInternet) {
    syncTimeFromNTP();
  } else {
    syncTimeFromRTC();
  }
  
  Serial.printf("Current time: %s\n", getCurrentTimeString().c_str());
}

void applyTimezone() {
  for (const auto& entry : tzList) {
    if (settings.tzRegion == entry.iana) {
      setenv("TZ", entry.posix, 1);
      tzset();
      Serial.printf("Timezone applied: %s (%s)\n", entry.iana, entry.posix);
      return;
    }
  }
  
  Serial.println("Timezone not found, using UTC");
  setenv("TZ", "UTC0", 1);
  tzset();
}

void syncTimeFromNTP() {
  Serial.println("Syncing time from NTP...");
  
  configTime(0, 0, "pool.ntp.org", "time.nist.gov", "time.cloudflare.com");
  
  int attempts = 0;
  while (time(nullptr) < 100000 && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  time_t now = time(nullptr);
  if (now > 100000) {
    Serial.printf("\nNTP sync successful: %s", ctime(&now));
    
    // Update RTC if available
    if (rtc.begin()) {
      rtc.adjust(DateTime(now));
      Serial.println("RTC updated from NTP");
    }
  } else {
    Serial.println("\nNTP sync failed, trying RTC...");
    syncTimeFromRTC();
  }
}

void syncTimeFromRTC() {
  if (!rtc.begin()) {
    Serial.println("RTC not found - time sync failed");
    return;
  }
  
  if (rtc.lostPower()) {
    Serial.println("RTC lost power - time may be incorrect");
    return;
  }
  
  DateTime rtcTime = rtc.now();
  struct tm tm;
  tm.tm_year = rtcTime.year() - 1900;
  tm.tm_mon = rtcTime.month() - 1;
  tm.tm_mday = rtcTime.day();
  tm.tm_hour = rtcTime.hour();
  tm.tm_min = rtcTime.minute();
  tm.tm_sec = rtcTime.second();
  
  time_t unixTime = mktime(&tm);
  struct timeval tv = { unixTime, 0 };
  settimeofday(&tv, nullptr);
  
  Serial.printf("Time synced from RTC: %s", ctime(&unixTime));
}

String getCurrentTimeString() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  
  char buffer[32];
  strftime(buffer, sizeof(buffer), "%H:%M:%S", timeinfo);
  return String(buffer);
}

String getCurrentDateString() {
  time_t now = time(nullptr);
  struct tm* timeinfo = localtime(&now);
  
  char buffer[32];
  strftime(buffer, sizeof(buffer), "%Y-%m-%d", timeinfo);
  return String(buffer);
}

bool setManualTime(const String& datetime) {
  struct tm tm = {};
  
  // Parse YYYY-MM-DDTHH:MM or YYYY-MM-DD HH:MM:SS format
  int year, month, day, hour, minute, second = 0;
  
  if (sscanf(datetime.c_str(), "%d-%d-%dT%d:%d:%d", 
             &year, &month, &day, &hour, &minute, &second) >= 5 ||
      sscanf(datetime.c_str(), "%d-%d-%d %d:%d:%d", 
             &year, &month, &day, &hour, &minute, &second) >= 5) {
    
    tm.tm_year = year - 1900;
    tm.tm_mon = month - 1;
    tm.tm_mday = day;
    tm.tm_hour = hour;
    tm.tm_min = minute;
    tm.tm_sec = second;
    
    time_t unixTime = mktime(&tm);
    struct timeval tv = { unixTime, 0 };
    
    if (settimeofday(&tv, nullptr) == 0) {
      // Update RTC if available
      if (rtc.begin()) {
        rtc.adjust(DateTime(unixTime));
      }
      
      Serial.printf("Manual time set: %s\n", getCurrentTimeString().c_str());
      return true;
    }
  }
  
  Serial.println("Failed to parse or set manual time");
  return false;
}
