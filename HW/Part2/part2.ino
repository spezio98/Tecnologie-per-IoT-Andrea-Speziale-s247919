#include <TimerOne.h>
#include <LiquidCrystal_PCF8574.h>
#include <math.h>
const int TEMP_PIN = A0;
const int B = 4275;
const long int R0 = 100000;
const float T0 = 298.15;
float T = 0;
float current_speed = 0;
const int FAN_PIN = 13;
const int RLED_PIN = 11;
float RLedValue = 0;
float setPointsIN[] = {25.0,35.0,12.0,22.0};      //set point da usare in caso di presenza in casa
float setPointsOUT[] = {28.0, 38.0, 15.0, 20.0};  //set point da usare se non sono state rilevate presenze in casa
float *setPointsUsed;                             //vettore che assumerà uno dei due valori precedenti

volatile int pirValue = 0;
const int PIR_PIN = 4;
int presence = 0;           //presenza effettiva
int presenceP = 0;          //presenza rilevata dal PIR
int presenceR = 0;          //presenza rilevata dal sensore di rumore
const unsigned long timeout_pir = 10;
unsigned long startMillis = 0;
//mi servirà a verificare lo scadere del tempo timeout_pir per confermare l'assenza di persone 
unsigned long currentMillis = 0;
//indicherà il tempo in cui è stata verificato l'ultima presenza dal sensore PIR        
unsigned long lastPresence = 0;

const int SOUND_PIN = 7;
int soundDetected = HIGH; //valore iniziale alto, essendo il sensore un attivo basso

int nSound = 0;
int nSoundsNecessary = 10;
int n_sound_events = 3;
int nEvents = 0;

//time_event è il tempo di acquisizione di un unico suono. Se dopo questo tempo il numero 
//di rumori rilevati non è maggiore di nSoundsNecessary allora non può essere considerato affidabile.
//più suoni costituiscono un evento e la presenza è verificata da più eventi in un tempo sound_interval 
unsigned long time_event = 5;       
//sound_interval è il tempo in cui devono essere velevati n_sounds_events eventi differenti affinche 
//venga riconosciuta la presenza
unsigned long sound_interval = 30;
//istante in cui è stato rilevato il primo evento (non ancora confermato in una presenza)
unsigned long firstEvent = 0;
//instante in cui è stato rilevato il primo suono (non ancora confermato in un evento)
unsigned long firstSound = 0;

unsigned long currentMillisSound = 0;

//tempo dopo il quale, senza presenze rilevate, la casa è libera.
unsigned long timeout_sound = 60;
//istante in cui si è rilevata una vera presenza per il rumore
unsigned long time_presenceR_detected = 0;

//istante in cui cambia il display. Necessario a contare i timeScreen secondi per cambiare schermata
unsigned long displayMillis = 0;
int numDisplay = 1;
int timeScreen = 5;
int i = 1;

String strInput;
const int SIZESTR = 8;

LiquidCrystal_PCF8574 lcd(0x27);

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Lab 2 starting");

  pinMode(FAN_PIN,OUTPUT);  
  analogWrite(FAN_PIN, (int) current_speed);
  pinMode(TEMP_PIN,INPUT);
  pinMode(RLED_PIN,OUTPUT);
  analogWrite(RLED_PIN,LOW);
  pinMode(PIR_PIN, INPUT);
  pinMode(SOUND_PIN, INPUT);

  setPointsUsed = setPointsOUT;     //inizialmente userò i set point come se non ci fossero presenze in casa

  lcd.begin(16,2);
  lcd.setBacklight(255);
  lcd.home();
  lcd.clear();

  //timer che scadrà ogni mezzo secondo scatenando un interrupt
  //l'interrupt eseguirà l'ISR che verificherà la presenza di persone in casa mediante il sensore PIR
  //e gestirà anche le varie rilevazioni del sensore di rumore per renderle affidabili.
  Timer1.initialize(0.5 * 1e06);   
  Timer1.attachInterrupt(checkPresence);

  //interrupt hardware scatenato quando il sensore di rumore rileva suoni. Nota: Falling perchè il sensore è un attivo basso.
  attachInterrupt(digitalPinToInterrupt(SOUND_PIN), checkNoise, FALLING);

  Serial.println("Per modificare i set-point digitali singolarmente e senza spazi ACm=10 e ACM=10");  
  
  numDisplay = 1;   //seleziona il display iniziale
  
  startMillis = millis();
}

