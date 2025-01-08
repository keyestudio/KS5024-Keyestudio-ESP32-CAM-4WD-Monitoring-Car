/*
  Project: print WiFi IP
  Author: Keyestudio
  Function: how to connect ESP32 to WiFi and output ESP32 ip address
*/
//Import wifi library
#include <WiFi.h>

//Set “your_SSID” to your wifi name
const char* ssid = "your_SSID";
//Set “your_PASSWORD” to your wifi passwords
const char* password = "your_PASSWORD";

void setup() {
  Serial.begin(9600);
  //Initialize wifi
  WiFi.begin(ssid, password);
  //Scan wifi; if connection is not complete, it is always in the state of connecting; while loop
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  //The connection is successful. Print the IP address
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());
}

void loop() {
}