#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <stdio.h>
#include <string.h>
#include "SPIFFS.h"
#include <LiquidCrystal_I2C.h>

//set the lcd number of col n row
int lcdColumns =16;
int lcdRows =2;
LiquidCrystal_I2C lcd(0x3E, lcdColumns, lcdRows);

//define the pins used by the transceiver module
#define ss 5
#define rst 14
#define dio0 2
String data;
bool id=false;
String input;

/////// SPIFFS FUNCTIONS ////////////////////////////

void listDir(fs::FS &fs, const char * dirname, uint8_t levels);
void readFile(fs::FS &fs, const char * path);
bool find(fs::FS &fs, const char * path, const char * message);
void writeFile(fs::FS &fs, const char * path, const char * message);
void appendFile(fs::FS &fs, const char * path, const char * message);
void renameFile(fs::FS &fs, const char * path1, const char * path2);
void deleteFile(fs::FS &fs, const char * path);
void addadmin();
void addid();
void blockid();
void unblock();
void deleteid(fs::FS &fs, const char * path, const char * LoRaData);
void removeid();
void printlcd(int clr,int cursor,char print[16]);
void senddata(String data);
void admin();
void user();

//////////////////////////////////////////////////////

void setup() {
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();


  //initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Receiver");

  //setup LoRa transceiver module
  LoRa.setPins(ss, rst, dio0);
  
  //replace the LoRa.begin(---E-) argument with your location's frequency 
  //433E6 for Asia
  //866E6 for Europe
  //915E6 for North America
  while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
  }
   // Change sync word (0xF3) to match the receiver
  // The sync word assures you don't get LoRa messages from other LoRa transceivers
  // ranges from 0-0xFF
  LoRa.setSyncWord(0xF3);
  Serial.println("LoRa Initializing OK!");

  if(!SPIFFS.begin(true)){
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }
  writeFile(SPIFFS, "/admin.txt","99c8a16d");     ///// list of admin ids 
  writeFile(SPIFFS, "/ids.txt","99c8a16d");       ///// list of all the ids
  writeFile(SPIFFS, "/blocked.txt","");   ///// list of blocked ids
  writeFile(SPIFFS, "/deleted.txt","");   ///// list of deleted ids    
} 

void loop() {
   ///////////////////////////////////////////////  AUNTHENTICATION ///////////////////////////////////////////
   //senddata("this is master");
   while(true){         
      int packetSize = LoRa.parsePacket();
      if (packetSize) {
         while (LoRa.available()){
            input = LoRa.readString();                     
         }
         break;                           
      }
   }
   Serial.print(input);
   if (find(SPIFFS, "/admin.txt",input.c_str())){
      senddata("1");               
   }
   else if(find(SPIFFS, "/ids.txt",input.c_str())){ 
      senddata("2");
   }
   else{
      senddata("3");
      loop();
   }  

   ///////////////////////*********************************************//////////////////////////////////////////////////////
   while(true){         
      int packetSize = LoRa.parsePacket();
      if (packetSize) {
         while (LoRa.available()){
            input = LoRa.readString();                     
         }
         break;                           
      }
   }

////////////////////////////////////////////////////////////// USER INPUT  /////////////////////////////////////////////////////////////////////   
   if (input=="1"){
      admin();
   }
   else if(input == "2"){
      user();
   }   
   
   loop();
}

void admin(){
   while(true){         
      int packetSize = LoRa.parsePacket();
      if (packetSize) {
         while (LoRa.available()){
            input = LoRa.readString();
            //Serial.print(input);         
         }
         if (input != "1" && input!= "2" && input!= "3" && input!= "4" && input!= "5" && input!= "6"){
            Serial.println("Enter the right command");      
         }
         else{ 
            break;
         }                    
      }
   }
//////////////////////*********************************************************************************//////////////////////////////////////
   if (input == "1"){
      addadmin();
   }

   else if (input == "2"){ ///// Add New Id ///////////
      addid();
   }
   
   else if (input == "3"){  ///////// Block id ///////////
      blockid();
   }
   
   else if (input == "4"){  ///////// Unblock Id /////////
      unblock();
   }
   
   else if (input == "5"){  ///////// Erase Id ///////////
      removeid();
   }
   
   else if (input == "6"){      
      readFile(SPIFFS, "/ids.txt");
   }
   
   else if (input == "7"){
      readFile(SPIFFS, "/blocked.txt");
   }
}

