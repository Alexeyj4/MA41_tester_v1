#include <Oled.h>
#include <Ble.h>

Oled oled;
Ble ble;

void setup() {    
  ble.begin();
  oled.begin();
}

void loop() {
  //oled.update();  
  delay(5000);
  //ble.send("aaaaa");
  delay(5000);  
}
