/*
 * Project Power_Monitor
 * Description:
 * Author:
 * Date:
 */

/*
  Library for the Allegro MicroSystems ACS37800 power monitor IC
  By: Paul Clark
  SparkFun Electronics
  Date: December 4th, 2021
  License: please see LICENSE.md for details

  Feel like supporting our work? Buy a board from SparkFun!
  https://www.sparkfun.com/products/17873
*/

#include "../lib/ACS37800/src/SparkFun_ACS37800_Arduino_Library.h"
#include "Wire.h"

// Include ST7789 TFT Display libraries //
#include "../lib/Adafruit_GFX_RK/src/Adafruit_GFX.h"
#include "../lib/Adafruit_ST7735_RK/src/Adafruit_ST7789.h"
#include "../lib/Adafruit_GFX_RK/src/FreeSansBold12pt7b.h"
#include "../lib/Adafruit_GFX_RK/src/FreeSansBold9pt7b.h"
#include "../lib/Adafruit_GFX_RK/src/FreeSans12pt7b.h"
#include "../lib/Adafruit_GFX_RK/src/FreeSans9pt7b.h"
#include "../lib/GFX/src/icon.h"
#include <SPI.h>

// ST7789 TFT  definitions // 
#define TFT_CS        S3                                            // Define CS pin for TFT display
#define TFT_RST       D6                                            // Define RST pin for TFT display
#define TFT_DC        D5                                            // Define DC pin for TFT display

Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);     // Hardware SPI

ACS37800 mySensor;                                                  //Create an object of the ACS37800 class

float volts = 0.0;
float prev_volts = 0.0;
float amps = 0.0;
float prev_amps = 0.0;
float papparent = 0.0;
float prev_papparent = 0.0;
float pfactor = 0.0;
float prev_pfactor = 0.0;
float pactive = 0.0;
float prev_pactive = 0.0;
float preactive = 0.0;
float prev_preactive = 0.0;

float OV_Set = 200.00;                                                // Set Over Voltage limit
float OC_Set = 10.0;                                                  // Set Over Current limit
float W_Set = ((OV_Set * OC_Set) / 1000);                             // Set Over Current limit

int Alarm_ledState = LOW;                                             // State used to set LED
unsigned long previousMillis = 0;                                     // Timer for blinking LED without delay() function
const long interval = 1000;                                           // interval at which to blink (milliseconds)

void setup() {

  pinMode(A2, OUTPUT);
  pinMode(A5, OUTPUT);

  tft.init(320, 240);                                                 // Init ST7789 320x240 
  tft.fillScreen(ST77XX_BLACK);                                       // creates black background in display
  tft.setRotation(3); 

  draw_screen();

  Serial.begin(115200);
  Serial.println(F("ACS37800 Example"));

  Wire.begin();

  //mySensor.enableDebugging(); // Uncomment this line to print useful debug messages to Serial

  //Initialize sensor using default I2C address
  if (mySensor.begin() == false)
  {
    Serial.print(F("ACS37800 not detected. Check connections and I2C address. Freezing..."));
    while (1)
      ; // Do nothing more
  }

  // From the ACS37800 datasheet:
  // CONFIGURING THE DEVICE FOR AC APPLICATIONS : DYNAMIC CALCULATION OF N
  // Set bypass_n_en = 0 (default). This setting enables the device to
  // dynamically calculate N based off the voltage zero crossings.
  mySensor.setBypassNenable(false, false); // Disable bypass_n in shadow memory and eeprom

  

  // We need to connect the LO pin to the 'low' side of the AC source.
  // So we need to set the divider resistance to 4M Ohms (instead of 2M).
  mySensor.setDividerRes(4000000); // Comment this line if you are using GND to measure the 'low' side of the AC voltage
}

void LEDs() {

  if (volts > OV_Set) { 
    alarm_led();

    } else if (volts > 1.0) {
      analogWrite(A2, 255);
      analogWrite(A5, 0);
    
    } else if (volts < 1.0) {
      analogWrite(A2, 0);
      analogWrite(A5, 255);
  }
}

void alarm_led() {

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (Alarm_ledState == LOW) {
      Alarm_ledState = HIGH;
    } else {
      Alarm_ledState = LOW;
    }
    // set the LED with the ledState of the variable:
    digitalWrite(A2, Alarm_ledState);
    digitalWrite(A5, LOW);

    }

  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (Alarm_ledState == LOW) {
      Alarm_ledState = HIGH;
    } else {
      Alarm_ledState = LOW;
    }
    // set the LED with the ledState of the variable:
    digitalWrite(A2, LOW);
    digitalWrite(A5, Alarm_ledState);

    }
}
  
