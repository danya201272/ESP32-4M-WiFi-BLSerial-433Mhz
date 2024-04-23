//////////////////////////////////////////////
//        RemoteXY include library          //
//////////////////////////////////////////////

// можете включить вывод отладочной информации в Serial на 115200
//#define REMOTEXY__DEBUGLOG    

// определение режима соединения и подключение библиотеки RemoteXY 
#define REMOTEXY_MODE__ESP32CORE_WIFI_POINT

#include <WiFi.h>

// настройки соединения 
#define REMOTEXY_WIFI_SSID "Keenetic-2013"
#define REMOTEXY_WIFI_PASSWORD "D201272d"
#define REMOTEXY_SERVER_PORT 6377
#define REMOTEXY_ACCESS_PASSWORD "201272"


#include <RemoteXY.h>

// конфигурация интерфейса RemoteXY  
#pragma pack(push, 1)  
uint8_t RemoteXY_CONF[] =   // 81 bytes
  { 255,4,0,131,0,74,0,17,0,0,0,16,1,200,198,1,1,5,0,10,
  132,86,47,47,48,4,36,31,74,117,109,109,101,114,0,31,74,77,58,79,
  70,70,0,1,63,86,47,47,0,247,31,83,101,110,100,0,3,6,72,31,
  112,4,149,26,4,46,157,151,22,128,163,26,67,255,25,201,7,0,2,29,
  131 };
  
// структура определяет все переменные и события вашего интерфейса управления 
struct {

    // input variables
  uint8_t btglush; // =1 если включено, иначе =0
  uint8_t sendcodes; // =1 если кнопка нажата, иначе =0
  uint8_t select_2; // =0 если переключатель в положении A, =1 если в положении B, ...
  int8_t slider_01; // oт 0 до 100

    // output variables
  char text_01[131]; // =строка UTF8 оканчивающаяся нулем

    // other variable
  uint8_t connect_flag;  // =1 if wire connected, else =0

} RemoteXY;   
#pragma pack(pop)
 
/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////
#include "BluetoothSerial.h"
BluetoothSerial SerialBT;
String RxBuffer = "";
char RxByte;
String device_name = "Iphone(Egor)";

#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();

long value;
int bitlength;
int protocol;
int pulselength;
#define txPin 32  // Передатчик
#define rxPin 33 // Приемник
int speeds = 4; // Brutforce speed
unsigned int freq = 1000; // Freq Jummer
bool jumstate = false;
bool svstate = false;

void setup()
{
  RemoteXY_Init();
  pinMode(txPin, OUTPUT);
  Serial.begin(115200);
  Serial.setTimeout(20);
  mySwitch.enableReceive(rxPin);
  mySwitch.enableTransmit(txPin);
  char str[] = "A-Sniff,B-Nice,C-Came: Vertical";
  strcpy(RemoteXY.text_01, str);
  RemoteXY.slider_01 = 60;
  SerialBT.begin(device_name); //Bluetooth device name
  SerialBT.setTimeout(10);
}

void loop ()
{
  RemoteXY_Handler ();
  if (RemoteXY.sendcodes!=0) {
    RemoteXY.btglush=0;
    send();
  }
  switch (RemoteXY.select_2) {
    case 0:
      priem();
      break;
    case 1:
      RemoteXY.btglush=0;
      nice();
      RemoteXY.select_2 = 0;
      break;
    case 2:
      RemoteXY.btglush=0;
      came();
      RemoteXY.select_2 = 0;
      break;
    case 3:
      break;
  }
  jummers();
  checkslider();
  btcheck();
}

void btcheck()
{
  if (SerialBT.available()){
    RxByte = SerialBT.read();
    if (RxByte != '\n'){
      RxBuffer += String(RxByte);
    }
    else{
      RxBuffer = "";
    }
    Serial.write(RxByte);  
  }
  if (RxBuffer == "+"){
    freq=freq+1000;
    SerialBT.print("Jum+: ");
    SerialBT.println( freq);
  }
  else if (RxBuffer == "-"){
    freq=freq-1000;
    SerialBT.print("Jum-: ");
    SerialBT.println( freq);
  }
  else if (RxBuffer == "stop"){
    RemoteXY.select_2=3;
    RemoteXY.btglush=0;
  }
  else if (RxBuffer == "nice"){
    RemoteXY.select_2=1;
  }
  else if (RxBuffer == "came"){
    RemoteXY.select_2=2;
  }
  else if (RxBuffer == "snif"){
    RemoteXY.select_2=0;
  }
  else if (RxBuffer == "jON"){
    RemoteXY.btglush=1;
  }
  else if (RxBuffer == "jOFF"){
    RemoteXY.btglush=0;
  }
  else if (RxBuffer == "send"){
    RemoteXY.sendcodes=1;
    RemoteXY.sendcodes=0;
  }
  else if (RxBuffer == "help"){
    SerialBT.println("Menu: + - jON jOFF send came nice snif stop");
  }
}

