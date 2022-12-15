#include <SPI.h>
#include <LoRa.h>

int counter = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Sender non-blocking");

  if (!LoRa.begin(433175000)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

void loop() {
  // wait until the radio is ready to send a packet
  while (LoRa.beginPacket() == 0) {
    //Serial.print("waiting for radio ... ");
    delay(100);
  }
  char msgBuf[64];
  counter++;
  int len = sprintf(msgBuf,"Sender NonBlocking GPS %d",counter);
  Serial.print(msgBuf);
  Serial.println("\n");

  // send in async / non-blocking mode
  LoRa.beginPacket();
  LoRa.write((uint8_t*)msgBuf,(size_t)len);  
  LoRa.endPacket(true); // true = async / non-blocking mode

  
}