void loop() {
  
  //calcolo temperatura
  int a = analogRead(TEMP_PIN);
  T = temp(a);
  current_speed = changeFanFreq(T);
  RLedValue = changeRLedBright(T);

  //acquisizione della scringa per modificare i set point
  strInput = Serial.readString();
  if(strInput.length()!=0)
    if(acquisisci(strInput) == 1)
      {
        Serial.print(strInput);
        Serial.println(" inserito.");
      }
    
  /*
  Serial.print("Current temperature: ");
  Serial.println(T);
  Serial.print("Current speed: ");
  Serial.println(current_speed);
  Serial.print("Current brightness RLED: ");
  Serial.println(RLedValue);
  Serial.print("Number sound detected: ");
  Serial.println(nSound);
  Serial.print("Number events detected: ");
  Serial.println(nEvents);
  Serial.print("PresenceP : ");
  Serial.print(presenceP);
  Serial.print(" PresenceR : ");
  Serial.print(presenceR);
  Serial.print(" Presence : ");
  Serial.println(presence);
*/
  setDisplay();
 
  //delay(2500);
}

void setDisplayPage1(float T, int pres, float AC, float HT)
{
  float ACperc =(float) AC / 255 * 100;
  float HTperc = (float) HT / 255 * 100;
  String str1 = "T:" + String(T) + " Pres:" + String(pres);
  String str2 = "AC:" + String((int)ACperc) + "% HT:" + String((int)HTperc) + "%";
  
  lcd.home();
  lcd.clear();

  lcd.print(str1);
  lcd.setCursor(0,1);
  lcd.print(str2);
}

void setDisplayPage2(float ACm, float ACM, float HTm, float HTM)
{
  String str1 = "AC m:" + String(ACm,1) + " M:" + String(ACM,1);
  String str2 = "HT m:" + String(HTm,1) + " M:" + String(HTM,1);
  
  lcd.home();
  lcd.clear();

  lcd.print(str1);
  lcd.setCursor(0,1);
  lcd.print(str2);
  
}

void setDisplay()
{
  displayMillis = millis();
  if(displayMillis - startMillis >= timeScreen * 1e03)
  { 
   startMillis = millis();
   if(numDisplay == 1)
      numDisplay = 2;
   else
      numDisplay = 1;
  }
  
  
  switch(numDisplay)
  {
    case 1:
      setDisplayPage1(T, presence, current_speed, RLedValue);
      break;
     case 2:
      setDisplayPage2(setPointsUsed[0], setPointsUsed[1], setPointsUsed[2], setPointsUsed[3]);
      break;
     default:
      lcd.home();
      lcd.clear();
      lcd.print("Errore pagina");
      break;   
  }
}

float temp(int a)
{
  float R = (1023.0/a)-1.0;
  R = R0*R;
  float T = 1.0/(log(R/R0)/B+1/T0)-273.15;
  return T;
}

float changeFanFreq(float T)
{
  float minTFan = setPointsUsed[0];
  float maxTFan = setPointsUsed[1];
  float speed = 0;
  
  if(T>=maxTFan)
  {
    speed = 255;
    analogWrite(FAN_PIN, speed);
    
  }
  else if(T<=minTFan)
  {
    speed = 0;
    analogWrite(FAN_PIN, speed);
  }
  else
  {
    //applico funzione lineare per calcolare la velocità 
    speed = (float) (255*(T-minTFan)/(maxTFan - minTFan));
    analogWrite(FAN_PIN, (int) speed);
  }
  return speed;
}

float changeRLedBright(float T)
{
  float value = 0;
  float minTHeat = setPointsUsed[2];
  float maxTHeat = setPointsUsed[3];
  
  if(T>=maxTHeat)
  {
    value = 0;
    analogWrite(RLED_PIN, value);
  }
  else if(T<=minTHeat)
  {
    value = 255;
    analogWrite(RLED_PIN, value);
  }
  else
  {
    //applico funzione lineare per calcolare il valore di luminosità del led
    value = (float) (-255*(T - minTHeat)/(maxTHeat - minTHeat)+255); 
    analogWrite(RLED_PIN, (int)value);
  }
  return value;
}

