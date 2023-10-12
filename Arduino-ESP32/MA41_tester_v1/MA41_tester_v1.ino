#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <FontsRus/TimesNRCyr7.h>//high. not long
#include <FontsRus/CourierCyr8.h>//long. not high
//#include <FontsRus/FreeMono8.h>//long. not high
//#include <FontsRus/FreeSans8.h>//high. not long
//#include <FontsRus/FreeSerif8.h>//high. not long
const int sw_pin=18;
const int btn_pin=19;
const int uart1_rx_pin=32;
const int uart1_tx_pin=33;
const int i_meas_pin=36;


const int SCREEN_WIDTH=128; // OLED display width, in pixels
const int SCREEN_HEIGHT=64; // OLED display height, in pixels
const int first_string=12;  //first string on LCD
const int second_string=28;  //second string on LCD
const int third_string=44;  //third string on LCD
const int fourth_string=62;  //fourth string on LCD

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)


void setup() {
  // put your setup code here, to run once:
  pinMode(sw_pin,INPUT_PULLUP);
  pinMode(btn_pin,INPUT_PULLUP);
  pinMode(btn_pin,INPUT_PULLUP);
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
  

}

void loop() {
  // put your main code here, to run repeatedly:
  if(Serial1.available()){
    Serial.write(Serial1.read());
  }
  if(Serial.available()){
    Serial1.write(Serial.read());
  }
}
