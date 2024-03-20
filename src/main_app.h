 
#include <PZEM004Tv30.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <LiquidCrystal_I2C.h>
#include "algorithm"
#include "ThingSpeak.h"
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include "RTClib.h"

#define SD_CS 5

#define MY_MAX(a, b, c) max(max(a, b), c)
#define relay 13

#define PZEM_RX_PIN 16 
#define PZEM_TX_PIN 17 

#define NUM_PZEMS 3

LiquidCrystal_I2C lcd(0x27, 16, 2);


const char* ssid = "ISHAK";   // your network SSID (name) 
const char* password = "87654321";   // your network password

RTC_DS3231 rtc;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};


WiFiClient  client;

unsigned long myChannelNumber = 1;
const char * myWriteAPIKey = "0VFY1QZOICQYV9AR";

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 20000;

int button;

PZEM004Tv30 pzems[NUM_PZEMS];

#define PZEM_SERIAL Serial2

int voltage[3];
float current[3];
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
String StatusL;
String linestring;
float linetoline;

void appendFile(fs::FS &fs, const char * path, String message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, float message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}

void ReadSenser(){
      // Print out the measured values from each PZEM module
    for(int i = 0; i < NUM_PZEMS; i++){
        DateTime now = rtc.now();
        // Print the Address of the PZEM
        Serial.print("PZEM ");
        Serial.print(i);
        Serial.print(" - Address:");
        Serial.println(pzems[i].getAddress(), HEX);
        Serial.println("===================");

        // Read the data from the sensor
        voltage[i] = pzems[i].voltage();
        float tegangan = pzems[i].voltage();
        String teganganstring = String(tegangan);
        current[i] = pzems[i].current();
        float arus = pzems[i].current();
        String stringarus = String(arus);
        power[i] = pzems[i].power();
        energy[i] = pzems[i].energy();
        frequency[i] = pzems[i].frequency();
        pf[i] = pzems[i].pf();
        VoltLL[i] = voltage[i]*1.73;
        // float linetoline = voltage[i]*1.73;
        // String linestring = String(linetoline);
        VoltMAX = MY_MAX(VoltLL[0], VoltLL[1], VoltLL[2]);
        VoltEV = ((VoltLL[0] + VoltLL[1] + VoltLL[2])/3);
        VoltDEF = VoltMAX - VoltEV;
        PerUnb = ((100*VoltDEF)/VoltEV);
        String statusi = String(i);

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
            Serial.print("Current: ");         Serial.print(current[i],4);    Serial.println("A");
            Serial.print("Power: ");           Serial.print(power[i]);        Serial.println("W");
            Serial.print("Energy: ");          Serial.print(energy[i],3);     Serial.println("kWh");
            Serial.print("Frequency: ");       Serial.print(frequency[i], 1); Serial.println("Hz");
            Serial.print("PF: ");              Serial.println(pf[i]);
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("V= ");
            lcd.setCursor(3,0);
            lcd.print(VoltLL[i]);
            lcd.setCursor(0,1);
            lcd.print("A= ");
            lcd.setCursor(3,1);
            lcd.print(current[i],4);
            // Serial.println(voltage[0]);
            // Serial.println(voltage[1]);
            // Serial.println(voltage[2]);
            
            if(VoltEV>390){
            digitalWrite(relay, LOW);
            }
            else{
            digitalWrite(relay,HIGH);
            }
            

            String tanggal = String(now.year()) + "/" + String(now.month()) + "/" + String(now.day()) + "/" + daysOfTheWeek[now.dayOfTheWeek()] + "," + String(now.hour()) + ":" + String(now.minute()) + ":" + String (now.second());
            if (statusi == "0"){
                StatusL = "RS";
                linetoline = voltage[i]*1.73 - 7;
                linestring = String(linetoline);
            }
            else if(statusi == "1"){
                StatusL = "RT";
                linetoline = voltage[i]*1.73 - 7;
                linestring = String(linetoline);
            }
            else{
                StatusL = "ST";
                linetoline = voltage[i]*1.73 + 12;
                linestring = String(linetoline);
            }
            appendFile(SD, "/Data.txt", tanggal + "= ");
            appendFile(SD, "/Data.txt", "Line = " + StatusL + " tegangan = " + linestring + "\n");
            appendFile(SD, "/Data.txt", "Arus = " +  stringarus + "\n");
            Serial.printf("line = %s", StatusL);
            Serial.println(linestring);
            // Serial.printf("PernUB = %f\n", PerUnb);
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

        if ((millis() - lastTime) > timerDelay) {
    
        // Connect or reconnect to WiFi
        if(WiFi.status() != WL_CONNECTED){
        Serial.print("Attempting to connect");
        while(WiFi.status() != WL_CONNECTED){
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Conecting...");
            WiFi.begin(ssid, password); 
            delay(5000);     
        } 
        Serial.println("\nConnected.");
        }
        int x = ThingSpeak.writeField(myChannelNumber, 1, linetoline, myWriteAPIKey);
        int y = ThingSpeak.writeField(myChannelNumber, 2, arus, myWriteAPIKey);
        int a = ThingSpeak.writeField(myChannelNumber, 3, linetoline, myWriteAPIKey);
        int b = ThingSpeak.writeField(myChannelNumber, 4, arus, myWriteAPIKey);
        if(x == 200){
        Serial.println("Channel update successful.");
        }
        else{
        Serial.println("Problem updating channel. HTTP error code " + String(x));
        }
        lastTime = millis();
        } 
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

    SD.begin(SD_CS);
    if (!SD.begin(SD_CS)) {
        Serial.println("Gagal Memuat Kartu SD");
        return;
    }
    uint8_t cardType = SD.cardType();
    if (cardType == CARD_NONE) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("No SD Card");
        Serial.println("Tidak Ada Kartu SD");
        return;
    }
    Serial.println("Menginisialisasi kartu SD...");
 
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

    WiFi.mode(WIFI_STA);   
  ThingSpeak.begin(client);  // Initialize ThingSpeak

    #ifndef ESP8266
  while (!Serial); // wait for serial port to connect. Needed for native USB
#endif

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  // When time needs to be re-set on a previously configured device, the
  // following line sets the RTC to the date & time this sketch was compiled
  // rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));


}

void loop() {   
    
    ReadSenser();
}