void draw_screen() {

  tft.fillRect(0,0,160,120,ST77XX_BLUE);                                               // draws background fills for readings
  tft.fillRect(20,80,120,30,ST77XX_WHITE);
  
  tft.fillRect(0,121,160,120,ST77XX_GREEN);
  tft.fillRect(20,201,120,30,ST77XX_WHITE);                                               
    
  tft.fillRect(161,0,160,120,ST77XX_RED);
  tft.fillRect(181,80,120,30,ST77XX_WHITE);

  tft.fillRect(161,121,160,120,ST77XX_WHITE);

  //tft.drawBitmap(0,0,icon);

////// Main headings ///////
  tft.setFont(&FreeSansBold9pt7b);
  tft.setTextSize(2);
  tft.setTextWrap(false);

  tft.setCursor(115, 40);                                                                 // set sursor to start writing text
  tft.print("V");
 
  tft.setCursor(275, 40);
  tft.println("A");

  tft.setCursor(110, 165); 
  tft.println("W");

////// Sub headings ///////
  tft.setFont(&FreeSans9pt7b);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setCursor(25, 102);                                                                 // set sursor to start writing text
  tft.print("O.V. ");
  tft.print(OV_Set);
  tft.print("V");

  tft.setCursor(185, 102);                                                                // set sursor to start writing text
  tft.print("O.C. ");
  tft.print(OC_Set);
  tft.print("A");

  tft.setCursor(25, 222);                                                                // set sursor to start writing text
  tft.print("SET: ");
  tft.print(W_Set);
  tft.print("kW");

  
////// Addtional Info ///////  
  tft.setFont(&FreeSans12pt7b);
  tft.setTextColor(ST77XX_BLACK);
  tft.setTextSize(1);
  
  tft.setCursor(175, 155);
  tft.print("PF :"); 

  tft.setCursor(175, 185);
  tft.print("PA :"); 

  tft.setCursor(175, 215);
  tft.print("VAR :"); 

}

  void print_values() {
   
  tft.setFont(&FreeSansBold12pt7b);
  tft.setTextSize(1);
  tft.setTextWrap(false);
  
  tft.setCursor(15, 40);
  tft.setTextColor(ST77XX_BLUE);
  tft.println(prev_volts);
  tft.setCursor(15, 40);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(volts,2 );

  prev_volts = volts;
  
  tft.setCursor(175, 40);
  tft.setTextColor(ST77XX_RED);
  tft.println(prev_amps);
  tft.setCursor(175, 40); 
  tft.setTextColor(ST77XX_WHITE);
  tft.println(amps, 2); 

  prev_amps = amps;

  tft.setCursor(15, 165);
  tft.setTextColor(ST77XX_GREEN);
  tft.println(prev_papparent);
  tft.setCursor(15, 165);
  tft.setTextColor(ST77XX_WHITE);
  tft.println(papparent, 2);
  
  prev_papparent = papparent;

  tft.setFont(&FreeSans12pt7b);
  tft.setTextSize(1);
  
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(255, 155);
  tft.println(prev_pfactor); 
  tft.setTextColor(ST77XX_BLACK);
  tft.setCursor(255, 155);
  tft.println(pfactor, 2);

  prev_pfactor = pfactor;
  
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(255, 185);
  tft.println(prev_pactive); 
  tft.setTextColor(ST77XX_BLACK);
  tft.setCursor(255, 185);
  tft.println(pactive, 2); 

  prev_pactive = pactive;

  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(255, 215);
  tft.println(prev_preactive); 
  tft.setTextColor(ST77XX_BLACK);
  tft.setCursor(255, 215);
  tft.println(preactive, 2); 

  prev_preactive = preactive;

  delay(50);
}

void measure() {

  mySensor.readRMS(&volts, &amps);                                            // Read the RMS voltage and current
  Serial.print(F("Volts: "));
  Serial.print(volts, 2);
  Serial.print(F(" Amps: "));
  Serial.println(amps, 2);

 // float pactive = 0.0;
 // float preactive = 0.0;
  
  mySensor.readPowerActiveReactive(&pactive, &preactive);                    // Read the active and reactive power
  Serial.print(F("Power: Active (W): "));
  Serial.print(pactive, 2);
  Serial.print(F(" Reactive (VAR): "));
  Serial.println(preactive, 2);
  
  bool posangle = 0;
  bool pospf = 0;
  
  mySensor.readPowerFactor(&papparent, &pfactor, &posangle, &pospf);        // Read the apparent power and the power factor
  Serial.print(F("Power: Apparent (VA): "));
  Serial.print(papparent, 2);
  Serial.print(F(" Power Factor: "));
  Serial.print(pfactor, 3);
    if (posangle)
      Serial.print(F(" Lagging"));
    else
      Serial.print(F(" Leading"));
    if (pospf)
      Serial.println(F(" Consumed"));
    else
      Serial.println(F(" Generated"));

  delay(250);

}

void loop() {

  LEDs();
  measure();
  print_values();

}
