#include <TimerOne.h>

const int RLED_PIN = 12;
const int GLED_PIN = 11;

const float R_HALF_PERIOD = 1.5;
const float G_HALF_PERIOD = 3.5;

//Stato iniziale dei led
int greenLedState = LOW;
int redLedState = LOW;

void setup() {
  // put your setup code here, to run once:
  pinMode(RLED_PIN, OUTPUT);
  pinMode(GLED_PIN, OUTPUT);
  Timer1.initialize(G_HALF_PERIOD * 1e06);
  Timer1.attachInterrupt(blinkGreen); //interrupt service routine attivata quando il timer va a 0
}

void loop() {
  // put your main code here, to run repeatedly:
  redLedState = !redLedState;
  digitalWrite(RLED_PIN, redLedState);
  delay(R_HALF_PERIOD * 1e03);
}

void blinkGreen(){
  greenLedState = !greenLedState;
  digitalWrite(GLED_PIN, greenLedState);
}
