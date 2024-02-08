#define OLED_DISPLAY_TYPE 96
#include <Oled.h>
#include <Ble.h>
#include <AbleButtons.h>
const gpio_num_t btn_pin=GPIO_NUM_19; //кнопка энкодера. иначе не работает выход из спящего режима, если просто int 19 указать
const int sw_pin=18;
const int uart1_rx_pin=32;
const int uart1_tx_pin=33;
const int i_meas_pin=36;

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
  Serial1.begin(115200, SERIAL_8N1, uart1_rx_pin, uart1_tx_pin);//* UART1  -> Serial1 //RX Pin //TX Pin //Внешний МА41, который нужно проверить
  Serial2.begin(115200, SERIAL_8N1); //Внутренний МА41
  ble.begin();
  oled.begin();
  pinMode(btn_pin,INPUT_PULLUP);
  pinMode(sw_pin,INPUT_PULLUP);
  pinMode(i_meas_pin,INPUT);
  btn.begin();  
}

void loop() {
  if(ble.recvd()!=""){
    oled.prints(ble.recvd());
    ble.clr();
  }
  oled.update();  
  delay(50);
  btn.handle();     
}

void buttonableCallback(Button::CALLBACK_EVENT event, uint8_t id) {
  if(event == Button::PRESSED_EVENT) {    
    static int msg_iter=0;
    msg_iter++;
    ble.send("test"+String(msg_iter));        
  } 
}
