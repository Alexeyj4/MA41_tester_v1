#pragma once
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <CourierCyr8.h>

class OLED{
  public:
    OLED();
    void begin();
    void update();
    void print(int row, String s);        
    int screen_width; // OLED display width, in pixels
    int screen_height; // OLED display height, in pixels    
    int oled_string_pos[4];//positon for each string on oled
    String oled_text[4];//text on OLED
    bool oled_str_changed[4];//OLED strings needs to update
    bool oled_need_update;//OLED need update flag
    Adafruit_SSD1306 display;
};
  
OLED::OLED(){
  pinMode(2, OUTPUT);
  digitalWrite(2, HIGH);
  screen_width=128;
  screen_height=64;            
  oled_string_pos[0]=12; oled_string_pos[1]=28; oled_string_pos[2]=44; oled_string_pos[3]=62;
  oled_text[0]=""; oled_text[1]=""; oled_text[2]=""; oled_text[3]=""; 
  oled_str_changed[0]=0; oled_str_changed[1]=0; oled_str_changed[2]=0; oled_str_changed[3]=0; 
  oled_need_update=0;
}   
  
void OLED::begin(){

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

void OLED::update(){
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
void OLED::print(int row, String s){
  oled_need_update=1;
  oled_str_changed[row]=1;
  oled_text[row]=s;
}