void user(){

}

void addadmin(){
   Serial.println("Scan Id card that is to be made admin");           
   while(true){      
      String LoRaData = "";         
      int packetSize = LoRa.parsePacket();
      if (packetSize) {
         // received a packet
         Serial.print("Received packet '");
         
         //String id;
         // read packet
         while (LoRa.available()) {
            LoRaData = LoRa.readString();
            Serial.print(LoRaData);         
         }                  
            
         //print RSSI of packet
         Serial.print("' with RSSI ");
         Serial.println(LoRa.packetRssi());
         if (LoRaData!=""){
            if (find(SPIFFS, "/blocked.txt",LoRaData.c_str())){
               Serial.println("This Id is blocked");
               senddata("1");
               break;            
            }
            else if (find(SPIFFS, "/admin.txt",LoRaData.c_str())){
               Serial.println("Id already admin");
               senddata("2");
               break;            
            }
            else if (!find(SPIFFS, "/ids.txt",LoRaData.c_str())){
               Serial.println("First Add Id then make admin");
               senddata("3");
               break;            
            }
            else{               
               appendFile(SPIFFS, "/admin.txt",LoRaData.c_str());
               Serial.println("Id made admin");
               senddata("4");
               break;
            }
         }   
      }                
   }
}

void addid(){
   Serial.println("Scan Id card that is to be added");           
   while(true){         
      String LoRaData = "";         
      int packetSize = LoRa.parsePacket();
      if (packetSize) {
         // received a packet
         Serial.print("Received packet '");
         
         //String id;
         //read packet
         while (LoRa.available()) {
            LoRaData = LoRa.readString();
            Serial.print(LoRaData);         
         }                  
            
         //print RSSI of packet
         Serial.print("' with RSSI ");
         Serial.println(LoRa.packetRssi());
         if (LoRaData!=""){
            if (find(SPIFFS, "/blocked.txt",LoRaData.c_str())){
               Serial.println("This Id is blocked");
               senddata("1");
               break;            
            }
            else if (find(SPIFFS, "/ids.txt",LoRaData.c_str()) || find(SPIFFS, "/admin.txt",LoRaData.c_str())){
               Serial.println("Id already exists");
               senddata("2");
               break;
            }
            else{
               appendFile(SPIFFS, "/ids.txt",LoRaData.c_str());
               Serial.println("Id has been added");
               senddata("3");
               break;
            }
         }        
      }                           
   }
}

void blockid(){
   Serial.println("Scan Id card that is to be blocked");           
   while(true){      
      String LoRaData = "";         
      int packetSize = LoRa.parsePacket();
      if (packetSize) {
         // received a packet
         Serial.print("Received packet '");
         
         //String id;
         // read packet
         while (LoRa.available()) {
            LoRaData = LoRa.readString();
            Serial.print(LoRaData);         
         }                  
            
         //print RSSI of packet
         Serial.print("' with RSSI ");
         Serial.println(LoRa.packetRssi());

         if (LoRaData!=""){
            if (find(SPIFFS, "/blocked.txt",LoRaData.c_str())){
               Serial.println("This Id is already blocked");
               senddata("1");
               break;            
            }
            else if(!find(SPIFFS, "/ids.txt",LoRaData.c_str())){
               Serial.println("This Id does not exist");
               senddata("2");
               break; 
            }
            else{
               appendFile(SPIFFS, "/blocked.txt",LoRaData.c_str());
               deleteid(SPIFFS,"/ids.txt", LoRaData.c_str());
               if (find(SPIFFS, "/admin.txt",LoRaData.c_str())){
                  deleteid(SPIFFS,"/admin.txt", LoRaData.c_str());
               }
               Serial.println("Id has been blocked");
               senddata("3");
               break;
            }
         }
      }                   
   }
}

