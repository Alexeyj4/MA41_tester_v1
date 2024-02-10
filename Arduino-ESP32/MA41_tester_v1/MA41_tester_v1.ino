//tests: I, adf tst 50, at addr - exe x sys info, alm
#define CONTROL_CHAR 13 //у Женьки управляющий символ в МАУПах в UART: \r CR
#define BLE_MODULE_NAME MA41_tester
#define OLED_DISPLAY_TYPE SSD1306
#define I_MEAS_MIN 95
#define I_MEAS_MAX 125
#define I_ADC_TO_MA_COEF 3.2//перевод ADC в мА I=ADC/i_adc_to_ma_coef
#define PRINT_PAUSE 500//пауза при выводе новой строки
#include <Oled.h>
#include <Ble.h>
#include <AbleButtons.h>
const gpio_num_t BTN_PIN=GPIO_NUM_19; //кнопка энкодера. иначе не работает выход из спящего режима, если просто int 19 указать
const int SW_PIN=18;
const int UART1_RX_PIN=32;
const int UART1_TX_PIN=33;
const int I_MEAS_PIN=36;

int test_i_result;
String test_adf50_result;

String intMArecvdStr="";
String extMArecvdStr="";
bool intMArecvdFlag=0;//получена строка с внутреннего МА
bool extMArecvdFlag=0;//получена строка с внешнего МА
bool extMArecvdATADDRflag=0;

Oled oled;
Ble ble;

void setup();
void loop();
void maUpdate();
String intMAread();
String extMAread();
void MAclr_read_buffer();
void intMAclr_read_buffer();
void extMAclr_read_buffer();
void intMAsend(String s);
void extMAsend(String s);
int test_i();
int test_adf50();
int tests();
//void buttonableCallback(Button::CALLBACK_EVENT event, uint8_t id);

// Identify which buttons you are using...
using Button = AblePullupCallbackButton;
using ButtonList = AblePullupCallbackButtonList;
// Declaration of callback function defined later.
void buttonableCallback(Button::CALLBACK_EVENT, uint8_t);
Button btn(int(BTN_PIN), buttonableCallback); // The button to check.

void setup() { 
  Serial.begin(115200); 
  Serial1.begin(115200, SERIAL_8N1, UART1_RX_PIN, UART1_TX_PIN);//* UART1  -> Serial1 //RX Pin //TX Pin //Внешний МА41, который нужно проверить
  Serial2.begin(115200, SERIAL_8N1); //Внутренний МА41
  ble.begin();
  oled.begin();
  pinMode(BTN_PIN,INPUT_PULLUP);
  pinMode(SW_PIN,INPUT_PULLUP);
  pinMode(I_MEAS_PIN,INPUT);
  btn.begin();
  oled.print(0,"Подключите");
  oled.print(1,"МАУП");
  oled.print(2,"Нажмите");
  oled.print(3,"кнопку");
  oled.update();
}

void loop() {  
  if(ble.recvd()!=""){
    oled.prints(ble.recvd());    
    ble.clr();
  }
  maUpdate();
  extMAclr_read_buffer();
  intMAclr_read_buffer();
  oled.update();  
  btn.handle(); 
}

void maUpdate(){
  char c;
  while(Serial2.available()){    
    intMArecvdFlag=0;
    c=Serial2.read();    
    if(c==CONTROL_CHAR){      
      intMArecvdFlag=1;
      Serial.print("int:");
      Serial.println(intMArecvdStr);
      break;
    }
    intMArecvdStr=intMArecvdStr+String(c);
  }
  while(Serial1.available()){    
    extMArecvdFlag=0;
    c=Serial1.read();    
    if(c==CONTROL_CHAR){
      extMArecvdFlag=1;
      Serial.print("ext:");
      Serial.println(extMArecvdStr);
      break;
    }
    extMArecvdStr=extMArecvdStr+String(c);
  }
}

String intMAread(){
  if(intMArecvdFlag==0){return "";}
  String s=intMArecvdStr;
  intMArecvdStr="";
  intMArecvdFlag=0;
  return s;  
}

String extMAread(){
  if(extMArecvdFlag==0){return "";}
  String s=extMArecvdStr;
  extMArecvdStr="";
  extMArecvdFlag=0;
  return s;  
}

void MAclr_read_buffer(){
  intMAclr_read_buffer();
  extMAclr_read_buffer(); 
}

void intMAclr_read_buffer(){
  while(Serial2.available()){
    Serial2.read();
  }
  intMArecvdStr="";
  intMArecvdFlag=0;
}

void extMAclr_read_buffer(){
  while(Serial1.available()){
    Serial1.read();
  }
  extMArecvdStr="";
  extMArecvdFlag=0;
}

void intMAsend(String s){
  Serial2.print(s);    
  Serial2.write(CONTROL_CHAR);    
}
void extMAsend(String s){
  Serial1.print(s);  
  Serial1.write(CONTROL_CHAR);    
}
int test_i(){
  test_i_result=0;
  int i_sum=0;
  for(int i=1;i<=10;i++){
    delay(10);
    i_sum=i_sum+analogRead(I_MEAS_PIN);        
  }  
  int test_i_rezult=int(i_sum/10);  
  if( (test_i_rezult>=I_MEAS_MIN) && (test_i_rezult<=I_MEAS_MAX)){return 1;}
    else{return 0;}
}

int test_adf50(){
  test_adf50_result="";  
  MAclr_read_buffer();
  for(int i=1;i<=5;i++){          
    MAclr_read_buffer;
    intMAsend("exe 0x0007 adf tst 50");    
    delay(5000);    
    for(int j=1;j<=5;j++){      
      maUpdate();
      extMAclr_read_buffer();
      if(intMArecvdFlag==1){
        String s=intMAread();
        maUpdate();
        oled.prints(s);//debug  
        if(s.startsWith("[0] [0] ")){
          test_adf50_result=s.substring(8,16);          
          if(test_adf50_result=="50/50/50"){
            return 1;
          }          
        }
      }
    }
    
  }    
  return 0; 
}

int tests(){  
  oled.clear();
  delay(PRINT_PAUSE);
  if(test_i()){ oled.prints("I тест-ok"); }
    else { 
      oled.prints( "I="+String ( int ( test_i_result/I_ADC_TO_MA_COEF) ) + " мА" ) ;
      delay(PRINT_PAUSE);
      oled.prints("I тест-плох"); 
      //return 0;//debug
  }

  if(test_adf50()){ oled.prints("adf tst-ok"); }
    else { 
      oled.prints( "adf="+test_adf50_result) ;
      delay(PRINT_PAUSE);
      oled.prints("adf tst-плох"); return 0;
  } 

  return 1;
}

void buttonableCallback(Button::CALLBACK_EVENT event, uint8_t id) {
  if(event == Button::PRESSED_EVENT) {    
    tests();    
  }        
  static int msg_iter=0;
  msg_iter++;
  ble.send("test"+String(msg_iter)); 
} 
