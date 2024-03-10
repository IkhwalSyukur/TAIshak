#include <Arduino.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#define SD_CS 5

void appendFile(fs::FS &fs, const char * path, const char * message){
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

void writeFile(fs::FS &fs, const char * path, const char * message){
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

void setup(){
  Serial.begin(115200);
  SD.begin(SD_CS);
  if (!SD.begin(SD_CS)) {
    Serial.println("Gagal Memuat Kartu SD");
    return;
  }
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    Serial.println("Tidak Ada Kartu SD");
    return;
  }
  Serial.println("Menginisialisasi kartu SD...");
  
  writeFile(SD, "/Data.txt", "Hai, Ini Robotik Indonesia\n ");
//   appendFile(SD, "/Data.txt", "Kita akan belajar bersama ");
  /*
  Menyimpan Tulisan atau String "Hai, Ini Robotik Indonesia "
  pada file yang bernama RobotikID.txt
  */ 
}

void loop(){
}