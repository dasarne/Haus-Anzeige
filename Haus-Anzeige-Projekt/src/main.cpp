#include <Arduino.h>
#include "HausUI.hpp"
#include "HausWifi.hpp"

HausUI myHausUI;
HausWifi myHausWifi;

void setup()
{
  Serial.begin(115200);

  myHausUI.init();
  myHausWifi.init(&myHausUI);

}

void loop()
{
}
