//lamp alm src //[0] 0xFFFF - есть алярм по 157,325 //[0] 0x0000 - нет алярма
//tests: измерение Iпотр, at addr - считывание, AT-приём/передача, adf tst 50, приём alm

#define CONTROL_CHAR 13 //у Женьки управляющий символ в МАУПах в UART: \r CR
#define BLE_MODULE_NAME MA41_tester
#define OLED_DISPLAY_TYPE SSD1306
#define I_MEAS_MIN 95 //минимально доустимый ток
#define I_MEAS_MAX 125 //максимально доустимый ток
#define I_ADC_TO_MA_COEF 3.2//перевод ADC в мА I=ADC/i_adc_to_ma_coef
#define PRINT_PAUSE 500//пауза при выводе новой строки
#define TEST_RETRY 15 //кол-во попыток каждого теста
#define TEST_AT_RETRY 30 //кол-во попыток каждого теста
#define READ_STRING_RETRY 10 //попытки найти нужную строку среди ненужных
#define ADF_TST_WAITING 2000 //время ожидания прохождения adf tst 50
#define AT_TEST_WAITING 1000 //время ожидания прохождения at-теста

#include <Oled.h>
#include <Ble.h>
#include <AbleButtons.h>

const gpio_num_t BTN_PIN=GPIO_NUM_19; //кнопка энкодера. gpio_num_t а не int иначе не работает выход из спящего режима, если просто int 19 указать
const int SW_PIN=18;
const int UART1_RX_PIN=32;
const int UART1_TX_PIN=33;
const int I_MEAS_PIN=36;
const String COMMAND_START_TEST="start"; //формат команды с BLE - начало теста. Формат: "start0x0007"

int test_i_result; //результат теста тока потребления
String test_adf50_result; //результат теста ADF TST 50

String intMArecvdStr="";//буфер. Строка, полученная из МА.
String extMArecvdStr="";//-//-
bool intMArecvdFlag=0;//если получена строка с внутреннего МА (если получен в конце упр.символ) 
bool extMArecvdFlag=0;//если получена строка с внешнего МА (если получен в конце упр.символ)
String extMArecvdATADDR="";//строка, полученная при чтении AT ADDR
bool extMArecvdATADDRflag=0;//если с МА считан AT ADDR успешно

Oled oled;
Ble ble;

void setup();
void loop();
void maUpdate(); //cитывает из обоих UART строку до управляющего символа (если она там есть). Помещает её в буфер intMArecvdStr и extMArecvdStr.  Ставит флаги intMArecvdFlag extMArecvdFlag
String intMAread(); //возвращает считанную строку. Стирает строку в буфере intMArecvdStr. Сбрасывает флаг intMArecvdFlag
String extMAread(); // -//-
void MAclr_read_buffer(); //делает (ниже) для обоих МА
void intMAclr_read_buffer(); //Стирает строку в буфере intMArecvdStr. Сбрасывает флаг intMArecvdFlag
void extMAclr_read_buffer(); // -//-
void intMAsend(String s); //отправляет строку. Добавляет в конце упр.символ
void extMAsend(String s); //-//-
int test_i(); //проверяет ток потребления 1-ок
int test_read_ataddr(); //считывает At ADDR 1-ок заполняет extMArecvdATADDR extMArecvdATADDRflag
int test_at(); //проверяет ответ по AT86 1-ок
int test_alarm();//проверят, принял ли МАУП alarm по 157.325МГц
int tests(String serial); //проводит последовательно все тесты. Выводит сообщения на экран
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
    delay(100);
    String command_from_ble="";
    command_from_ble=ble.recvd();
    ble.clr();
    oled.prints(command_from_ble);//debug
    if(command_from_ble.startsWith(COMMAND_START_TEST)){
      command_from_ble.replace(COMMAND_START_TEST, "");
      if(tests(command_from_ble)==1){
        ok_message();   
      } else{
        not_ok_message();
      }     
    }    
  }
  maUpdate();
  intMAread();
  extMAread();
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
  extMAclr_read_buffer();
  intMAclr_read_buffer(); 
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
  test_i_result=int(i_sum/10);  
  if( (test_i_result>=I_MEAS_MIN) && (test_i_result<=I_MEAS_MAX)){
    return 1;
    } else{return 0;}
}

