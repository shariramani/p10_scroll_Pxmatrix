
#include "wifi_manager.h"

bool connectToWiFi() {
  String ssid = preferences.getString("ssid", "");
  String pass = preferences.getString("pass", "");
  
  if (ssid.length() == 0) {
    Serial.println("No Wi-Fi credentials stored");
    startAccessPoint();
    return false;
  }
  
  Serial.printf("Connecting to Wi-Fi: %s\n", ssid.c_str());
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), pass.c_str());
  
  unsigned long startTime = millis();
  const unsigned long timeout = 15000; // 15 seconds
  
  while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
    delay(500);
    Serial.print(".");
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("\nWi-Fi connected successfully!\n");
    Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Signal Strength: %d dBm\n", WiFi.RSSI());
    
    // Test internet connectivity
    if (isConnectedToInternet()) {
      Serial.println("Internet connectivity confirmed");
      return true;
    } else {
      Serial.println("Wi-Fi connected but no internet access");
      return false;
    }
  }
  
  Serial.println("\nWi-Fi connection failed - starting AP mode");
  startAccessPoint();
  return false;
}

void startAccessPoint() {
  Serial.println("Starting Access Point mode...");
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(AP_SSID, AP_PASS);
  
  IPAddress IP = WiFi.softAPIP();
  Serial.printf("AP IP address: %s\n", IP.toString().c_str());
  Serial.printf("Connect to Wi-Fi '%s' and visit http://%s/wifi\n", AP_SSID, IP.toString().c_str());
}

bool isConnectedToInternet() {
  HTTPClient http;
  http.begin("http://httpbin.org/get");
  http.setTimeout(5000);
  
  int httpCode = http.GET();
  http.end();
  
  return (httpCode > 0 && httpCode < 400);
}

void setupWiFiRoutes() {
  // Wi-Fi scan endpoint
  server.on("/wifiscan", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("Scanning for Wi-Fi networks...");
    int networkCount = WiFi.scanNetworks();
    
    DynamicJsonDocument doc(2048);
    JsonArray networks = doc.to<JsonArray>();
    
    for (int i = 0; i < networkCount; ++i) {
      JsonObject network = networks.createNestedObject();
      network["ssid"] = WiFi.SSID(i);
      network["rssi"] = WiFi.RSSI(i);
      network["secure"] = (WiFi.encryptionType(i) != WIFI_AUTH_OPEN);
    }
    
    String response;
    serializeJson(doc, response);
    request->send(200, "application/json", response);
    
    Serial.printf("Found %d networks\n", networkCount);
  });
  
  // Wi-Fi configuration page
  server.on("/wifi", HTTP_GET, [](AsyncWebServerRequest *request) {
    String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Wi-Fi Configuration</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; }
        .container { max-width: 500px; margin: 0 auto; }
        select, input { width: 100%; padding: 10px; margin: 5px 0; }
        button { background: #007bff; color: white; padding: 10px 20px; border: none; cursor: pointer; }
        button:hover { background: #0056b3; }
        .status { margin: 10px 0; padding: 10px; border-radius: 4px; }
        .success { background: #d4edda; color: #155724; }
        .error { background: #f8d7da; color: #721c24; }
    </style>
</head>
<body>
    <div class="container">
        <h2>Wi-Fi Configuration</h2>
        <form id="wifiForm">
            <label>Network:</label>
            <select id="ssid" required>
                <option value="">Select Network...</option>
            </select>
            <label>Password:</label>
            <input type="password" id="password" placeholder="Enter Wi-Fi password">
            <button type="submit">Connect</button>
        </form>
        <div id="status"></div>
        <button onclick="location.href='/'">Back to Home</button>
    </div>
    <script>
        // Load networks on page load
        fetch('/wifiscan')
            .then(r => r.json())
            .then(networks => {
                const select = document.getElementById('ssid');
                networks.forEach(net => {
                    const option = document.createElement('option');
                    option.value = net.ssid;
                    option.textContent = `${net.ssid} (${net.rssi} dBm)`;
                    select.appendChild(option);
                });
            });
        
        document.getElementById('wifiForm').onsubmit = function(e) {
            e.preventDefault();
            const ssid = document.getElementById('ssid').value;
            const password = document.getElementById('password').value;
            
            fetch('/wifi', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: `ssid=${encodeURIComponent(ssid)}&pass=${encodeURIComponent(password)}`
            })
            .then(r => r.text())
            .then(msg => {
                document.getElementById('status').innerHTML = 
                    `<div class="success">${msg}</div>`;
            })
            .catch(err => {
                document.getElementById('status').innerHTML = 
                    `<div class="error">Error: ${err}</div>`;
            });
        };
    </script>
</body>
</html>
    )";
    request->send(200, "text/html", html);
  });
  
  // Wi-Fi connection handler
  server.on("/wifi", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (!request->hasParam("ssid", true) || !request->hasParam("pass", true)) {
      request->send(400, "text/plain", "Missing SSID or password");
      return;
    }
    
    String ssid = request->getParam("ssid", true)->value();
    String pass = request->getParam("pass", true)->value();
    
    // Save credentials
    preferences.putString("ssid", ssid);
    preferences.putString("pass", pass);
    
    request->send(200, "text/plain", "Credentials saved. Device will restart in 3 seconds...");
    
    // Restart after delay
    delay(3000);
    ESP.restart();
  });
  
  // Wi-Fi reset
  server.on("/wifi/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    preferences.clear();
    request->send(200, "text/plain", "Wi-Fi credentials cleared. Restarting...");
    delay(2000);
    ESP.restart();
  });
}
