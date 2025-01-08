/*
  Project: LED control
  Author: Keyestudio
  Function: how to control LED on/off
*/
int ledPin = 4; //set variable ledPin to pin GPIO4
void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin,OUTPUT); //set pin GPIO4 to output
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(ledPin,HIGH);  //set pin GPIO4 to output high
  delay(500); //delay 500ms
  digitalWrite(ledPin,LOW); //set pin GPIO4 to output low
  delay(500); //delay 500ms
}
