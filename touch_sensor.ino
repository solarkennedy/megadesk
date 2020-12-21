#include <CapacitiveSensor.h>
CapacitiveSensor touch_sensor = CapacitiveSensor(D5, D6);
bool touched = false;

void setupTouchSensor() {
  touch_sensor.set_CS_Timeout_Millis(100);
}

bool isTouched() {
  float t = touch_sensor.capacitiveSensor(30);
  bool s = t > -2;
  if (touched == true && s == false) {
    touched = false;
    Serial.println("Finger Untouched!");
    digitalWrite(LED_BUILTIN, HIGH);
    delay(100);
    return true;
  } else if (touched == true && s == true) {
    // finger still down
    return false;
  } else if (touched == false && s == true) {
    Serial.println("Finger applied!");
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    touched = true;
    return false;
  } else if (touched == false && s == false) {
    // finger still up
    return false;
  }
}
