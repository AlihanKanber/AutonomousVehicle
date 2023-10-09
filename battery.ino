int b_canid =0x66;
int NTC_1 = A1;
int NTC_2 = A2;
int NTC_3 = A3;
int Vo_1;
int Vo_2;
int Vo_3;
int akim;
byte role1 = 2; //fan
byte role2 = 4; //kontaktör
float R1 = 10000;
float logR2, R2, T, Tc_1, Tc_2, Tc_3;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
const float arduinoVCC = 5.01;
unsigned long ValueR1 = 5100;
unsigned long ValueR2 = 100000;
double Voltage_Source = 100;
const int analogpin = A4;
const int inputResolution = 1023;
const float average_of = 500;
float voltage;
unsigned long taym = millis();
#include <mcp2515.h>
struct can_frame canMsg;
struct can_frame canMsg1;
MCP2515 mcp2515(10); //canbus cs pin girişi
void setup() {
  SPI.begin();
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  canMsg1.can_id = b_canid; // modülün can idsi
  canMsg1.can_dlc = 4;
  Serial.begin(9600);
  pinMode(2, OUTPUT); //Fan röle
  pinMode(4, OUTPUT); // kontaktör kesme
  digitalWrite(2,HIGH);
  digitalWrite(4,HIGH);
}
void readVoltage() {
  int A0Value = analogRead(analogpin);
  float voltage_sensed = A0Value * (arduinoVCC / (float)inputResolution);
  voltage = voltage_sensed * ( 1 + ( (float) ValueR2 /  (float) ValueR1) );

}

float getVoltageAverage() {
  float voltage_temp_average = 0;
  for (int i = 0; i < average_of; i++)
  {
    readVoltage();
    voltage_temp_average += voltage;
  }

  return  voltage_temp_average / average_of;
  //Robojax.com ARDVC-01 Measure any voltage with Arduino
}//getVoltageAverage
void kontrol() {
  if (Tc_1 > 45 || Tc_2 > 45 || Tc_3 > 45) {
    Serial.println ("aktif soğutmayı aç");
    digitalWrite(2, LOW);

  }
  else if (Tc_1 < 35 || Tc_2 < 35 || Tc_3 < 35) {
    Serial.println("soğutma kapat");
    digitalWrite(2, HIGH);
  }

  if (Tc_1 > 60   || Tc_2 > 60 || Tc_3 > 60) {
    digitalWrite(4, LOW);
    Serial.println("gücü kes");
  }
}
void sicaklik_sensor() {
  Vo_1 = analogRead(NTC_1);
  R2 = R1 * (1023.0 / (float)Vo_1 - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  Tc_1 = T - 273.15;

  Serial.print("Temperature_Battery1: ");
  Serial.print(Tc_1);
  Serial.println(" C");
  Vo_2 = analogRead(NTC_2);
  R2 = R1 * (1023.0 / (float)Vo_2 - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  Tc_2 = T - 273.15;

  Serial.print("Temperature_Battery2: ");
  Serial.print(Tc_2);
  Serial.println(" C");


  Vo_3 = analogRead(NTC_3);
  R2 = R1 * (1023.0 / (float)Vo_3 - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  Tc_3 = T - 273.15;

  Serial.print("Temperature_BMS: ");
  Serial.print(Tc_3);
  Serial.println(" C");

}
void sendcan(int x) {
  bool d = 1;
  if ((millis() - taym) > x) {
    d = !d;
    taym = millis();
    mcp2515.sendMessage(&canMsg1);

  }
}
void loop() {
  readVoltage();
  kontrol();
  sicaklik_sensor();
  Serial.print("Vin: ");
  Serial.print(getVoltageAverage());
  Serial.println("V");
  canMsg1.data[0] = getVoltageAverage();
  canMsg1.data[1] = Tc_2;
  canMsg1.data[2] = Tc_3;
  canMsg1.data[3] = Tc_1;
  //canMsg1.data[4] = akim;
  sendcan(100);
  if (mcp2515.readMessage(&canMsg) ==  MCP2515:: ERROR_OK) {
    if (canMsg.can_id == 100) {
      int acil = canMsg.data[6];
      if (acil == 1) {
      digitalWrite(role2, LOW);

      } else {
        digitalWrite(role2, HIGH);
      }

    }
}

}
