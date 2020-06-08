const int LED_PIN = 12;
const int PIR_PIN = 7;
volatile int tot_count = 0;
volatile int pirValue = 0;

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Lab 1.3 starting");

  pinMode(LED_PIN,OUTPUT);
  digitalWrite(LED_PIN,LOW);
  pinMode(PIR_PIN, INPUT);
  
  attachInterrupt(digitalPinToInterrupt(PIR_PIN), checkPresence, CHANGE); //change dato che deve reagire sia sui fronti di salita che discesa

}

void loop() {
  Serial.print("Total people count: ");
  Serial.println(tot_count);
  delay(30*1e03);
}

void checkPresence(){
  pirValue = digitalRead(PIR_PIN);
  if(pirValue == HIGH){
    tot_count++;
  }
  digitalWrite(LED_PIN,pirValue);
}
