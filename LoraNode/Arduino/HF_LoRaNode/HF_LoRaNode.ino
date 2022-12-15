
#include <SPI.h>              // include libraries
#include <LoRa.h>

const long frequency = 433175000;  // LoRa Frequency

const int csPin = 10;          // LoRa radio chip select
const int resetPin = 9;        // LoRa radio reset
const int irqPin = 2;          // change for your board; must be a hardware interrupt pin

const char HF_KEY[] = "oga9zB_N3A77lgzY3BuT{8BDDC82b";//"pQm9rRTK3-_AlQz52xaT{9ADeD139";
byte _isSending = 0;

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
  //printf();
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



void LoRa_sendPacket(char* msg, int len){
  if (_isSending) return;
  _isSending = 1;
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.write((const uint8_t *)msg,(size_t)len);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}

void LoopSend(unsigned long interval){
  static unsigned long _count = 0;
  static unsigned long _tStart = 0;
  if (!runEvery(interval,_tStart)) return;
  
  ++_count;
  unsigned long t = millis();
  String sensor = StringFormat("{\"ph\":5.7}");
  String message = StringFormat("{\"k\":\"%s\",\"s\":%s}",HF_KEY,sensor.c_str());
  LoRa_sendMessage(message);
}

/////////////////////////////////////////////
void setup() {
 
  Serial.begin(9600);                   // initialize serial UNO GPS

  LoRa.setPins(csPin, resetPin, irqPin);

  if (!LoRa.begin(frequency)) {
    while (true);                       // if failed, do nothing
  }

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
}

void loop() {
  LoopSend(3000);
}
