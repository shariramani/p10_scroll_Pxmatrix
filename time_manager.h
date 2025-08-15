
#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include "config.h"

// Function declarations
void initializeTime();
void applyTimezone();
void syncTimeFromNTP();
void syncTimeFromRTC();
String getCurrentTimeString();
String getCurrentDateString();
bool setManualTime(const String& datetime);

#endif
