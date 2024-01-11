#include <Oled.h>
#include <Ble.h>
#include <AbleButtons.h>
const gpio_num_t btn_pin=GPIO_NUM_19; //иначе не работает выход из спящего режима, если просто int 19 указать

Oled oled;
Ble ble;

// Identify which buttons you are using...
using Button = AblePullupCallbackButton;
using ButtonList = AblePullupCallbackButtonList;
// Declaration of callback function defined later.
void buttonableCallback(Button::CALLBACK_EVENT, uint8_t);
Button btn(int(btn_pin), buttonableCallback); // The button to check.

void setup() { 
  Serial.begin(115200);   
  ble.begin();
  oled.begin();
  pinMode(btn_pin,INPUT_PULLUP);
  btn.begin();
}

void loop() {
  oled.update();  
  delay(500);
  btn.handle();     
}

void buttonableCallback(Button::CALLBACK_EVENT event, uint8_t id) {
  if(event == Button::PRESSED_EVENT) {
    Serial.println("button pressed");
    ble.send(const String& "aaaaa");    
  } 
}
