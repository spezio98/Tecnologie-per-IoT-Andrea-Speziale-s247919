#include <TimerOne.h>
#include <math.h>
const int TEMP_PIN = A0;
const int B = 4275;
const long int R0 = 100000;
const float T0 = 298.15;

void setup() {
  Serial.begin(9600);
  while(!Serial); //aspetta finchè la connessione con la porta seriale non è stabilita
  Serial.println("Lab 1.5 starting");
  pinMode(TEMP_PIN,INPUT);
}

void loop() {
  int a = analogRead(TEMP_PIN);
  
  float R = (1023.0/a)-1.0;
  R = R0*R;

  float T = 1.0/(log(R/R0)/B+1/T0)-273.15;
  Serial.print("temperature = ");
  Serial.println(T);

  delay(10000);

}
