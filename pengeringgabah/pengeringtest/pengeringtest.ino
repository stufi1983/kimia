#define TOMBOL1 A0
#define TOMBOL2 A1
#define TOMBOL3 A2

#define SENSORSUHU 2

#define DHTPIN  3

#define HEATERPIN 5
#define EXHAUSTPIN 6

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4); // set the LCD address to 0x27 for a 16 chars and 2 line display

#include <OneWire.h>
#include <DallasTemperature.h>

OneWire oneWire(SENSORSUHU);

DallasTemperature sensors(&oneWire);

//DeviceAddress sensorSuhuDepan, sensorSuhuBelakang;
DeviceAddress sensorSuhuBelakang = { 0x28, 0x32, 0xD7, 0x75, 0xD0, 0x01, 0x3C, 0x7B }; //2832D775D0013C7B
DeviceAddress sensorSuhuDepan    = { 0x28, 0xD3, 0x1F, 0x75, 0xD0, 0x01, 0x3C, 0x79 }; //28D31F75D0013C79

bool sensorOK = false;

#include "DHT.h"
#define DHTTYPE DHT11   // DHT 11

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  pinMode(HEATERPIN, OUTPUT); digitalWrite(HEATERPIN, LOW);
  pinMode(EXHAUSTPIN, OUTPUT); digitalWrite(EXHAUSTPIN, LOW);

  lcd.init();
  lcd.clear();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print(F("Pengering Mokaf"));

  Serial.begin(115200);

  sensors.begin();
  Serial.print(F("Jumlah Sensor: "));
  Serial.println(sensors.getDeviceCount(), DEC);

  lcd.setCursor(0, 0);
  if (!oneWire.search(sensorSuhuDepan)) {
    Serial.println("Unable to find address for sensorSuhuDepan");
    lcd.print(F("Suhu depan error"));
    return;
  }
  if (!oneWire.search(sensorSuhuBelakang)) {
    Serial.println("Unable to find address for sensorSuhuBelakang");
    lcd.print(F("Suhu belakang error"));
    return;
  }

  dht.begin();
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println("Unable to read DHT");
    lcd.print(F("Sensor DHT error"));
    return;
  }

  Serial.println("TDepan TBlkg Humidity SetPoint");

  sensorOK = true;
  lcdClearLine(3); lcd.setCursor(0, 3); lcd.print(F("Heater      :"));


float TD = 0, TB = 0, TH = 0, H = 0; float sp = 40.0;
void loop() {
  if (!sensorOK) {
    return;
  }

  getSensorData(TD, TB, TH, H);
  displaySuhuLembab(TD, TB, TH, H);

  lcd.setCursor(0, 3); lcd.print(F("Heater    :"));
  if (((TD+TB)/2) < sp) {
    digitalWrite(HEATERPIN, HIGH);
    digitalWrite(EXHAUSTPIN, LOW);
    lcd.print(F("ON "));

  } else {
    digitalWrite(HEATERPIN, LOW);
    digitalWrite(EXHAUSTPIN, HIGH);
    lcd.print(F("OFF"));
  }

  // wait for a second
}

void lcdClearLine(int lineNumber) {
  lcd.setCursor(0, lineNumber);
  lcd.print(F("                   "));
}

void displaySuhuLembab(float &TD, float &TB, float &TH, float &H) {

  lcdClearLine(1); lcd.setCursor(0, 1); lcd.print(F("Suhu      :")); lcd.print((TD+TB)/2); lcd.print(F("C"));
  lcdClearLine(2); lcd.setCursor(0, 2); lcd.print(F("Kelembaban:")); lcd.print((int)H); lcd.print(F("%"));
}

void getSensorData(float &TD, float &TB, float &TH, float &H) {
  sensors.requestTemperatures(); // Send the command to get temperatures

  float tempsensorSuhuDepan = sensors.getTempC(sensorSuhuDepan);
  if (tempsensorSuhuDepan != DEVICE_DISCONNECTED_C) {
    //Serial.print(F("sensorSuhuDepan: "));
    Serial.print(tempsensorSuhuDepan);
    Serial.print(",");
    TD = tempsensorSuhuDepan;
  } else {
  }

  float tempsensorSuhuBelakang = sensors.getTempC(sensorSuhuBelakang);
  if (tempsensorSuhuBelakang != DEVICE_DISCONNECTED_C) {
    //Serial.print(F("sensorSuhuBelakang: "));
    Serial.print(tempsensorSuhuBelakang);
    Serial.print(",");
    TB = tempsensorSuhuBelakang;
  } else {
  }

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) {
    //    Serial.print(F("H:"));
    Serial.print(h);
    Serial.print(",");
    //    Serial.print(F("T:"));
    //    Serial.print(t);
    //    Serial.println();
    TH = t;
    H = h;
  } else {
    //Serial.println(F("DHT Read failed!"));
  }
  Serial.println(sp);
}
