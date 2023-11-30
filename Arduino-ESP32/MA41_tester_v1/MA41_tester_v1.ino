#include <Oled.h>
#include <Ble.h>

static bool deviceConnected;

Oled oled;
Ble ble;

void setup() {    
  ble.begin();
  oled.begin();
}

void loop() {
  //oled.update();  
  delay(1000);
  //ble.send("aaaaa");
  delay(3000);  
}
