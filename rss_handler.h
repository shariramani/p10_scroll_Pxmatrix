
#ifndef RSS_HANDLER_H
#define RSS_HANDLER_H

#include "config.h"

// Function declarations
void fetchAllRSSFeeds();
void handleFeedFetch(const RSSFeed& feed);
bool isRecentNews(const char* pubDate);
void extractTitlesFallback(String& payload, const char* name);
void sanitizeXmlNamespaces(String& xml);

#endif
