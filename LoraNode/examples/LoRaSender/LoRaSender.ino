#include <SPI.h>
#include <LoRa.h>

int counter = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Sender");

  if (!LoRa.begin(433175000)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  char msgBuf[64];
  counter++;
  int len = sprintf(msgBuf,"Sender with GPS %d",counter);
  Serial.print(msgBuf);
  Serial.print("\n");
  // send packet
  LoRa.beginPacket();
  LoRa.write((uint8_t*)msgBuf,(size_t)len);  
  LoRa.endPacket();

  delay(1000);
}