int test_read_ataddr(){
  extMArecvdATADDR="";
  extMArecvdATADDRflag=0;    
  MAclr_read_buffer();  
  for(int i=1;i<=TEST_RETRY;i++){          //делает неск.тестов, т.к. иногда из-за помех м.б. сбои    
    extMAsend("at addr");
    delay(50);
    for(int i=1;i<=READ_STRING_RETRY;i++){          //делает неск.считываний, т.к. получает эхо и др.
        maUpdate();
        String s=extMAread(); 
        if(s.startsWith("[0] ")){
          extMArecvdATADDR=s.substring(4,10);               
          extMArecvdATADDRflag=1;          
          return 1;
        }
    }
  }
  return 0;
  
}

int test_at(){
  if(extMArecvdATADDRflag==0){return 0;} //AT ADDR не был считан. Тест не получится провести.
  String str_to_send="";  
  MAclr_read_buffer();
  for(int i=1;i<=TEST_AT_RETRY;i++){          //делает неск.тестов, т.к. иногда из-за помех м.б. 49/50/50, например, а не 50/50/50
    MAclr_read_buffer;
    str_to_send="exe ";
    str_to_send=str_to_send+extMArecvdATADDR;
    str_to_send=str_to_send+" at addr";
    intMAsend(str_to_send);    
    delay(AT_TEST_WAITING);    //таймаут
    for(int i=1;i<=READ_STRING_RETRY;i++){      //считывает строку несколько раз, пока не увидит ответ [0] [0]. Т.к. приходит эхо и могут прийти информационные сообщения
      maUpdate();
      extMAclr_read_buffer();
      if(intMArecvdFlag==1){
        String s=intMAread();        
        if(s.startsWith("[0] [0] ")){
            return 1;                    
        }        
      }
    }
    
  }    
  return 0; 
}  


int test_adf50(){
  if(extMArecvdATADDRflag==0){return 0;} //AT ADDR не был считан. Тест не получится провести.
  String str_to_send="";
  test_adf50_result="";  
  MAclr_read_buffer();
  for(int i=1;i<=TEST_RETRY;i++){          //делает неск.тестов, т.к. иногда из-за помех м.б. 49/50/50, например, а не 50/50/50
    MAclr_read_buffer;
    str_to_send="exe ";
    str_to_send=str_to_send+extMArecvdATADDR;
    str_to_send=str_to_send+" adf tst 50";
    intMAsend(str_to_send);    
    delay(ADF_TST_WAITING);    //таймаут. раньше не успевает провести 50 тестов ADF
    for(int i=1;i<=READ_STRING_RETRY;i++){      //считывает строку несколько раз, пока не увидит ответ 50/50/50. Т.к. приходит эхо и могут прийти информационные сообщения
      maUpdate();
      extMAclr_read_buffer();
      if(intMArecvdFlag==1){
        String s=intMAread();        
        if(s.startsWith("[0] [0] ")){
          test_adf50_result=s.substring(8,16);          
          if(test_adf50_result=="50/50/50"){
            return 1;
          }          
        }
        if(s.startsWith("alarms 0x01, slot -1[0] [0] ")){
          test_adf50_result=s.substring(28,36);          
          if(test_adf50_result=="50/50/50"){
            return 1;
          }          
        }        
      }
    }
    
  }    
  return 0; 
}