void checkPresence(){
  currentMillis = millis();
  pirValue = digitalRead(PIR_PIN);

  //controllo presenza per la prima volta
  if(lastPresence == 0){
    if(pirValue == HIGH){
      lastPresence = millis();
      presenceP = 1;
      Serial.println("\tPIR:  Presence at this moment");
    }
  }
  else
  {
    //nei primi 5 secondi il sensore resta alto, quindi se non sono ancora passati non controllo
    //in realtà sono 3 secondi circa, ma per sicurezza metto 5.
    //Se invece sono passati i 5 secondi e c'è una nuova rilevazione, aggiorna lastPresence
    if( currentMillis - lastPresence >= 5*1e03 && pirValue == HIGH){
      presenceP = 1;
      Serial.println("\tPIR:  Presence again");
      //aggiorno lastPresence
      lastPresence = millis();
    }
  }

  //Se l'ultima presenza è stata rilevata in tempo precedente superiore a timeout_pir, 
  //allora posso dire al momento che non c'è più nessuno in casa
  if(lastPresence != 0 && currentMillis - lastPresence >= timeout_pir * 1e03 )
  {
      presenceP = 0;
      lastPresence = 0;
      Serial.println("\tPIR:  No presence at this moment");  
  }

  //controllo movimento sensore Rumore
  
  if(firstSound != 0 && currentMillis - firstSound >= time_event * 1e03)
  {
    if(nSound >= nSoundsNecessary)
    {
      //Evento verificato
      nEvents++;
      Serial.println("\tTrovato evento di rumore");
    }else
    {
      //falso suono
      Serial.println("\nFalso evento");
    }
    
    nSound = 0;
    firstSound = 0;
  }

  if(firstEvent != 0 && currentMillis - firstEvent >= sound_interval * 1e03)
  {
    if(nEvents >= n_sound_events)
    {
      Serial.println("\tTrovati tutti gli eventi di rumore necessari. Presenza rilevata");
      time_presenceR_detected= millis();
      presenceR = 1;
      
    }
    nSound = 0;
      nEvents = 0;
      firstEvent = 0;
      firstSound = 0;
  }

  if(currentMillis - time_presenceR_detected >= timeout_sound *1e03 && presenceR == 1)
  {
      Serial.println("No presenze per rumori");
      presenceR = 0;
      nSound = 0;
      firstSound = 0;
  }

  if(presenceR || presenceP)
  {
    presence = 1;
    setPointsUsed = setPointsIN;
  }
  else
  {
    presence = 0;
    setPointsUsed = setPointsOUT;
  }
}
  
void checkNoise()
{
  currentMillisSound = millis();
  if(firstSound == 0)
    firstSound = currentMillisSound;

  if(firstEvent == 0)
    firstEvent = currentMillisSound;
  
  if(currentMillisSound - firstSound < time_event * 1e03)
     nSound++;
}

int acquisisci(String strInput)
{
    char *str =(char*) malloc(SIZESTR * sizeof(char));
    strInput.toCharArray(str, SIZESTR);
    char *nameSetPoint = strtok(str,"=");
    char *value = strtok(NULL,"=");
    float valueF = atof(value);
    
    if(strcmp(nameSetPoint,"ACm")==0){
      if(valueF>setPointsUsed[1])
        Serial.println("Errore. Valore più grande di ACM");
      else
      {
        setPointsUsed[0] = atof(value);
        return 1;
      }
    }
    else if(strcmp(nameSetPoint, "ACM")==0)
      {
        if(valueF<setPointsUsed[0])
          Serial.println("Errore. Valore più piccolo di ACm");
        else{
          setPointsUsed[1] = atof(value);
          return 1;
        }
      }
      else if(strcmp(nameSetPoint, "HTm")==0)
        {
          if(valueF>setPointsUsed[3])
            Serial.println("Errore. Valore più grande di HTM");
          else{
            setPointsUsed[2] = atof(value);
            return 1;
          }
        }
        else if(strcmp(nameSetPoint, "HTM")==0)
          {
            if(valueF<setPointsUsed[2])
              Serial.println("Errore. Valore più piccolo di HTm");
            else{
              setPointsUsed[3] = atof(value);
              return 1;
            }
          }
          else
            Serial.println("valore non riconosciuto");
  return 0;
}
