#include <Arduino.h>
#include <stdio.h>
#include <WiFiMulti.h>



void connectWiFi(WiFiMulti& wifiMulti, int retryLimit, bool restartOnLimit) {
  int retryCount = 0;

  printf("Connecting to Wi-Fi\n");

  // Keep trying to connect until the retry limit is reached
  while (wifiMulti.run() != WL_CONNECTED && retryCount < retryLimit) {
    delay(1000);
    printf(".");
    retryCount++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    char *buf[4];
    IPAddress ip = WiFi.localIP();

    printf("\nWi-Fi connected!\n");
    // sprintf(buf, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
    printf("WiFi: %s\n",WiFi.SSID());
    printf("IP address: %u.%u.%u.%u\n", ip[0], ip[1], ip[2], ip[3]);

    
    // Serial.printf("IP Address: %s",IPAddress(WiFi.localIP()));
    // Serial.printf(WiFi.localIP());
  } else {
    printf("\nFailed to connect to Wi-Fi.\n");
    
    if (restartOnLimit) {
      Serial.println("Restarting ESP32...");
      delay(1000); // Wait a second before restarting
      ESP.restart();  // Restart the ESP32
    } else {
      printf("No restart. Continuing...");
    }
  }
}
