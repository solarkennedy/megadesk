#define FASTLED_ALLOW_INTERRUPTS 0
#define POWER_DEBUG_PRINT 1
#define FASTLED_INTERRUPT_RETRY_COUNT 1

#define COOLING 20 // defines the level at which the lighting effect fades before a new "flame" generates
#define SPARKING 20

#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_INTERRUPT_RETRY_COUNT 1
#include <FastLED.h>

int wait = 0;
int faderate = 40000;

#define DATA_PIN 0
#define NUM_LEDS 300
#define MAX_POWER_MILLIAMPS 50000
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB

CRGB leds[NUM_LEDS];

uint8_t gHue = 0; // rotating "base color" used by many of the patterns

CRGBPalette16 gPal;

bool override_mode = false;
int override_timer = 0;
int override_duration = 3600;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  setupSerial();
  setupStrip();
  setupWifi();
  syncTimeFromWifi();
  digitalWrite(LED_BUILTIN, HIGH);
}

uint8 getBrightnessOverride(uint8 original)
{
  if (override_timer != 0)
  {
    EVERY_N_SECONDS(1)
    {
      override_timer--;
    }
  }
  int b;
  Serial.print("Override timer: ");
  Serial.print(override_timer);
  Serial.print("  Mode: ");
  Serial.println(override_mode);
  if (override_timer == 0)
  {
    b = original;
  }
  else if (override_mode)
  {
    // The first 30 seconds of override, we fade down
    if (override_timer > (override_duration - 30))
    {
      Serial.println(override_timer - (override_duration - 30));
      b = map(override_timer - (override_duration - 30), 30, 0, int(original), 0);
    }
    else
    { // after the first seconds of the override
      b = 0;
    }
  }
  else
  { // override canceld
    if (original == 0)
    {
      // fading back to black
      b = map(override_timer, 10, 0, 255, original);
    }
    else
    {
      // Fading back to whatever we had before
      b = map(override_timer, 10, 0, 0, original);
    }
  }
  Serial.print("Override brightness: ");
  Serial.print(b);
  Serial.print("  - original: ");
  Serial.println(original);
  return b;
}

void figureOutWhatToShow()
{
  uint8 brightness;
  uint8 final_brightness;
  uint8 h = getHour();
  int wakeup_hour = getWakeupHour();

  if (h >= 0 && h < wakeup_hour)
  {
    final_brightness = getBrightnessOverride(0);
    FastLED.setBrightness(final_brightness);
    pacifica_loop();
  }
  else if (h >= wakeup_hour && h < wakeup_hour + 1)
  {
    brightness = map(getMinuteOfTheHour() * 60 + getSecond(), 0, 3600, 0, 255);
    final_brightness = getBrightnessOverride(brightness);
    FastLED.setBrightness(final_brightness);
    pacifica_loop();
  }
  else if (h >= wakeup_hour + 1 && h < 21)
  {
    final_brightness = getBrightnessOverride(255);
    FastLED.setBrightness(final_brightness);
    flame_loop();
  }
  else if (h >= 21 && h < 22)
  {
    brightness = map(getMinuteOfTheHour() * 60 + getSecond(), 0, 3600, 255, 0);
    final_brightness = getBrightnessOverride(brightness);
    FastLED.setBrightness(final_brightness);
    flame_loop();
  }
  else
  {
    final_brightness = getBrightnessOverride(0);
    FastLED.setBrightness(final_brightness);
    flame_loop();
  }
}

void toggleOverride()
{
  if (override_timer == 0)
  {
    // Toggling in
    override_timer = 3600;
  }
  else
  {
    // Toggling out, give us some seconds to wakeup
    override_timer = 10;
  }
  override_mode = !override_mode;
}

void loop()  {
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
