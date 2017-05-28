      
#include <LiquidCrystal.h>
#include <TimerOne.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PID_v1.h>

#include "max6675.h"

int thermoDO = 2;
int thermoCS = 3;
int thermoCLK = 4;
MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);
int vccPin = 5;
int gndPin = A0;


//Define Variables we'll be connecting to
double Setpoint, Input, Output;
//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &Setpoint, 3, 1, 5, DIRECT);

// Data wire is plugged into pin 2 on the Arduino
#define ONE_WIRE_BUS 6
// Setup a oneWire instance to communicate with any OneWire devices
// (not just Maxim/Dallas temperature ICs)
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);
int sensorValue = 0;

/*******************
 * LCD VSS (1) pin to ground
 * LCD VCC (2) pin to 5V
 * 10K resistor: ends to +5V and ground wiper to LCD VO pin (pin 3)
 * LCD RS(4) pin to digital pin 12
 * LCD R/W (5) pin to ground
 * LCD Enable(6) pin to digital pin 11
 * LCD D4 (11) pin to digital pin 10
 * LCD D5 (12) pin to digital pin 9
 * LCD D6 (13) pin to digital pin 8
 * LCD D7 (14) pin to digital pin 7
 */
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);

int VRValue = 0;
const int analogInPin = A2;
const int opto = A4;
const int optopid = 3;

void setup() {
  //LED
  pinMode(13, OUTPUT);
  pinMode(opto, OUTPUT); digitalWrite(opto, LOW);
  Timer1.initialize(100000); //mS
  Timer1.attachInterrupt( timerIsr );

  //LCD
  lcd.begin(16, 2);
  //lcd.clearDisplay();
  lcd.setCursor(0, 0); lcd.print("Set Suhu :");
  lcd.setCursor(0, 1); lcd.print("Suhu kini:");

  digitalWrite(A3, LOW);
  digitalWrite(A1, HIGH);

  digitalWrite(13, HIGH);

  Serial.begin(9600);

  // use Arduino pins for MAX6675
  pinMode(vccPin, OUTPUT); digitalWrite(vccPin, HIGH);
  pinMode(gndPin, OUTPUT); digitalWrite(gndPin, LOW);
  
  Serial.println("MAX6675 test");
  // wait for MAX chip to stabilize
  delay(500);

  //DS1820 Start up the library
  sensors.begin(); // IC Default 9 bit. If you have troubles consider upping it 12. Ups the delay giving the IC more time to process the temperature measurement

  // read the analog in value:
  VRValue = readTemperature(analogInPin);
  // map it to the range of the analog out:
  VRValue = map(VRValue, 0, 1023, 30, 320);
  // change the analog out value:
  Setpoint = VRValue ;

  sensors.requestTemperatures(); // Send the command to get temperatures
  sensorValue = sensors.getTempCByIndex(0);
  Input = sensorValue;
myPID.SetOutputLimits(-255, 255);
  //turn the PID on
  myPID.SetMode(AUTOMATIC);
}

int LVRValue  = 0;
int changed = 0;
void loop() {
  lcd.setCursor(11, 0);
  // read the analog in value:
  VRValue = readTemperature(analogInPin);
  // map it to the range of the analog out:
  VRValue = map(VRValue, 0, 1023, 30, 320);
  if (LVRValue != VRValue)changed = 1;
  LVRValue = VRValue;
  // change the analog out value:
  Serial.print(VRValue, DEC);

  //if ( ((LVRValue - VRValue) == 1) || ((LVRValue - VRValue) == -1) )
  ///{}
  //else {

  lcd.print(VRValue); lcd.write(B11011111); lcd.print("C ");
  //}


  lcd.setCursor(11, 1);
  sensors.requestTemperatures(); // Send the command to get temperatures
  //-sensorValue = thermocouple.readCelsius();
  sensorValue = sensors.getTempCByIndex(0);
  Serial.println(sensorValue, DEC);
  lcd.print(sensorValue); lcd.write(B11011111); lcd.print("C ");

  Setpoint = VRValue ;
  Input = sensorValue;


  // ONOFF
  if (changed==1) {
    if (sensorValue > (LVRValue - 1)) {
      digitalWrite(opto, LOW);
      changed=0;
    }
    else {
      digitalWrite(opto, HIGH);
    }
  } else {
    if (sensorValue > (LVRValue - 5)) {
      digitalWrite(opto, LOW);
    }
    else {
      digitalWrite(opto, HIGH);
    }
  }


  //PID
  myPID.Compute();
  //analogWrite(optopid,Output);
  Serial.println(Output);
  if(Output<0){Serial.print('K');Serial.println((byte)abs(Output),DEC);}
  if(Output>0){Serial.print('H');Serial.println((byte)Output,DEC);}

  noInterrupts();
  //lcd.setCursor(0, 1);
  //lcd.print(millis() / 1000);
  interrupts();
  delay(20);
}


void timerIsr()
{
  digitalWrite( 13, digitalRead( 13 ) ^ 1 );
}




#define NUM_READS 10
float readTemperature(int sensorpin) {
  // read multiple values and sort them to take the mode
  int sortedValues[NUM_READS];
  for (int i = 0; i < NUM_READS; i++) {
    int value = analogRead(sensorpin);
    int j;
    if (value < sortedValues[0] || i == 0) {
      j = 0; //insert at first position
    }
    else {
      for (j = 1; j < i; j++) {
        if (sortedValues[j - 1] <= value && sortedValues[j] >= value) {
          // j is insert position
          break;
        }
      }
    }
    for (int k = i; k > j; k--) {
      // move all values higher than current reading up one position
      sortedValues[k] = sortedValues[k - 1];
    }
    sortedValues[j] = value; //insert current reading
  }
  //return scaled mode of 10 values
  float returnval = 0;
  for (int i = NUM_READS / 2 - 5; i < (NUM_READS / 2 + 5); i++) {
    returnval += sortedValues[i];
  }
  returnval = returnval / 10;
  return returnval * 1100 / 1023;
}
