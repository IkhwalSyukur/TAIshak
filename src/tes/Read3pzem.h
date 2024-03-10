 
#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <LiquidCrystal_I2C.h>
#include "algorithm"

#define MY_MAX(a, b, c) max(max(a, b), c)
#define relay 2

#define PZEM_RX_PIN 16 
#define PZEM_TX_PIN 17 

#define NUM_PZEMS 3

int button;

PZEM004Tv30 pzems[NUM_PZEMS];

#define PZEM_SERIAL Serial2

int voltage[3];
int current[3];
int power[3];
int energy[3];
float frequency[3];
float pf[3];
int VoltLL[3];
int VoltMAX;
int VoltDEF;
int VoltEV;
float PerUnb;
int wifiFlag = 0;


LiquidCrystal_I2C lcd(0x27, 16, 2);

void ReadSenser(){
      // Print out the measured values from each PZEM module
    for(int i = 0; i < NUM_PZEMS; i++){
        // Print the Address of the PZEM
        Serial.print("PZEM ");
        Serial.print(i);
        Serial.print(" - Address:");
        Serial.println(pzems[i].getAddress(), HEX);
        Serial.println("===================");

        // Read the data from the sensor
        voltage[i] = pzems[i].voltage();
        current[i] = pzems[i].current();
        power[i] = pzems[i].power();
        energy[i] = pzems[i].energy();
        frequency[i] = pzems[i].frequency();
        pf[i] = pzems[i].pf();
        VoltLL[i] = voltage[i]*1.73;
        VoltMAX = MY_MAX(VoltLL[0], VoltLL[1], VoltLL[2]);
        VoltEV = ((VoltLL[0] + VoltLL[1] + VoltLL[2])/3);
        VoltDEF = VoltMAX - VoltEV;
        PerUnb = ((100*VoltDEF)/VoltEV);
        
        // Check if the data is valid
        if(isnan(voltage[i])){
            Serial.println("Error reading voltage");
        } else if (isnan(current[i])) {
            Serial.println("Error reading current");
        } else if (isnan(power[i])) {
            Serial.println("Error reading power");
        } else if (isnan(energy[i])) {
            Serial.println("Error reading energy");
        } else if (isnan(frequency[i])) {
            Serial.println("Error reading frequency");
        } else if (isnan(pf[i])) {
            Serial.println("Error reading power factor");
        } else {
            // Print the values to the Serial console
            Serial.print("Voltage: ");         Serial.print(voltage[i]);      Serial.println("V");
            Serial.print("Voltage L-L: ");     Serial.print(VoltLL[i]);       Serial.println("V");
            Serial.print("Voltage Everage: "); Serial.print(VoltEV);          Serial.println("V");
            Serial.print("Current: ");         Serial.print(current[i]);      Serial.println("A");
            Serial.print("Power: ");           Serial.print(power[i]);        Serial.println("W");
            Serial.print("Energy: ");          Serial.print(energy[i],3);     Serial.println("kWh");
            Serial.print("Frequency: ");       Serial.print(frequency[i], 1); Serial.println("Hz");
            Serial.print("PF: ");              Serial.println(pf[i]);
            Serial.println(voltage[0]);
            Serial.println(voltage[1]);
            Serial.println(voltage[2]);
            
            // lcd.setCursor(0,0);
            // lcd.print("PZEM   : "); lcd.print(i);
            // lcd.setCursor(0,1);
            // lcd.print("Volt   : "); lcd.print(VoltLL[i]); lcd.print(" V");
            // lcd.setCursor(0,2);
            // lcd.print("PerUnb : "); lcd.print(PerUnb); //lcd.print(" ''");
            delay(2000);

        }

        Serial.println("-------------------");
        Serial.println();
        delay(200);
    }
  }

void setup() {
  Serial.begin(115200);
    
  lcd.init();
  lcd.backlight();

  lcd.setCursor(7,1);
  lcd.print("WELCOME");
  delay(3000);
  lcd.clear();
 
// Initialize the PZEMs with Hardware Serial2 on RX/TX pins 16 and 17
  pzems[0] = PZEM004Tv30(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN, 0x01);
  pzems[1] = PZEM004Tv30(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN, 0x02);
  pzems[2] = PZEM004Tv30(PZEM_SERIAL, PZEM_RX_PIN, PZEM_TX_PIN, 0x03);
// }

  //timer.setInterval(2000L, checkBlynkStatus); // check if Blynk server is connected every 2 seconds

  Serial.println();
  Serial.print("ESP32-CAM IP Address: ");
  Serial.println(WiFi.localIP()); 

  pinMode(relay, OUTPUT);
}

void loop() {    
    ReadSenser();
}

