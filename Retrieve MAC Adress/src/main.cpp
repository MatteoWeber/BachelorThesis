// This code retrieves the MAC address of the esp32 
// The MAC address is needed to establish a connection between the different esps 
// For a bidirectional connection the MAC address of the sender and reciever are needed
// The Mac address is output as follows: XX:XX:XX:XX:XX:XX
// This output needs to be formatted and pasted into the sketch of the reciever / transmitter 
// The line for the reciever / transmitter sketch looks like this: uint8_t recieverMAC[] = {0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX, 0xXX}
// Example: Mac Address from this Sketch: 08:d1:f9:cf:a7:74 => uint8_t receiverMAC[] = {0x08, 0xd1, 0xf9, 0xcf, 0xa7, 0x74};

#include <Arduino.h>
#include <WiFi.h>


void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  Serial.println(WiFi.macAddress());
}

void loop() {
  
}
