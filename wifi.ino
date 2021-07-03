#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoHA.h>
#include "secrets.h"

HADevice device;
WiFiClient wifiClient;
HAMqtt mqtt(wifiClient, device);
HALight led("MegaDesk", false);

void wifiEvents() {
  ArduinoOTA.handle();
  mqtt.loop();
}

void setupWifi() {
  Serial.println("Setting up wifi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_ESSID, WIFI_PASSWORD);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    for (int i = 0; i < 25; i++ ) {
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
    }
    ESP.restart();
  }
  char hostname[32];
  sprintf(hostname, "megadesk_%08X", ESP.getChipId());
  ArduinoOTA.setHostname(hostname);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  setupMQTT();
}

void sendPushNotification() {
  BearSSL::WiFiClientSecure client;
  client.setInsecure();
  HTTPClient http;
  http.begin(client, "gotify.xkyle.com", 443, "/message?token="GOTIFY_TOKEN);
  http.addHeader("Content-Type", "application/json");
  uint16_t httpResponseCode;

  httpResponseCode = http.POST("{\"message\": \"Megadesk Booted\",\"title\": \"Megadesk Booted\",\"priority\": 1}");
  if (httpResponseCode > 0) {
    String response = http.getString();
    Serial.println(httpResponseCode);
    Serial.println(response);
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

void setupMQTT() {
  device.setName("MegaDesk");
  device.setAvailability(true);
  byte mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);
  device.setUniqueId(mac, sizeof(mac));
  led.setName("Desk LEDs");
  led.onStateChanged(onSwitchStateChanged);
  mqtt.begin(MQTT_HOST);
}

void onSwitchStateChanged(bool state)
{
  RemoteToggle = state;
  if (state) {
    digitalWrite(LED_BUILTIN, LOW);
  } else {
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
