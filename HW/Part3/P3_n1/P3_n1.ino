#include <Bridge.h>
#include <BridgeServer.h>
#include <BridgeClient.h>
#include <ArduinoJson.h>
#include <TimerOne.h>
#include <math.h>

//parametri per led
const int INT_LED_PIN = 13;
const int LED_PIN = 11;

//parametri per temperatura
const int TEMP_PIN = A0;
const int B = 4275;
const long int R0 = 100000;
const float T0 = 298.15;

const int capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + 40;
DynamicJsonDocument doc_snd(capacity);

BridgeServer server;

void setup() {
  pinMode(INT_LED_PIN, OUTPUT);
  digitalWrite(INT_LED_PIN, LOW);
  Bridge.begin();
  digitalWrite(INT_LED_PIN,HIGH);
  pinMode(TEMP_PIN,INPUT);
  server.listenOnLocalhost();
  server.begin();

}

void loop() {
  BridgeClient client = server.accept();

  if(client){
    process(client);
    client.stop();
  }
  delay(50);
}

void process(BridgeClient client){
  String command = client.readStringUntil('/');
  command.trim();

  if(command == "led"){
    int val = client.parseInt();
    if(val==0 || val==1){
      digitalWrite(LED_PIN,val);
      printResponse(client, 200, senMlEncode(F("led"), val, F("")));
    }
    else{
      printResponse(client, 400, "Error 400. Wrong led value");
    }
  }
  else if(command == "temperature")
  {
    float val = temperature(analogRead(TEMP_PIN));
    printResponse(client, 200, senMlEncode(F("temperature"), val, F("Cel")));
    
  }
  else
  {
    printResponse(client,404,"Error 404. Command not recognized");
  }
}

float temperature(int a){
  float R = (1023.0/a)-1.0;
  R = R0*R;

  float T = 1.0/(log(R/R0)/B+1/T0)-273.15;
  return T;
}

void printResponse(BridgeClient client, int code, String body){
  client.println("Status: " + String(code));
  if(code==200)
  {
    client.println(F("Content-type: application/json; charset=utf-8"));
    client.println();
    client.println(body);
  }
  
  else
  {
    client.println(F("Content-type: text/plain; charset=utf-8"));
    client.println();
    client.println(body);
  }
}

String senMlEncode(String res, float v, String unit){
  doc_snd.clear();
  doc_snd["bn"] = "Yun group 20"; //adesso consiene {"bn":"Yun group 20"}
  long t = millis()/1000;
  if(unit != ""){
    doc_snd["e"][0]["n"] = res;
    doc_snd["e"][0]["t"] = t;
    doc_snd["e"][0]["v"] = String(v,2);
    doc_snd["e"][0]["u"] = unit;
  }
  else
  {
    doc_snd["e"][0]["n"] = res;
    doc_snd["e"][0]["t"] = t;
    doc_snd["e"][0]["v"] = v;
    doc_snd["e"][0]["u"] = (char*)NULL;
  }

  String output;
  //crea json dal documento dinamico
  serializeJson(doc_snd, output);
  return output;
}
