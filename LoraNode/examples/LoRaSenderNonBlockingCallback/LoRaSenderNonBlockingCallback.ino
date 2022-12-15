#include <SPI.h>
#include <LoRa.h>

int counter = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Sender non-blocking Callback");

  if (!LoRa.begin(433175000)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  LoRa.onTxDone(onTxDone);
}

void loop() {
  counter++;
  char msgBuf[64];
  int len = sprintf(msgBuf,"Node GPS Sending %d",counter);
  Serial.println(msgBuf);

  delay(1000);
  
  /*if (!IsNextTimer(5000)) return;
  counter++;
  char msgBuf[64];
  int len = sprintf(msgBuf,"Node GPS Sending %d",counter);
  Serial.print(msgBuf);
  Serial.println("\n");

  // send in async / non-blocking mode
  LoRa.beginPacket();
  LoRa.write((uint8_t*)msgBuf,(size_t)len);    
  LoRa.endPacket(true); // true = async / non-blocking mode
  */
  
}

void onTxDone() {
  Serial.println("TxDone");
}

boolean IsNextTimer(unsigned long interval)
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
