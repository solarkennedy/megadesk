#define POWER_DEBUG_PRINT 1
#define FASTLED_INTERRUPT_RETRY_COUNT 1

#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_INTERRUPT_RETRY_COUNT 1
#include <FastLED.h>

#define DATA_PIN 0
#define NUM_LEDS 216
#define MAX_POWER_MILLIAMPS 50000
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

CRGBPalette16 gPal;

bool RemoteToggle = true;
int faderate = 5;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  setupSerial();
  setupStrip();
  setupWifi();
  syncTimeFromWifi();
  sendPushNotification();
  digitalWrite(LED_BUILTIN, HIGH);
}

int calculateBrightness(int b, int rate) {
  EVERY_N_MILLISECONDS(200) {
    b += rate;
  }
  if (b < 0) {
    b = 0;
  } else if (b > 255) {
    b = 255;
  }
  return b;
}

void figureOutWhatToShow()
{
  uint8 brightness;
  uint8 h = getHour();
  int wakeup_hour = getWakeupHour();

  if (h >= 0 && h < wakeup_hour)
  {
    brightness = 0;
  }
  else if (h >= wakeup_hour && h < wakeup_hour + 1)
  {
    brightness = map(getMinuteOfTheHour() * 60 + getSecond(), 0, 3600, 0, 255);
    pacifica_loop();
  }
  else if (h >= wakeup_hour + 1 && h < 21)
  {
    brightness = 255;
    flame_loop();
  }
  else if (h >= 21 && h < 22)
  {
    brightness = map(getMinuteOfTheHour() * 60 + getSecond(), 0, 3600, 255, 0);
    flame_loop();
  }
  else
  {
    brightness = 0;
  }
  brightness = calculateBrightness(brightness, faderate);
  FastLED.setBrightness(brightness);
}

void reactToRemote() {
  if (RemoteToggle) {
    faderate = 10;
  } else {
    faderate = -10;
  }
}

void loop()  {
  reactToRemote();
  figureOutWhatToShow();
  FastLED.show();
  wifiEvents();
  EVERY_N_SECONDS(3600)
  {
    syncTimeFromWifi();
  }
}

void setupSerial()
{
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
  Serial.println();
  Serial.println();
  const char compile_date[] = __DATE__ " " __TIME__;
  Serial.printf("ESP8266 Chip id = %08X\n", ESP.getChipId());
  Serial.printf("Sketch compiled %s\n", compile_date);
}

void setupStrip()
{
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS)
  .setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(255);
  FastLED.setMaxPowerInVoltsAndMilliamps(5, MAX_POWER_MILLIAMPS);
}