void unblock(){
   Serial.println("Scan Id card that is to be unblocked");           
   while(true){         
   String LoRaData = "";         
   int packetSize = LoRa.parsePacket();
   if (packetSize) {
      // received a packet
      Serial.print("Received packet '");
      
      //String id;
      // read packet
      while (LoRa.available()) {
         LoRaData = LoRa.readString();
         Serial.print(LoRaData);         
      }              
      //print RSSI of packet
      Serial.print("' with RSSI ");
      Serial.println(LoRa.packetRssi());
   }

   if (LoRaData!=""){
         if (!find(SPIFFS, "/blocked.txt",LoRaData.c_str())){
            Serial.println("This Id is not blocked");
            senddata("1");
            break;            
         }
         else{
            appendFile(SPIFFS, "/ids.txt",LoRaData.c_str());
            deleteid(SPIFFS,"/blocked.txt", LoRaData.c_str());
            Serial.println("Id has been unblocked");
            senddata("2");
            break;
         }
      }                   
   }
}

void removeid(){
   Serial.println("Scan Id card that is to be Removed");           
   while(true){         
   String LoRaData = "";         
   int packetSize = LoRa.parsePacket();
   if (packetSize) {
      // received a packet
      Serial.print("Received packet '");
      
      //String id;
      // read packet
      while (LoRa.available()) {
         LoRaData = LoRa.readString();
         Serial.print(LoRaData);         
      }              
      //print RSSI of packet
      Serial.print("' with RSSI ");
      Serial.println(LoRa.packetRssi());
   }

   if (LoRaData!=""){
         if (find(SPIFFS, "/blocked.txt",LoRaData.c_str())){
            Serial.println("This Id is blocked. Unblock to delete Id.");
            senddata("1");
            break;            
         }
         else if (!find(SPIFFS, "/ids.txt",LoRaData.c_str())){
            Serial.println("This Id does not exist.");
            senddata("2");
            break;            
         }
         else{
            if (find(SPIFFS, "/admin.txt",LoRaData.c_str())){
               deleteid(SPIFFS,"/admin.txt", LoRaData.c_str());
            }
            deleteid(SPIFFS,"/ids.txt", LoRaData.c_str());
            Serial.println("Id has been removed");
            senddata("3");
            break;
         }
      }                   
   }
}

void deleteid(fs::FS &fs, const char * path, const char * LoRaData){
   Serial.println("Scan Id card that is to be deleted");
   // printlcd(1,0,"Scan Id card");
   // printlcd(0,1,"to delete");           
       while(true){
         if (LoRaData!=""){
            if (path == "/blocked.txt"){
               if (!find(SPIFFS, "/blocked.txt",LoRaData)){
                  Serial.println("id is not blocked");
                  // printlcd(1,0,"id is not");
                  // printlcd(0,1,"blocked");
                  break;
               }
               else if (find(SPIFFS, "/blocked.txt",LoRaData)){
                  writeFile(SPIFFS, "/temp.txt","");
                  File file = fs.open(path);
                  while(file.available()){
                     String id="";
                     int count = 1;
                     while (count!=9){
                        char letter=file.read();
                        id=id+letter;         
                        count = count + 1;
                     }
                     
                     if (id!=LoRaData){
                        appendFile(SPIFFS, "/temp.txt",id.c_str());                        
                     }
                  }
                  deleteFile(SPIFFS, "/blocked.txt");
                  renameFile(SPIFFS, "/temp.txt", "/blocked.txt");
               }
            }
            if (path == "/ids.txt"){
               if (!find(SPIFFS, "/ids.txt",LoRaData)){
                  Serial.println("id to be deleted does not exist");
                  break;
               }
               else if (find(SPIFFS, "/ids.txt",LoRaData)){
                  writeFile(SPIFFS, "/temp.txt","");
                  File file = fs.open(path);
                  while(file.available()){
                     String id="";
                     int count = 1;
                     while (count!=9){
                        char letter=file.read();
                        id=id+letter;         
                        count = count + 1;
                     }
                     
                     if (id!=LoRaData){
                        appendFile(SPIFFS, "/temp.txt",id.c_str());                        
                     }
                  }
                  deleteFile(SPIFFS, "/ids.txt");
                  renameFile(SPIFFS, "/temp.txt", "/ids.txt");
                  break;
               }
            }                       
      }            
   }                   
}

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
   Serial.printf("Listing directory: %s\r\n", dirname);

   File root = fs.open(dirname);
   if(!root){
      Serial.println("- failed to open directory");
      return;
   }
   if(!root.isDirectory()){
      Serial.println(" - not a directory");
      return;
   }

   File file = root.openNextFile();
   while(file){
      if(file.isDirectory()){
         Serial.print("  DIR : ");
         Serial.println(file.name());
         if(levels){
            listDir(fs, file.name(), levels -1);
         }
      } else {
         Serial.print("  FILE: ");
         Serial.print(file.name());
         Serial.print("\tSIZE: ");
         Serial.println(file.size());
      }
      file = root.openNextFile();
   }
}

