#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//#include <FontsRus/TimesNRCyr7.h>//high. not long
#include <FontsRus/CourierCyr8.h>//long. not high
//#include <FontsRus/FreeMono8.h>//long. not high
//#include <FontsRus/FreeSans8.h>//high. not long
//#include <FontsRus/FreeSerif8.h>//high. not long

const int SCREEN_WIDTH=128; // OLED display width, in pixels
const int SCREEN_HEIGHT=64; // OLED display height, in pixels
const int first_string=12;  //first string on LCD
const int second_string=28;  //second string on LCD
const int third_string=44;  //third string on LCD
const int fourth_string=62;  //fourth string on LCD

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)


void setup() {
  // put your setup code here, to run once:
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

}
