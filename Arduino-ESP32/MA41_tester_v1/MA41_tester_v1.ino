#define CONTROL_CHAR 13 //у Женьки управляющий символ в МАУПах в UART: \r CR
#define BLE_MODULE_NAME MA41_tester
#define OLED_DISPLAY_TYPE SSD1306
#include <Oled.h>
#include <Ble.h>
#include <AbleButtons.h>
const gpio_num_t btn_pin=GPIO_NUM_19; //кнопка энкодера. иначе не работает выход из спящего режима, если просто int 19 указать
const int sw_pin=18;
const int uart1_rx_pin=32;
const int uart1_tx_pin=33;
const int i_meas_pin=36;

String intMArecvdStr="";
String extMArecvdStr="";
bool intMArecvdFlag=0;//получена строка с внутреннего МА
bool extMArecvdFlag=0;//получена строка с внешнего МА

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
  maUpdate();
  oled.update();  
  btn.handle(); 
  if(extMArecvdFlag==1){    
    oled.prints(extMAread());
    //Serial.println(extMAread());//debug
  }    
}

void maUpdate(){
  char c;
  if(Serial2.available()){    
    intMArecvdFlag=0;
    c=Serial2.read();    
    if(c==CONTROL_CHAR){      
      intMArecvdFlag=1;
      return;
    }
    intMArecvdStr=intMArecvdStr+String(c);
  }
  if(Serial1.available()){    
    extMArecvdFlag=0;
    c=Serial1.read();    
    if(c==CONTROL_CHAR){
      extMArecvdFlag=1;
      return;
    }
    extMArecvdStr=extMArecvdStr+String(c);
  }
}

String intMAread(){
  String s=intMArecvdStr;
  intMArecvdStr="";
  intMArecvdFlag=0;
  return s;  
}

String extMAread(){
  String s=extMArecvdStr;
  extMArecvdStr="";
  extMArecvdFlag=0;
  return s;  
}

void intMAsend(String s){
  Serial2.print(s);    
  Serial2.write(CONTROL_CHAR);    
}
void extMAsend(String s){
  Serial1.print(s);  
  Serial1.write(CONTROL_CHAR);    
}

void buttonableCallback(Button::CALLBACK_EVENT event, uint8_t id) {
  if(event == Button::PRESSED_EVENT) {
    extMAsend("at addr");
  }        
  static int msg_iter=0;
  msg_iter++;
  ble.send("test"+String(msg_iter)); 
} 
