#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <FontsRus/CourierCyr8.h>
#include <ESP32Encoder.h>
#include <EEPROM.h>

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


char message[127]; //debug

const int sw_pin=18;
const gpio_num_t btn_pin=GPIO_NUM_19; //иначе не работает выход из спящего режима, если просто int 19 указать
const int enc_a_pin=4;//debug
const int enc_b_pin=15;//debug

const int uart1_rx_pin=32;
const int uart1_tx_pin=33;
const int i_meas_pin=36;


const int SCREEN_WIDTH=128; // OLED display width, in pixels
const int SCREEN_HEIGHT=64; // OLED display height, in pixels
const int first_string=12;  //first string on LCD
const int second_string=28;  //second string on LCD
const int third_string=44;  //third string on LCD
const int fourth_string=62;  //fourth string on LCD

// НЕ МЕНЯЙТЕ ЭТИ UUID.
// Если поменяете, вам также нужно будет поменять их
// в Android-приложении, используемом для этого проекта:
#define SERVICE_UUID            "C6FBDD3C-7123-4C9E-86AB-005F1A7EDA01"
#define CHARACTERISTIC_UUID_RX  "B88E098B-E464-4B54-B827-79EB2B150A9F"
#define CHARACTERISTIC_UUID_TX  "D769FACF-A4DA-47BA-9253-65359EE480FB"

ESP32Encoder encoder;


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

// функции обратного вызова, которые будут запускаться
// при подключении и отключении BLE-клиента от BLE-сервера:
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Connected");
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Disconnected");
  }
};

// функция обратного вызова, которая будет запускаться
// при получении нового значения от Android-приложения:
class MyCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string rxValue = pCharacteristic->getValue();
    if(rxValue.length() > 0) {
      Serial.print("Received value: ");  //  "Полученное значение: "
      for(int i = 0; i < rxValue.length(); i++) {
        Serial.print(rxValue[i]);
      }
      // включаем и выключаем светодиод согласно полученной команде:
      if(rxValue.find("ON") != -1) { 
        Serial.println(" - LED ON");  //  " - светодиод включен"
        //digitalWrite(ledPin, HIGH);
      }
      else if(rxValue.find("OFF") != -1) {
        Serial.println(" - LED OFF");  //  " - светодиод выключен"
        //digitalWrite(ledPin, LOW);
      }
    }
  }
};


void print_wakeup_reason(){ //debug
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_GPIO: Serial.println("Wakeup caused by GPIO"); break;
    default : Serial.println("Other cause of wakeup"); break;   
  }
}



void setup() {
  // put your setup code here, to run once:

  //ESP32Encoder::useInternalWeakPullResistors=UP; //debug
  encoder.attachHalfQuad(enc_a_pin,enc_b_pin);
  
 
  pinMode(sw_pin,INPUT_PULLUP);
  pinMode(btn_pin,INPUT_PULLUP);  
  pinMode(enc_a_pin,INPUT_PULLUP);  
  pinMode(enc_b_pin,INPUT_PULLUP);  
  pinMode(i_meas_pin,INPUT);

  
  Serial.begin(115200, SERIAL_8N1);
  Serial1.begin(115200, SERIAL_8N1, uart1_rx_pin, uart1_tx_pin);//* UART1  -> Serial1 //RX Pin //TX Pin //Внешний
  Serial2.begin(115200, SERIAL_8N1); //Внутренний
   
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setFont(&CourierCyr8pt8b);
  display.setTextSize(1);             
  display.setTextColor(WHITE);  
  display.clearDisplay();
  display.setCursor(0,first_string);  
  display.println("loading OS...");
  display.display(); 
   
  //pinMode(ledPin, OUTPUT);
  //sensors.begin();
  
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
             //  "Ждем подключения..."
}


void loop() {
  // put your main code here, to run repeatedly:
  // Если устройство подключено... 
  if(deviceConnected) {
    if(Serial.available()){
      message[0]=Serial.read();
      pCharacteristic->setValue(message);
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