int test_alarm(){ 
  if(extMArecvdATADDRflag==0){return 0;} //AT ADDR не был считан. Тест не получится провести.
  String str_to_send="";  
  MAclr_read_buffer();
  for(int i=1;i<=TEST_RETRY;i++){          //делает неск.тестов, т.к. алярм не сразу принимается
    delay(1000);//пауза для принятия алярма
    MAclr_read_buffer;
    str_to_send="exe ";
    str_to_send=str_to_send+extMArecvdATADDR;
    str_to_send=str_to_send+" lamp alm src";
    intMAsend(str_to_send);    
    delay(50);    //таймаут
    for(int i=1;i<=READ_STRING_RETRY;i++){      //считывает строку несколько раз, пока не увидит ответ [0] [0]. Т.к. приходит эхо и могут прийти информационные сообщения
      maUpdate();
      extMAclr_read_buffer();
      if(intMArecvdFlag==1){
        String s=intMAread();        
        if(s.startsWith("[0] [0] ")){
          if(s.substring(8,14)=="0xFFFF") {
            return 1;
          }                               
        }        
      }
    }
    
  }    
  return 0; 
} 

int tests(String serial){  
  oled.clear();
  oled.prints("Тест "+serial+"...");
  ble.send("Start testing "+serial+"..."); 
  delay(PRINT_PAUSE);
  if(test_i()){ 
    oled.prints("I ТЕСТ-OK");
    ble.send("Power test - ok");
    delay(PRINT_PAUSE);
  } else { 
      oled.prints( "I="+String ( int ( test_i_result/I_ADC_TO_MA_COEF) ) + " мА" ) ;
      ble.send( "I="+String ( int ( test_i_result/I_ADC_TO_MA_COEF) ) + " мА" ) ;
      delay(PRINT_PAUSE);            
      oled.prints("I ТЕСТ-ПЛОХ"); 
      ble.send("Power test - bad");
      delay(PRINT_PAUSE);
      return 0;
  }

  if(serial==""){//если serial получен с BLE - то не надо делать считывание
    if(test_read_ataddr()){
      String s;
      s="AT="+extMArecvdATADDR;
      oled.prints(s);
      ble.send(s);
      delay(PRINT_PAUSE);
      } else { 
        oled.prints( "AT ADDR-ПЛОХ"); 
        ble.send( "AT ADDR reading - bad"); 
        delay(PRINT_PAUSE);
        return 0;
    } 
  }else{
    extMArecvdATADDR=serial;
    extMArecvdATADDRflag=1;     
  }

  if(test_at()){    
    oled.prints("AT-OK"); 
    ble.send("AT test - ok"); 
    delay(PRINT_PAUSE);
    } else { 
      oled.prints( "AT-ПЛОХ"); 
      ble.send( "AT test - bad"); 
      delay(PRINT_PAUSE);
      return 0;
  } 

  
  
  if(test_adf50()){ 
    oled.prints("ADF TST-OK"); 
    ble.send("ADF test 50/50 - ok"); 
    delay(PRINT_PAUSE);
    } else { 
      oled.prints( "ADF="+test_adf50_result) ;
      ble.send( "ADF="+test_adf50_result) ;
      delay(PRINT_PAUSE);      
      oled.prints("ADF TST-ПЛОХ"); 
      ble.send("ADF test 50/50 - bad"); 
      delay(PRINT_PAUSE);
      return 0;
    }
 
  
  if(digitalRead(SW_PIN)==0){
    if(test_alarm()){ 
      oled.prints("ALARM-OK");
      ble.send("ALARM receive - ok");
      delay(PRINT_PAUSE);
    } else { 
      oled.prints("ALARM-ПЛОХ"); 
      ble.send("ALARM receive - bad"); 
      delay(PRINT_PAUSE);      
      return 0;
    }
  }

  return 1;  
}

void ok_message(){
      
      
      
      oled.prints("ГОДНЫЙ");    
      ble.send("ГОДНЫЙ");  
}
void not_ok_message(){

      
      oled.prints(String(extMArecvdATADDR.substring(2).toInt()));//debug
      
      oled.prints("БРАК");  
      ble.send("БРАК");   
}

void buttonableCallback(Button::CALLBACK_EVENT event, uint8_t id) {
  if(event == Button::PRESSED_EVENT) {    
    if(tests("")==1){
      ok_message();
    } else{
      not_ok_message();
    }
  }        

} 
