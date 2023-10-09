#include <Servo.h>
#include <SPI.h>
#include <mcp2515.h>
Servo onfren;
byte lamba = 0;
int ss = 300; //sinyallerin yanıp sönme süresi
byte r_pwm = 10;
byte l_pwm = 9;
byte servo = 8;
byte can = 4;
byte role1 = 7; //fren
byte role2 = 6; //sol sinyal
byte role3 = 5; //sag sinyal
byte role4 = 2; //dortlu flasor
byte hall = 3; //hall effect
struct can_frame can1;
MCP2515 mcp2515(can);

unsigned long taym = millis();


void setup() {
  onfren.attach(servo);
  Serial.begin(9600);
  SPI.begin();
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  for (byte i = 5; i <= 7; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, 1);
  }
  pinMode(r_pwm, OUTPUT);
  pinMode(l_pwm, OUTPUT);
  Serial.begin(9600);
  pinMode(role4, OUTPUT);
  digitalWrite(role4, 1);



}
void sinyal(int y, int x) {
  if ((millis() - taym) > y) {
    digitalWrite(x, !digitalRead(x));
    taym = millis();

  }
}
void dortlu(int y) {
  if ((millis() - taym) > y) {
    digitalWrite(role2, !digitalRead(role2));
    digitalWrite(role3, !digitalRead(role3));
    taym = millis();

  }
}
void servo_motor() {
  int data2 = can1.data[2];
  int servoaci = map(data2, 0, 100, 180, 0);
  onfren.write(servoaci);
  //Serial.println(servoaci);
  if (data2 >= 10) { //fren ışığı
    lamba = 1;

  } else {
    lamba = 0;
  }
}
void el_freni() {
  int data1 = can1.data[1];
  if (data1 == 1) {// Serial.println("fren");
    digitalWrite( r_pwm, HIGH);
    digitalWrite(l_pwm, LOW);

    lamba = 1;
  } else if ( data1 == 0) {
    //Serial.println("fren yapmiyor");
    if (digitalRead(hall) == 0) {

      digitalWrite( r_pwm, LOW);
      digitalWrite(l_pwm, HIGH); //freni salıyor

    } else {

      digitalWrite( r_pwm, LOW);
      digitalWrite(l_pwm, LOW);
    }

    lamba = 0;
  } else {
    digitalWrite( r_pwm, LOW);
    digitalWrite(l_pwm, LOW);
    lamba = 0;
  }
}
void arka_stop() {
  if (lamba  == 1 ) {
    digitalWrite(role1, LOW);
  } else {
    digitalWrite(role1, HIGH);

  }
}
void loop() {
  Serial.println(digitalRead(hall));
  if (mcp2515.readMessage(&can1) == MCP2515::ERROR_OK) {
    if (can1.can_id == 0x64) {
      el_freni();
      servo_motor();
      arka_stop();
      //Serial.print("stop ");
      //Serial.println(lamba);
      int sinyal_kolu = can1.data[5];
      //Serial.println(sinyal_kolu);
      if (sinyal_kolu == 0) {
        digitalWrite(role2, HIGH);
        digitalWrite(role3, HIGH);
        digitalWrite(role4, HIGH);
      } else if (sinyal_kolu == 1) { //sol sinyal
        sinyal(ss, role2);
        digitalWrite(role3, 1);
        digitalWrite(role4, 1);

      }

      else if (sinyal_kolu == 2) { //sag sinyal
        sinyal(ss, role3);
        digitalWrite(role2, 1);
        digitalWrite(role4, 1);

      } else if (sinyal_kolu == 3) { //dortlu
        dortlu(ss);

      } else {
        digitalWrite(role2, 1);
        digitalWrite(role3, 1);
        digitalWrite(role4, 1);
      }

    }

  }
}
