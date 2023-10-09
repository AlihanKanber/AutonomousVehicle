int b = 100; // can id
int x;
int siny;
#include <mcp2515.h>
#include <SPI.h>
struct can_frame can;
MCP2515 mcp2515(10);
byte sol = A2;
byte sag = A4;
byte hiz1 = 2;
byte hiz2 = 3;
byte vites = 4;
byte acil = 5;
byte far = 6;
byte role = 7;
byte dortlu = 8;
byte selektor = 9;
unsigned long taym = millis();

void setup() {
  SPI.begin();
  Serial.begin(9600);
  for (byte i = 2; i <= 6; i++) {
    pinMode(i, INPUT_PULLUP);
  }
  pinMode(sol, INPUT_PULLUP);
  pinMode(sag, INPUT_PULLUP);
  pinMode(dortlu, INPUT_PULLUP);
  pinMode(selektor, INPUT_PULLUP);
  pinMode(role, OUTPUT);
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  can.can_id = b;
  can.can_dlc = 8;
  can.data[0] = 40;
}
void sinyal() {

  can.data[5] = siny;

  if ((digitalRead(sol) == 1) && (digitalRead(sag) == 1) && (digitalRead(dortlu) == 1)) { //kapali
  siny = 0;
  Serial.println("kapali");
  } else if ((digitalRead(sol) == 0) && (digitalRead(sag) == 1)) { //sol sin
  siny = 1;
  Serial.println("sol");

  } else if ((digitalRead(sol) == 1) && (digitalRead(sag) == 0)) { //sag sin
  siny = 2;
  Serial.println("sag");
  }
  else if (digitalRead(dortlu) == 0) { //dortlu
  siny = 3;
  Serial.println("dortlu");
  }
}
void hiz() { //hız  ve vites
  can.data[4] = x;
  if ((digitalRead(hiz2) == 1) && (digitalRead(hiz1) == 1) && (digitalRead(vites) == 1)) { //Normal geri
    x = 2;
  } else if ((digitalRead(hiz2) == 1) && (digitalRead(hiz1) == 1) && (digitalRead(vites) == 0)) { // NORMAL ileri
    x = 3;
  } else if ((digitalRead(hiz2) == 1) && (digitalRead(hiz1) == 0) && (digitalRead(vites) == 1)) { //yavaş geri
    x = 0;

  } else if ( (digitalRead(hiz2) ==  1) && (digitalRead(hiz1) == 0) && (digitalRead(vites) == 0)) { // yavaş ileri
    x = 1;

  } else if ( (digitalRead(hiz2) ==  0) && (digitalRead(hiz1) == 1) && (digitalRead(vites) == 1)) { // hızlı geri
    x = 4;
  } else if ( (digitalRead(hiz2) ==  0) && (digitalRead(hiz1) == 1) && (digitalRead(vites) == 0)) { // hızlı ileri
    x = 5;
  }
}
void pedal() {
  int fren_pedal = analogRead(A0);
  int gaz_pedal = analogRead(A1); //170 860
  int frr;
  if (fren_pedal > 500) {

    fren_pedal = 860;
    gaz_pedal = 170;
    frr = 1;
  } else {
    fren_pedal = analogRead(A0);
    gaz_pedal = analogRead(A1);
    frr = 0;
  }

  can.data[2] = map(fren_pedal, 170, 860, 0, 100); //fren pedal kalibre edilmeli
  can.data[3] = map(gaz_pedal, 170, 860, 0, 100); //gaz datası
  can.data[1] = frr;
}
void farr() { //far selektör
  if (digitalRead(far) == 0) {
    digitalWrite(role, HIGH);
  } else {
    digitalWrite(role, LOW);
  }
  if (digitalRead(selektor) == 0) {
    digitalWrite(role, !digitalRead(role));
  }
}
void acild() {
  if (digitalRead(acil) == 1) {
    can.data[6] = 1;
  } else {
    can.data[6] = 0;
  }
}
void sendcan(int y) {
  bool d = 1;
  if ((millis() - taym) > y) {
    d = !d;
    taym = millis();
    mcp2515.sendMessage(&can);
  }
}
void loop() {
  acild();
  pedal();
  farr();
  hiz();
  sinyal();
  sendcan(100);


}
