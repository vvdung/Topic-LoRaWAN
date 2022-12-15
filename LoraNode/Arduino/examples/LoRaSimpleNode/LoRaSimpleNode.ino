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

const int BLINK_PIN = 13;

const int R_PIN = 13;       //select the pin for the red LED
const int G_PIN = 12; // select the pin for the  green LED
const int B_PIN = 11;// select the pin for the  blue LED

bool _ONOFF = 0;
unsigned long _rgbStart = 0;


void setup() {
  //pinMode(R_PIN, OUTPUT);
  //pinMode(G_PIN, OUTPUT);
  //pinMode(B_PIN, OUTPUT);
  pinMode(BLINK_PIN, OUTPUT);
  Serial.begin(9600);                   // initialize serial UNO GPS

  LoRa.setPins(csPin, resetPin, irqPin);

  if (!LoRa.begin(frequency)) {
    //Serial.println("LoRa init failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  //Serial.println("LoRa init succeeded.");
  //Serial.println();
  //Serial.println("LoRa Simple Node");
  //Serial.println("Only receive messages from gateways");
  //Serial.println("Tx: invertIQ disable");
  //Serial.println("Rx: invertIQ enable");
  //Serial.println();

  LoRa.onReceive(onReceive);
  LoRa.onTxDone(onTxDone);
  LoRa_rxMode();
}

void LoRa_sendPacket(char* msg, int len){
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.write((const uint8_t *)msg,(size_t)len);                  // add payload
  LoRa.endPacket(true);                 // finish packet and send it
}


void loop() {

  if (!runEvery(2000)) return;
  static uint32_t i = 0;
  ++i;
  String message = "LEONADO 1. ";
  message += "I'm a Node ";
  message +=i;

  LoRa_sendMessage(message); // send a message

  rgbLED(false);
  //Serial.println(message);
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
  Serial.print("Node Receive: ");
  Serial.println(message);
}

void onTxDone() {
  //Serial.println("TxDone");
  LoRa_rxMode();
}

boolean runEvery(unsigned long interval)
{
  static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
    return true;
  }
  return false;
}
boolean nextTimer(unsigned long interval, unsigned long& uStart)
{
  unsigned long currentMillis = millis();
  if (currentMillis - uStart >= interval)
  {
    uStart = currentMillis;
    return true;
  }
  return false;
}
void rgbLED(bool on){
  if (on){
    if (_ONOFF) return;
    digitalWrite(BLINK_PIN,HIGH);
    //analogWrite(R_PIN, 255);
    //analogWrite(G_PIN, 0);
    //analogWrite(B_PIN, 0);    
    _ONOFF = true;
    return;
  }
  if (!_ONOFF) return;
  digitalWrite(BLINK_PIN,LOW);
  //analogWrite(R_PIN, 0);
  //analogWrite(G_PIN, 0);
  //analogWrite(B_PIN, 0);    
  _ONOFF = false;
}
