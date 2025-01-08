/*
  Project: motor drive
  Author: Keyestudio
  Function: how to control car move forward and backward, turn left and right
*/

//set motor control pins
#define MOTOR_R_PIN_1 14  
#define MOTOR_R_PIN_2 15
#define MOTOR_L_PIN_1 13
#define MOTOR_L_PIN_2 12

void setup() {
  //set serial baud rate
  Serial.begin(115200);
  //set pin modes
  pinMode(MOTOR_R_PIN_1, OUTPUT);
  pinMode(MOTOR_R_PIN_2, OUTPUT);
  pinMode(MOTOR_L_PIN_1, OUTPUT);
  pinMode(MOTOR_L_PIN_2, OUTPUT);
}

void loop() {
   //Forward
  Serial.println("Forward");
  digitalWrite(MOTOR_R_PIN_1, LOW);
  digitalWrite(MOTOR_R_PIN_2, HIGH);
  digitalWrite(MOTOR_L_PIN_1, HIGH);
  digitalWrite(MOTOR_L_PIN_2, LOW);
  delay(2000);
  // Backward
  Serial.println("Backward");
  digitalWrite(MOTOR_R_PIN_1, HIGH);
  digitalWrite(MOTOR_R_PIN_2, LOW);
  digitalWrite(MOTOR_L_PIN_1, LOW);
  digitalWrite(MOTOR_L_PIN_2, HIGH);
  delay(2000);
  // Left
  Serial.println("Left");
  digitalWrite(MOTOR_R_PIN_1, LOW);
  digitalWrite(MOTOR_R_PIN_2, HIGH);
  digitalWrite(MOTOR_L_PIN_1, LOW);
  digitalWrite(MOTOR_L_PIN_2, HIGH);
  delay(2000);
  // Right
  Serial.println("Right");
  digitalWrite(MOTOR_R_PIN_1, HIGH);
  digitalWrite(MOTOR_R_PIN_2, LOW);
  digitalWrite(MOTOR_L_PIN_1, HIGH);
  digitalWrite(MOTOR_L_PIN_2, LOW);
  delay(2000);
  // Stop
  Serial.println("Stop");
  digitalWrite(MOTOR_R_PIN_1, LOW);
  digitalWrite(MOTOR_R_PIN_2, LOW);
  digitalWrite(MOTOR_L_PIN_1, LOW);
  digitalWrite(MOTOR_L_PIN_2, LOW);
  delay(2000);
}  