//part 2 finita
#include <Process.h>
#include <Bridge.h>
#include <ArduinoJson.h>
#include <math.h>

const int timeDelay = 5000;
const int INT_LED_PIN = 13;

//parametri per temperatura
const int TEMP_PIN = A0;
const int B = 4275;
const long int R0 = 100000;
const float T0 = 298.15;

const int capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + 40;
DynamicJsonDocument doc_snd(capacity);

//da modificare ogni volta che cambia l'ip del computer
const String url = String("192.168.43.40:8080/log");

Process p;

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Lab 3.2 starting");
  
  pinMode(INT_LED_PIN, OUTPUT);
  digitalWrite(INT_LED_PIN, LOW);
  Bridge.begin();
  digitalWrite(INT_LED_PIN,HIGH);
  pinMode(TEMP_PIN,INPUT);

}

void loop() {
  float T = temperature(analogRead(TEMP_PIN));
  sendTemperature(T);
  delay(timeDelay);
}

float temperature(int a){
  float R = (1023.0/a)-1.0;
  R = R0*R;

  float T = 1.0/(log(R/R0)/B+1/T0)-273.15;
  return T;
}

void sendTemperature(float val){
  String strJson = senMlEncode(F("temperature"), val, F("Cel"));
  int exitVal = postRequest(strJson);
  switch(exitVal)
  {
    case 0:
      Serial.println("curl success");
      break;
    case 1:
      Serial.println("Unsupported protocol");
      break;
    case 2:
      Serial.println("Failed to initialize");
      break;
    case 6:
      Serial.println("Couldn't resolve host");
      break;
    case 7:
      Serial.println("Failed to connect to host");
      break;
    default:
      Serial.println("Unknown error");
      break;
  }
}


int postRequest(String data)
{
  p.begin("curl");
  p.addParameter("-H");
  p.addParameter("Content-Type: application/json");
  p.addParameter("-X");
  p.addParameter("POST");
  p.addParameter("-d");
  p.addParameter(data);
  p.addParameter(url);
  p.run();
  return p.exitValue();
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
