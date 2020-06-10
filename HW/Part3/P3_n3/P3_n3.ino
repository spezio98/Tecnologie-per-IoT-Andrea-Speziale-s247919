#include <MQTTclient.h>
#include <Process.h>
#include <Bridge.h>
#include <ArduinoJson.h>
#include <math.h>

const int timeDelay = 1000;
const int INT_LED_PIN = 13;
const int LED_PIN = 11;

//parametri per temperatura
const int TEMP_PIN = A0;
const int B = 4275;
const long int R0 = 100000;
const float T0 = 298.15;

const int capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + 40;
DynamicJsonDocument doc_snd(capacity);
DynamicJsonDocument doc_rec(capacity);

const String my_base_topic = String("tiot/20");

unsigned long startMillis = 0;
unsigned long timeSendTemperature = 10;
unsigned long currentMillis = 0;
int firstTime = 1;

Process p;

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Lab 3.3 starting");
  
  pinMode(INT_LED_PIN, OUTPUT);
  pinMode(LED_PIN,OUTPUT);
  pinMode(TEMP_PIN,INPUT);
  digitalWrite(LED_PIN,LOW);
  digitalWrite(INT_LED_PIN, LOW);
  Bridge.begin();
  digitalWrite(INT_LED_PIN,HIGH);

  mqtt.begin("test.mosquitto.org",1883);
  mqtt.subscribe(my_base_topic + String("/led"),setLedValue);
  startMillis = millis();
}

void loop() {
  //controlla se ci sono messaggi in arrivo dai topic a cui è stata fatta la sottoscrizione.
  //se ci sono messaggi allora invoca la callback setLedValue
  mqtt.monitor();

  //sezione publisher
  //calcola temperatura e invia messaggio ogni 10 secondi.
  //ho bisogno del flag firstTime perchè altrimenti la prima temperatura sarà inviata dopo i primi 10 secondi
  currentMillis = millis();
  if(firstTime || currentMillis - startMillis >= timeSendTemperature * 1e03){
    firstTime = 0;
    float T = temperature(analogRead(TEMP_PIN));
    String message = senMlEncode("temperature", T, "Cel");
    mqtt.publish(my_base_topic + String("/temperature"),message);
    startMillis = currentMillis;
  }
  delay(timeDelay);
}

float temperature(int a){
  float R = (1023.0/a)-1.0;
  R = R0*R;

  float T = 1.0/(log(R/R0)/B+1/T0)-273.15;
  return T;
}

//mosquitto_pub -h test.mosquitto.org -t "tiot/20/led" -m "{\"bn\": \"Yun group 20\", \"e\": [{\"n\": \"led\", \"t\": null, \"v\": \"1\", \"u\": null}]}"
//callback invocata quando ci sono dati disponibili in ricezione
void setLedValue(const String& topic, const String& subtopic, const String& message){
  DeserializationError err = deserializeJson(doc_rec, message);
  if(err)
  {
    Serial.print(F("deserializeJson() failed with code "));
    Serial.println(err.c_str());
  }
  if(doc_rec["e"][0]["n"]=="led"){
    int ledValue = doc_rec["e"][0]["v"];
    switch(ledValue){
      case 0:
        digitalWrite(LED_PIN,LOW);
        break;
      case 1:
        digitalWrite(LED_PIN,HIGH);
        break;
      default:
        Serial.println("Wrong led value");
        break;
    }
    
  }
  else
    Serial.println("Json format required is not correct");
  
}

String senMlEncode(String res, float v, String unit){
  doc_snd.clear();
  doc_snd["bn"] = "Yun group 20"; //adesso contiene {"bn":"Yun group 20"}
  long t = millis()/1000;
  
  doc_snd["e"][0]["n"] = res;
  doc_snd["e"][0]["t"] = t;
  doc_snd["e"][0]["v"] = String(v,2);
  doc_snd["e"][0]["u"] = unit;

  String output;
  //crea json dal documento dinamico
  serializeJson(doc_snd, output);
  return output;
}