void jummers()
{
  if(RemoteXY.btglush!=0) {
    if (svstate == false){
      sentsv();
      svstate = true;
      char str[] = "JM:ON";
      sprintf(RemoteXY.text_01, "%s is %d Hz", str, freq);
      SerialBT.print("JM: ");
      SerialBT.println( freq);
    }
    tone(txPin, freq);
    jumstate = true;
  }
  else {
    if (jumstate == true){
      noTone(txPin);
      jumstate = false;
      svstate = false;
      char str[] = "JM:OFF";
      strcpy(RemoteXY.text_01, str);
    }
  }
}

void checkslider()
{
  int pos = RemoteXY.slider_01;
  if (pos == 0) {
    speeds = 1;
  }
  else if (pos <= 30) {
    speeds = 2;
  }
  else if (pos <= 60) {
    speeds = 4;
  }
  else if (pos <= 99) {
    speeds = 6;
  }
  else if (pos == 100) {
    speeds = 10;
    RemoteXY_delay(3500);
  }
}

void sentsv() // Saves codes
{
  char str[] = "Send Saves Codes";
  strcpy(RemoteXY.text_01, str);
  mySwitch.setProtocol(11);
  mySwitch.setPulseLength(320);
  mySwitch.send(1796, 12);
  mySwitch.send(1796, 12);
  mySwitch.send(1796, 12);
  mySwitch.send(1796, 12);//Belka
  RemoteXY_delay(1);
  mySwitch.send(2300, 12);
  mySwitch.send(2300, 12);
  mySwitch.send(2300, 12);
  mySwitch.send(2300, 12);//Belka
}

void send()
{
  mySwitch.setProtocol(protocol);
  mySwitch.setPulseLength(pulselength);
  mySwitch.send(value, bitlength);
  mySwitch.send(value, bitlength);
  RemoteXY_delay(1);
  char str[] = "Send";
  sprintf(RemoteXY.text_01, "%s value %d bit: %d protocol: %d pulse: %d", str, value,bitlength,protocol,pulselength);
  SerialBT.print("Send ");
  SerialBT.print(value);
  SerialBT.print(" Bit: ");
  SerialBT.print(bitlength);
  SerialBT.print(" Prot: ");
  SerialBT.print(protocol);
  SerialBT.print(" Pulse: ");
  SerialBT.println(pulselength);
}

void priem()
{
  if (mySwitch.available()) {
    value = mySwitch.getReceivedValue();
    bitlength = mySwitch.getReceivedBitlength();
    protocol = mySwitch.getReceivedProtocol();
    pulselength = mySwitch.getReceivedDelay();
    output(mySwitch.getReceivedValue(), mySwitch.getReceivedBitlength(), mySwitch.getReceivedDelay(), mySwitch.getReceivedRawdata(),mySwitch.getReceivedProtocol());
    char str[] = "Rx";
    sprintf(RemoteXY.text_01, "%s value %d bit: %d protocol: %d pulse: %d", str, value,bitlength,protocol,pulselength);
    mySwitch.resetAvailable();
  }
}

void nice()
{
for (int send_code = 0; send_code < 4096; send_code++) // цикли перебора всех кодов для 12 бит
  {
    for (int j = 0; j < speeds; j++) // в пультах используется 4 повторения
    {
      digitalWrite(txPin, HIGH); // стартовый импульс
      delayMicroseconds(700);
      digitalWrite(txPin, LOW);
      for (int i = 12; i > 0; i--)
      {
        boolean bit_code = bitRead(send_code, i - 1);
        if (bit_code)
        {
          digitalWrite(txPin, LOW); // единица
          delayMicroseconds(1400);
          digitalWrite(txPin, HIGH);
          delayMicroseconds(700);
        }
        else
        {
          digitalWrite(txPin, LOW); // ноль
          delayMicroseconds(700);
          digitalWrite(txPin, HIGH);
          delayMicroseconds(1400);
        }
      }
      digitalWrite(txPin, LOW); // пилотный период
      delayMicroseconds(25200);
    }
    RemoteXY_Handler();
    char str[] = "Nice";
    sprintf(RemoteXY.text_01, "%s is %d", str, send_code);
    if (RemoteXY.select_2 != 1) break;
    SerialBT.println(send_code);
    checkslider();
    btcheck();
  }
}

void came()
{
  for (int send_code = 0; send_code < 4096; send_code++) // цикли перебора всех кодов для 12 бит
  {
    for (int j = 0; j < speeds; j++) // в пультах используется 4 повторения
    {
      digitalWrite(txPin, HIGH); // стартовый импульс
      delayMicroseconds(320);
      digitalWrite(txPin,LOW);
      for (int i = 12; i > 0; i--)
      {
        boolean bit_code = bitRead(send_code, i - 1);
        if (bit_code)
        {
          digitalWrite(txPin, LOW); // единица
          delayMicroseconds(640);
          digitalWrite(txPin, HIGH);
          delayMicroseconds(320);
        }
        else
        {
          digitalWrite(txPin, LOW); // ноль
          delayMicroseconds(320);
          digitalWrite(txPin, HIGH);
          delayMicroseconds(640);
        }
      }
      digitalWrite(txPin, LOW); // пилотный период
      delayMicroseconds(11520);
    }
    RemoteXY_Handler();
    char str[] = "Came";
    sprintf(RemoteXY.text_01, "%s is %d", str, send_code);
    if (RemoteXY.select_2 != 2) break;
    SerialBT.println(send_code);
    checkslider();
    btcheck();
  }
}