#include <TimerOne.h>

const int RLED_PIN = 12;
const int GLED_PIN = 11;

const float R_HALF_PERIOD = 1.5;
const float G_HALF_PERIOD = 3.5;

//Stato iniziale dei led
//dichiarata volatile perchè viene modificata in modo inaspettato dall'interrupt e in questo modo
//viene salvata nella RAM piuttosto che nel disco fisso.
volatile int greenLedState = LOW;
//anche quella utilizzata dentro la funzione loop è dichiarata volatile perchè modificata molte molte
//e, essendo memorizzata nella RAM, la modifica è più veloce.
volatile int redLedState = LOW;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  while(!Serial); //aspetta finchè la connessione con la porta seriale non è stabilita
  Serial.println("Lab 1.2 starting");

  pinMode(RLED_PIN, OUTPUT);
  pinMode(GLED_PIN, OUTPUT);
  Timer1.initialize(G_HALF_PERIOD * 1e06);
  Timer1.attachInterrupt(blinkGreen); //interrupt service routine associata al timer e attivata quando il timer va a 0
}

void loop() {
  redLedState = !redLedState;
  digitalWrite(RLED_PIN, redLedState);
  serialPrintStatus();
  delay(R_HALF_PERIOD * 1e03);
}

void serialPrintStatus() {
  if(Serial.available() > 0) {
    int inByte = Serial.read();
    char R = 'R';
    char G = 'G';

    if(inByte == (int)G){
      
      Serial.print("Led 2 Status: ");
      Serial.println(greenLedState);
    }
    else{
      if(inByte == (int)R){
        Serial.print("Led 1 Status: ");
        Serial.println(redLedState);
      }
      else{
        Serial.println("Errore comando");
      }
    }
  }
}

void blinkGreen(){
  greenLedState = !greenLedState;
  digitalWrite(GLED_PIN, greenLedState);
}