void readFile(fs::FS &fs, const char * path){
   Serial.printf("Reading file: %s\r\n", path);

   File file = fs.open(path);
   if(!file || file.isDirectory()){
       Serial.println("- failed to open file for reading");
       return;
   }

   //Serial.println("- read from file:");
   if (file.size() == 0){
      Serial.print("No ids in this list");
   }
   else{
      while(file.available()){
         int count = 1;
         while (count!=9){
            Serial.write(file.read());
            count = count+1;
         }
         Serial.print("\n");      
      }
   }   
}

void writeFile(fs::FS &fs, const char * path, const char * message){
   Serial.printf("Writing file: %s\r\n", path);

   File file = fs.open(path, FILE_WRITE);
   if(!file){
      Serial.println("- failed to open file for writing");
      return;
   }
   if(!file.print(message)){
      Serial.println("- write failed");
   }
}

void appendFile(fs::FS &fs, const char * path, const char * message){
   Serial.printf("Appending to file: %s\r\n", path);

   File file = fs.open(path, FILE_APPEND);
   if(!file){
      Serial.println("- failed to open file for appending");
      return;
   }
   if(file.print(message)){
      Serial.println("- message appended");
   } else {
      Serial.println("- append failed");
   }
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
   Serial.printf("Renaming file %s to %s\r\n", path1, path2);
   if (fs.rename(path1, path2)) {
      Serial.println("- file renamed");
   } else {
      Serial.println("- rename failed");
   }
}

void deleteFile(fs::FS &fs, const char * path){
   Serial.printf("Deleting file: %s\r\n", path);
   if(fs.remove(path)){
      Serial.println("- file deleted");
   } else {
      Serial.println("- delete failed");
   }
}

bool find(fs::FS &fs, const char * path, const char * message){

   Serial.printf("finding id: %s\r\n", message);

   File file = fs.open(path);
   if(!file || file.isDirectory()){
       Serial.println("- failed to open file for finding id");
       //return;
   }
    
   Serial.println("- read from file:");
   bool found=false;
   while(file.available()){
      String id="";
      int count = 1;
      while (count!=9){
         char letter=file.read();
         id=id+letter;         
         count = count + 1;
      }
      
      if (id==message){
         Serial.println("id found");
         found = true;
         break;
      }
      // Serial.write(file.read());
   }
   return found;
}

void printlcd(int clr,int cursor,char print[16]){
   if(clr==1){
      lcd.clear();
   }
   lcd.setCursor(0,cursor);
   lcd.print(print);   
}

void senddata(String data){
   LoRa.beginPacket();
   LoRa.print(data);
   LoRa.endPacket();
   Serial.println("data sent");
}