/*
  LoRa Simple Gateway/Node Exemple

  This code uses InvertIQ function to create a simple Gateway/Node logic.

  Gateway - Sends messages with enableInvertIQ()
          - Receives messages with disableInvertIQ()

  Node    - Sends messages with disableInvertIQ()
          - Receives messages with enableInvertIQ()

  With this arrangement a Gateway never receive messages from another Gateway
  and a Node never receive message from another Node.
  Only Gateway to Node and vice versa.

  This code receives messages and sends a message every second.

  InvertIQ function basically invert the LoRa I and Q signals.

  See the Semtech datasheet, http://www.semtech.com/images/datasheet/sx1276.pdf
  for more on InvertIQ register 0x33.

  created 05 August 2018
  by Luiz H. Cassettari
*/

#include <SPI.h>              // include libraries
#include <LoRa.h>

const long frequency = 433175000;  // LoRa Frequency

const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;        // LoRa radio reset
const int irqPin = 2;          // change for your board; must be a hardware interrupt pin

const int R_PIN = 7;//13;       //select the pin for the red LED
const int G_PIN = 6;//12; // select the pin for the  green LED
const int B_PIN = 5;//11;// select the pin for the  blue LED

bool _ONOFF = 0;
unsigned long _rgbStart = 0;

byte _isSending;
char _dataGPS[128];
int _chCount;
unsigned long _gpsStart = 0;
unsigned long _msgStart = 0;
const char HF_KEY[] = "pQm9rRTK3-_AlQz52xaT{9ADeD139";//"oga9zB_N3A77lgzY3BuT{8BDDC82b";//

String StringFormat(const char* fmt, ...){
  va_list vaArgs;
  va_start(vaArgs, fmt);
  va_list vaArgsCopy;
  va_copy(vaArgsCopy, vaArgs);
  const int iLen = vsnprintf(NULL, 0, fmt, vaArgsCopy);
  va_end(vaArgsCopy);
  int iSize = iLen + 1;
  char* buff = (char*)malloc(iSize);
  vsnprintf(buff, iSize, fmt, vaArgs);
  va_end(vaArgs);
  String s = buff;
  free(buff);
  return String(s);
}

void setup() {
  _isSending = 0;
  _chCount = 0;
  memset(_dataGPS,0,sizeof(_dataGPS));

  pinMode(R_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  pinMode(B_PIN, OUTPUT);

  analogWrite(R_PIN, 0);
  analogWrite(G_PIN, 0);
  analogWrite(B_PIN, 0); 
  randomSeed(analogRead(0));
  
  Serial.begin(9600);                   // initialize serial UNO GPS

  LoRa.setPins(csPin, resetPin, irqPin);

  if (!LoRa.begin(frequency)) {
    while (true);                       // if failed, do nothing
  }

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
}

void LoRa_sendPacket(char* msg, int len){
  if (_isSending) return;
  _isSending = 1;
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.write((const uint8_t *)msg,(size_t)len);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}

void sendGPS(){

  char msgBuf[256];
  memset(msgBuf,0,256);  
  sprintf(msgBuf,"[GPS]");  

  while (Serial.available()){
    char c = Serial.read(); 
    if (c == '\r' || c == '\n') continue;
    if (c == '$' && _chCount > 0){      
      _dataGPS[_chCount] = 0;
      if (strstr(_dataGPS,"$GPRMC") != NULL){
        strcat(msgBuf, _dataGPS);       
        //memset(_dataGPS,0,sizeof(_dataGPS));   
        if (runEvery(5000,_gpsStart)){
          LoRa_sendPacket(msgBuf,strlen(msgBuf)); 
          _msgStart = _gpsStart;
        }
        sprintf(msgBuf,"[GPS]");
      }
      memset(_dataGPS,0,sizeof(_dataGPS));

      _chCount = 0;      
    }//if (c == '$' && _chCount > 0){
    
     _dataGPS[_chCount++] = c;
     if (_chCount > 128) _chCount = 0;
  }//while (Serial.available()){

  if (!runEvery(5000,_msgStart)) return;
  LoRa_sendPacket(msgBuf,strlen(msgBuf)); 

}

void LoopSend(unsigned long interval){
  static unsigned long _tStart = 0;
  if (!runEvery(interval,_tStart)) return;
  
  String sensor = StringFormat("{\"ph\":4.7,\"water_temp\":14.7}");
  String message = StringFormat("{\"k\":\"%s\",\"s\":%s}",HF_KEY,sensor.c_str());
  LoRa_sendMessage(message);
}

void loop() {
  LoopSend(5000);
  //sendGPS();

  if (!runEvery(2000,_rgbStart)) return;
  rgbLED(false);
}


void LoRa_rxMode(){
  LoRa.enableInvertIQ();                // active invert I and Q signals
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode(){
  LoRa.idle();                          // set standby mode
  LoRa.disableInvertIQ();               // normal mode
}

void LoRa_sendMessage(String message) {
  if (_isSending) return;
  _isSending = 1;
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.print(message);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}

void onReceive(int packetSize) {
  String message = "";

  while (LoRa.available()) {
    message += (char)LoRa.read();
  }
  rgbLED(true);
}

void onTxDone() {  
  LoRa_rxMode();
  _isSending = 0;
}

boolean runEvery(unsigned long interval, unsigned long &previousMillis)
{  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}
void rgbLED(bool on){
  if (on){
    if (_ONOFF) return;
    int r = random(255);
    int g = random(255);
    int b = random(255);
    analogWrite(R_PIN, r);
    analogWrite(G_PIN, g);
    analogWrite(B_PIN, b);    
    _ONOFF = true;
    return;
  }
  if (!_ONOFF) return;
  analogWrite(R_PIN, 0);
  analogWrite(G_PIN, 0);
  analogWrite(B_PIN, 0);    
  _ONOFF = false;
}
