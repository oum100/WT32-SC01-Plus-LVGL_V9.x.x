
#include <Arduino.h>
#include <stdio.h>
#include <WiFiMulti.h>
#include <WiFi.h>


bool wifiStatus;
WiFiMulti wifimulti;

void connectWiFi(WiFiMulti& wifiMulti, int retryLimit, bool restartOnLimit);
