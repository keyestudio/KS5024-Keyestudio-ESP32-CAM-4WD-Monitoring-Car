/*
  Project: Breathing LED
  Author: Keyestudio
  Function: how to conttrol the brightness of LED
*/
int ledPin = 4;  //set variable ledPin to pin GPIO4
void setup() {
  // put your setup code here, to run once:
  pinMode(ledPin, OUTPUT);  //set pin GPIO4 to output
}

void loop() {
  // put your main code here, to run repeatedly:
  for (int i = 0; i <= 255; i++) {  //Loop increases the value of i from 0 to 255
    analogWrite(ledPin, i); //GPIO4 outputs as PWM and sets the PWM value to i
    delay(10);  //Delay 10ms
  }
  for (int i = 255; i >= 0; i--) {  //Loop reduces the value of i from 255 to 0
    analogWrite(ledPin, i); //GPIO4 outputs as PWM and sets the PWM value to i
    delay(10);
  }
}
