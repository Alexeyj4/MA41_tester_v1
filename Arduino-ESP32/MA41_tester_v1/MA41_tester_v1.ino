#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "lib/FontsRus/CourierCyr8.h"
#include "lib/ESP32Encoder/ESP32Encoder.h"
#include <EEPROM.h>
#include <AbleButtons.h>
//#include "lib/Oled/Oled.h"


#define uS_TO_S_FACTOR 1000000  /* коэффициент пересчета //debug
                                   микросекунд в секунды */
RTC_DATA_ATTR int bootCount = 0;


/*********
  Руи Сантос
  Более подробно о проекте на: http://randomnerdtutorials.com  
*********/

// Подключаем необходимые библиотеки:
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

const int sw_pin=18;
const gpio_num_t btn_pin=GPIO_NUM_19; //иначе не работает выход из спящего режима, если просто int 19 указать
const int enc_a_pin=4;//debug
const int enc_b_pin=15;//debug

const int uart1_rx_pin=32;
const int uart1_tx_pin=33;
const int i_meas_pin=36;

class OLED{
  public:
    OLED(int n){
      screen_width=128;
      screen_height=64;            
      oled_string_pos[0]=12; oled_string_pos[1]=28; oled_string_pos[2]=44; oled_string_pos[3]=62;
      oled_text[0]=""; oled_text[1]=""; oled_text[2]=""; oled_text[3]=""; 
      oled_str_changed[0]=0; oled_str_changed[1]=0; oled_str_changed[2]=0; oled_str_changed[3]=0; 
      oled_need_update=0;
      Adafruit_SSD1306 display(screen_width, screen_height, &Wire, -1); // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)      
      display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
      display.setFont(&CourierCyr8pt8b);
      display.setTextSize(1);             
      display.setTextColor(WHITE);  
      display.clearDisplay();
      display.setCursor(0,oled_string_pos[0]);  
      display.println("loading OS...");
      display.display();
      delay(1000); 
    }        
    
    int screen_width; // OLED display width, in pixels
    int screen_height; // OLED display height, in pixels    
    int oled_string_pos[4];//positon for each string on oled
    String oled_text[4];//text on OLED
    bool oled_str_changed[4];//OLED strings needs to update
    bool oled_need_update;//OLED need update flag
    Adafruit_SSD1306 display;
  
    void test(){
      Adafruit_SSD1306 display(128, 64, &Wire, -1); // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)  
      display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
      display.setFont(&CourierCyr8pt8b);
      display.setTextSize(1);             
      display.setTextColor(WHITE);  
      display.clearDisplay();
      display.setCursor(0,12);  
      display.println("loading OS...");
      display.display();
      delay(1000);       
    }
    
    void update(){
      if(oled_need_update){
        display.display();
        for(int i=0;i<=3;i++){
          if(oled_str_changed[i]){
            display.setCursor(0,oled_string_pos[0]);
            display.print(oled_text[i]);
            oled_str_changed[i]=0;
          }
        }
        display.display();
        oled_need_update=0;
      }
    }
    void print(int row, String s){
      oled_need_update=1;
      oled_str_changed[row]=1;
      oled_text[row]=s;
    }
};

OLED oled1(1);

int message_i=0;

// Identify which buttons you are using...
using Button = AblePullupCallbackButton;
using ButtonList = AblePullupCallbackButtonList;

// Declaration of callback function defined later.
void buttonableCallback(Button::CALLBACK_EVENT, uint8_t);

Button btn(int(btn_pin), buttonableCallback); // The button to check.


String message_str="";

// НЕ МЕНЯЙТЕ ЭТИ UUID.
// Если поменяете, вам также нужно будет поменять их
// в Android-приложении, используемом для этого проекта:
#define SERVICE_UUID            "C6FBDD3C-7123-4C9E-86AB-005F1A7EDA01"
#define CHARACTERISTIC_UUID_RX  "B88E098B-E464-4B54-B827-79EB2B150A9F"
#define CHARACTERISTIC_UUID_TX  "D769FACF-A4DA-47BA-9253-65359EE480FB"

//ESP32Encoder encoder;




BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// функции обратного вызова, которые будут запускаться
// при подключении и отключении BLE-клиента от BLE-сервера:
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Connected");
    oled1.print(0,"Connected");
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Disconnected");
    oled1.print(0,"Disconnected");

    // Начинаем рассылку оповещений:
    pServer->getAdvertising()->start();
    Serial.println("Waiting to connect...");
    oled1.print(0,"Waiting to connect...");
    //  "Ждем подключения..."
  }
};

 //функция обратного вызова, которая будет запускаться
 //при получении нового значения от Android-приложения:
class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();
    String s;
    if(rxValue.length() > 0) {      
      for(int i = 0; i < rxValue.length(); i++) {
        s=s+rxValue[i];
      }
    }
    Serial.println(s);
    oled1.print(0,s);    
  }
};

void setup() {
  // put your setup code here, to run once:

  //ESP32Encoder::useInternalWeakPullResistors=UP; //debug
  //encoder.attachHalfQuad(enc_a_pin,enc_b_pin);
  
  btn.begin();
  
  pinMode(sw_pin,INPUT_PULLUP);
  pinMode(btn_pin,INPUT_PULLUP);  
  pinMode(enc_a_pin,INPUT_PULLUP);  
  pinMode(enc_b_pin,INPUT_PULLUP);  
  pinMode(i_meas_pin,INPUT);

  
  Serial.begin(115200, SERIAL_8N1);
//  Serial1.begin(115200, SERIAL_8N1, uart1_rx_pin, uart1_tx_pin);//* UART1  -> Serial1 //RX Pin //TX Pin //Внешний
//  Serial2.begin(115200, SERIAL_8N1); //Внутренний
   
  oled1.test();//debug
  
  // создаем BLE-устройство:
  BLEDevice::init("ESP32_Board");

  // создаем BLE-сервер:
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Создаем BLE-сервис:
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Создаем BLE-характеристику:
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY);
                      
  pCharacteristic->addDescriptor(new BLE2902());

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                               CHARACTERISTIC_UUID_RX,
                               BLECharacteristic::PROPERTY_WRITE);

  pCharacteristic->setCallbacks(new MyCallbacks());

  // Запускаем сервис:
  pService->start();

  // Начинаем рассылку оповещений:
  pServer->getAdvertising()->start();
  Serial.println("Waiting to connect...");
  oled1.print(0,"Waiting to connect...");
             //  "Ждем подключения..."
}


void loop() {
  // put your main code here, to run repeatedly:

  //oled.update();
  btn.handle();
  
  // Если устройство подключено... 
  if(deviceConnected) {
    if(Serial.available()){
      message_str=Serial.readString();
      pCharacteristic->setValue(message_str.c_str());
      // отправляем значение Android-приложению:
      pCharacteristic->notify();     
    }  
  }  
  gpio_wakeup_enable(GPIO_NUM_19,GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();
  //esp_light_sleep_start();//debug  
  //print_wakeup_reason();//debug
  //Serial.println(encoder.getCount()); //debug

}

void buttonableCallback(Button::CALLBACK_EVENT event, uint8_t id) {
  if(event == Button::PRESSED_EVENT) {
    message_i=message_i+1;    
    message_str=message_i;
    pCharacteristic->setValue(message_str.c_str());
    Serial.print(message_i);
    Serial.print("-");
    Serial.println(message_str);
    
    // отправляем значение Android-приложению:
    pCharacteristic->notify();
  } 
}
