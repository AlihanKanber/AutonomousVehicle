float teker_cap = 0.4;
int x;
int km_h;
int role4 = 7;
int hall = 3;
int gaz = 5;
int role1 = 2; //vites;
int role3 = 8;
int role2 = 9; //hızlı yavaş;
int canbus = 10;
#include <SPI.h>
#include <mcp2515.h>
struct can_frame can1;
struct can_frame can2;
MCP2515 mcp2515(canbus);
unsigned long rpmtime;
float rpmfloat;
unsigned int rpm;
bool tooslow = 1;
unsigned long taym = millis();
void setup() {
  pinMode(role1, OUTPUT);
  pinMode(role2, OUTPUT);
  pinMode(role3, OUTPUT);
  pinMode(role4, OUTPUT);
  for ( int i = 7; i <= 9; i++) {
    digitalWrite(i, HIGH);

  }
  digitalWrite(role1, HIGH);
  Serial.begin(9600);
  SPI.begin();
  mcp2515.reset();
  mcp2515.setBitrate(CAN_500KBPS, MCP_8MHZ);
  mcp2515.setNormalMode();
  can2.can_id = 0x65; // modül ID
  can2.can_dlc = 7;
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1B |= (1 << CS12); //Prescaler 256
  TIMSK1 |= (1 << TOIE1); //enable timer overflow
  pinMode(3, INPUT);
  attachInterrupt(1, RPM, FALLING);
}
ISR(TIMER1_OVF_vect) {
  tooslow = 1;
}
void hiz_rpm() {
  if (tooslow == 1) {
    rpm = 0;
    //Serial.println(rpm);
    km_h = 0;
  }
  else {
    rpmfloat = 120 / (rpmtime / 31250.00);
    rpm = round(rpmfloat);
    //Serial.println(rpm);
    km_h = teker_cap * rpm * 0.1885;
  }
  can2.data[0] = rpm;
  can2.data[1] = km_h;

}
void RPM () {
  rpmtime = TCNT1;
  TCNT1 = 0;
  tooslow = 0;
}
void sicaklik() {
  int ThermistorPin = 0;
  int Vo;
  float R1 = 10000;
  float logR2, R2, T;
  float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
  Vo = analogRead(ThermistorPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  T = T - 273.15;
  //T = (T * 9.0) / 5.0 + 32.0;
  can2.data[2] = T;
  //Serial.println(T);
}
void sendcan(int x) {
  bool d = 1;
  if ((millis() - taym) > x) {
    d = !d;
    taym = millis();
    mcp2515.sendMessage(&can2);

  }
}

void loop() {
  if (mcp2515.readMessage(&can1) == MCP2515:: ERROR_OK) {
    if (can1.can_id == 0x64) {
      int a = can1.data[3]; //gaz datası
      int b = can1.data[2]; //fren datası servo
      int c = can1.data[1]; //fren datası el freni
      int h = can1.data[4]; //vites
      int gaz_datasi = map(a, 0, 100, 50, 203);
      analogWrite(gaz, gaz_datasi );
      // Serial.print("gaz=");
      //Serial.println(a);


      /*
         röle 1= vites LOW iken ileri HIGH iken geri
         röle 2 = hız seçici HIGH iken hızlı low ise yavaş
         röle 3 normal hız = high ise röle 2 nin durumuna göre low ise normal
      */
      if (h == 0)  { //YAVAŞ GERİ
        digitalWrite(role1, HIGH);
        digitalWrite(role2, LOW);
        digitalWrite(role3, HIGH);
      } else if (h == 1)  { //YAVAŞ İLERİ
        digitalWrite(role1, LOW);
        digitalWrite(role2, LOW);
        digitalWrite(role3, HIGH);

      } else if (h == 2) { //NORMAL GERİ
        digitalWrite(role1, HIGH); //GERİ
        digitalWrite(role2, LOW); // bunun durumu farketmez
        digitalWrite(role3, LOW); // NORMAL

      } else if (h == 3) { //NORMAL İLERİ

        digitalWrite(role1, LOW); // VİTES İLERİ
        digitalWrite(role2, LOW); //FARKETMEZ
        digitalWrite(role3, LOW); //NIRMAL

      } else if (h == 4) { //HIZLI GERİ
        digitalWrite(role1, HIGH); // VİTES GERİ
        digitalWrite(role2, HIGH); //HIZLI
        digitalWrite(role3, HIGH); // HIGH OLMASI LAZIM

      } else if (h == 5) { //hızlı ileri
        digitalWrite(role1, LOW); // VİTES İLERİ
        digitalWrite(role2, HIGH); //HIZLI
        digitalWrite(role3, HIGH); // HIGH OLMASI LAZIM


      } else {
      }
      if  (b > 24) {
        digitalWrite(role4, LOW);
        Serial.println("fren");
      } else if (b < 24) {
        digitalWrite(role4, HIGH);
        Serial.println("fren yok");

      }

    }

  }
  sicaklik();
  hiz_rpm();
  sendcan(100);




}
