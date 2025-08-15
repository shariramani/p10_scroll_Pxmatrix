
#ifndef TZ_MAPPINGS_H
#define TZ_MAPPINGS_H

#include <Arduino.h>

struct TimezoneMapping {
  const char* iana;
  const char* posix;
  const char* description;
};

// Common timezone mappings (IANA -> POSIX)
const TimezoneMapping tzList[] = {
  {"UTC", "UTC0", "Coordinated Universal Time"},
  {"America/New_York", "EST5EDT,M3.2.0,M11.1.0", "Eastern Time"},
  {"America/Chicago", "CST6CDT,M3.2.0,M11.1.0", "Central Time"},
  {"America/Denver", "MST7MDT,M3.2.0,M11.1.0", "Mountain Time"},
  {"America/Los_Angeles", "PST8PDT,M3.2.0,M11.1.0", "Pacific Time"},
  {"Europe/London", "GMT0BST,M3.5.0/1,M10.5.0", "Greenwich Mean Time"},
  {"Europe/Paris", "CET-1CEST,M3.5.0,M10.5.0/3", "Central European Time"},
  {"Europe/Berlin", "CET-1CEST,M3.5.0,M10.5.0/3", "Central European Time"},
  {"Asia/Tokyo", "JST-9", "Japan Standard Time"},
  {"Asia/Shanghai", "CST-8", "China Standard Time"},
  {"Asia/Kolkata", "IST-5:30", "India Standard Time"},
  {"Asia/Dubai", "GST-4", "Gulf Standard Time"},
  {"Australia/Sydney", "AEST-10AEDT,M10.1.0,M4.1.0/3", "Australian Eastern Time"},
  {"Australia/Perth", "AWST-8", "Australian Western Time"},
  {"Pacific/Auckland", "NZST-12NZDT,M9.5.0,M4.1.0/3", "New Zealand Time"},
  {"America/Sao_Paulo", "BRT3BRST,M10.3.0/0,M2.3.0/0", "Brazil Time"},
  {"America/Mexico_City", "CST6CDT,M4.1.0,M10.5.0", "Central Standard Time"},
  {"Africa/Cairo", "EET-2", "Eastern European Time"},
  {"Asia/Bangkok", "ICT-7", "Indochina Time"},
  {"Asia/Singapore", "SGT-8", "Singapore Time"}
};

const int tzListSize = sizeof(tzList) / sizeof(TimezoneMapping);

#endif
