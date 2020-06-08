float current_speed = 0;
int FAN_PIN = 10;
const float step = 25.5;

void setup() {
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Lab 1.4 starting");
  
  pinMode(FAN_PIN,OUTPUT);  
  analogWrite(FAN_PIN, (int) current_speed);
}

void loop() {
  if(Serial.available()>0)
  {
    String command = Serial.readString();

    if(command.equals("max"))
    {
      analogWrite(FAN_PIN, 255);
      current_speed = 255;
      Serial.println("Increased to max");
      return;
    }
    if(command.equals("min"))
    {
      analogWrite(FAN_PIN, 0);
      current_speed = 0;
      Serial.println("Increased to min");
      return;
    }
    
    if(command.equals("+"))
      add();
    else
      if(command.equals("-"))
        sub();
      else
        Serial.println("Command not recognised");
  }
}

void add()
{
  if(current_speed >=255)
    Serial.println("Already at max speed");
  else
  {
    current_speed += step;
    analogWrite(FAN_PIN, (int) current_speed);
    Serial.print("Increasing speed: ");
    Serial.println(current_speed);
  }
}

void sub()
{
  if(current_speed <=0)
    Serial.println("Already at min speed");
  else
  {
    current_speed -= step;
    analogWrite(FAN_PIN, (int) current_speed);
    Serial.print("Decreasing speed: ");
    Serial.println(current_speed);
  }
}
