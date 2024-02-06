#include <Arduino.h>
#include <SPI.h>
#include <LoRa.h>
#include <stdio.h>
#include <string.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h> 

//set the lcd number of col n row
int lcdColumns =16;
int lcdRows =2;
LiquidCrystal_I2C lcd(0x3E, lcdColumns, lcdRows);

//define the pins used by the transceiver module
#define ss1 5
#define ss2 4
#define rst 14
#define dio0 2

int counter = 0;
String input;

MFRC522 rfid(ss2, rst); // Instance of the class

MFRC522::MIFARE_Key key; 

// Init array that will store new NUID 
byte nuidPICC[4];
String s = "";

/////////////////////////////////////////////////////// FUNCTIONS //////////////////////////////////////////////////////

void printHex(byte *buffer, byte bufferSize);
void printDec(byte *buffer, byte bufferSize);
String rfidread();
void printlcd(int clr,int cursor,char print[16]);
void senddata(String data);
void admin();
void user();
void addadmin();
void addid();
void blockid();
void unblock();
void removeid();
void emergency();
void staff();
void mantainance();

//////////////////////////*********************************************************/////////////////////////////////////

///////////////////////////////////////////////////// KEYPAD /////////////////////////////////////////////////////////////////////

#define ROW_NUM     4 // four rows
#define COLUMN_NUM  4 // four columns

char keys[ROW_NUM][COLUMN_NUM] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte pin_rows[ROW_NUM]      = {12, 18, 14, 17}; // GIOP12, GIOP18, GIOP14, GIOP17 connect to the row pins
byte pin_column[COLUMN_NUM] = {16, 27, 15, 13};   // GIOP16, GIOP27, GIOP15, GIOP13 connect to the column pins

//initialize an instance of class NewKeypad
Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM ); 

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void setup() {
  // initialize LCD
  lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();


  //initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial);
  Serial.println("LoRa Sender");
  // lcd.init();
  // lcd.backlight();
  //setup LoRa transceiver module
  LoRa.setPins(ss1, rst, dio0);
  
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

  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522 

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));
  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}

void loop() {
  // char customKey = keypad.getKey();
  // while(true){
  //  if (customKey){
  //   Serial.println("enter");
  //   LoRa.beginPacket();
  //   LoRa.print(customKey);
  //   LoRa.endPacket();
  //   Serial.println(customKey);
  //   break;
  //   }
  // }

  ////////////////////////////////////////// Verification //////////////////////////////////////////////////////////
  printlcd(1,0,"Scan Card");
  while (rfidread() ==""){}
  senddata(s);

  while(true){             
    int packetSize = LoRa.parsePacket();
    if (packetSize != 0) {
      while (LoRa.available()){        
        input = LoRa.readString();                 
      }
      if (input == "1"){
        Serial.println("Access Granted");
        printlcd(1,0,"Access Granted");
        break;      
      }
      else if (input == "2"){ 
        Serial.println("Access Granted");
        printlcd(1,0,"Access Granted");
        break;
      }
      else{
        Serial.println("Access Denied");
        printlcd(1,0,"Access Denied");
        delay(5000);
        break;
      }                    
    }
  }
  /////////////////////////////////////******************************************************//////////////////////////////////////////////
  if (input=="1"){
    delay(2000);
    Serial.println("Choose Access Type:");
    printlcd(1,0,"Choose ");
    printlcd(0,1,"Access Type");
    delay(2000);
    printlcd(1,0,"1. Admin");
    printlcd(0,1,"2. User");
    char input;

    while (true){
      if(Serial.available()){
        input = Serial.read();
        if (input != '1' && input != '2'){
          Serial.println("Enter the right Command");
          printlcd(1,0,"Enter the right");
          printlcd(0,1,"Command");
          delay(2000);
          printlcd(1,0,"1. Admin");
          printlcd(0,1,"2. User");
        }
        else{
          if (input == '1'){
            admin();
            break;
          }
          else if (input == '2'){
            user();
            break;
          }
        }
      }           
    }
  }
  
  else if (input=="2"){
    delay(2000);
    user();
  }

  loop();
}


void admin(){
  senddata("1");  
  printlcd(1,0,"choose what ");
  printlcd(0,1,"to do :");
  Serial.println("choose what to do:");
  Serial.println("1. Add Admin.");
  Serial.println("2. Add New Id.");
  Serial.println("3. Block Id");
  Serial.println("4. unblock Id");
  Serial.println("5. Erase Id");
  delay(1500);

  char input;

  while (true){
    while(!Serial.available()){
      
      printlcd(1,0,"1.Add Admin");
      printlcd(0,1,"2.Add New Id");
      delay(1500);
      printlcd(1,0,"3.Block Id ");
      printlcd(0,1,"4.Unblock Id");
      delay(1500);
      printlcd(1,0,"5.Erase Id"); 
      delay(1500);     
    }

    if(Serial.available()){
      input = Serial.read();
      if (input != '1' && input != '2' && input != '3' && input != '4' && input!= '5' && input != '6' && input != '7'){
        Serial.println("Enter the right command");      
      }
      else{
        if (input == '1'){ ///// Add admin ///////////
          addadmin();
          break;
        }

        else if (input == '2'){ ///// Add New Id ///////////
          addid();
          break;
        }
        
        else if (input == '3'){  ///////// Block id ///////////
          blockid();
          break;
        }
        
        else if (input == '4'){  ///////// Unblock Id /////////
          
          unblock();
          break;
        }
        
        else if (input == '5'){  ///////// Erase Id ///////////
          
          removeid();
          break;
        }
        
        else if (input == '6'){      
          senddata("6");
          //readFile(SPIFFS, "/ids.txt");
          break;
        }
        
        else if (input == '7'){
          senddata("7");
          //readFile(SPIFFS, "/blocked.txt");
          break;
        }    
        // break;
      }
    }    
  }
}

void user(){ 
  senddata("2"); 
  printlcd(1,0,"choose what ");
  printlcd(0,1,"to do :");
  Serial.println("choose what to do:");
  Serial.println("1. Emergency");
  Serial.println("2. Request Staff");
  delay(1500);
  printlcd(1,0,"1.Emergency");
  printlcd(0,1,"2.Request Staff");
  char input;

  while (true){
    if(Serial.available()){
      input = Serial.read();
      if (input != '1' && input != '2'){
        Serial.println("Enter the right command");      
      }
      else{
        if (input == '1'){ ///// Add admin ///////////
          emergency();
          break;
        }

        else if (input == '2'){ ///// Add New Id ///////////
          staff();
          break;
        }       
                
      }
    }    
  }  
}

void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void printDec(byte *buffer, byte bufferSize) {

  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}

String rfidread(){

  s = "";
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
  //if ( !
   rfid.PICC_IsNewCardPresent();
   //)
  //  return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial()){
    return s;
  }
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&  
    piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
    piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return s;
  } 

  // Store NUID into nuidPICC array
  for (byte i = 0; i < 4; i++) {
    nuidPICC[i] = rfid.uid.uidByte[i];
    s += String(nuidPICC[i],HEX);
  }

  Serial.println(s);
  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1(); 

  return s;
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

void addadmin(){
  senddata("1");
  Serial.println("Scan Id card that is to be made admin");
  printlcd(1,0,"Scan ID card");
  printlcd(0,1,"to make admin");
  while (rfidread() ==""){}
  senddata(s);

  while(true){             
    int packetSize = LoRa.parsePacket();
    if (packetSize != 0) {
      while (LoRa.available()){        
        input = LoRa.readString(); 
        Serial.println("data recieved");
        Serial.println(input);                
      }
      if (input == "1"){
        Serial.println("Id is blocked");
        printlcd(1,0,"Id is blocked");
        printlcd(0,1,"unblock first");
                     
      }
      else if (input == "2"){ 
        Serial.println("Id already admin");
        printlcd(1,0,"Id already admin");        
      }
      else if (input == "3"){
        Serial.println("First Add Id then make admin");
        printlcd(1,0,"Add user first");
        printlcd(0,1,"then make admin");
        
      }
      else{
        Serial.println("Id has been made admin");
        printlcd(1,0,"Id has been");
        printlcd(0,1,"made admin");        
      }
      delay(5000);
      break;                    
    }    
  }
}

void addid(){
  senddata("2");
  Serial.println("Scan Id card that is to be added");
  printlcd(1,0,"Scan ID card");
  printlcd(0,1,"to add");
  while (rfidread() ==""){}
  senddata(s);

  while(true){             
    int packetSize = LoRa.parsePacket();
    if (packetSize != 0) {
      while (LoRa.available()){        
        input = LoRa.readString(); 
        Serial.println("data recieved");
        Serial.println(input);                
      }
      if (input == "1"){
        Serial.println("Id is blocked");
        printlcd(1,0,"Id is blocked");
             
      }
      else if (input == "2"){ 
        Serial.println("Id already exists");
        printlcd(1,0,"Id already exists");        
      }
      else if (input == "3"){
        Serial.println("Id has been added");
        printlcd(1,0,"Id has been added");        
      }
      delay(5000);
      break;                    
    }    
  }
}

void blockid(){
  senddata("3");
  Serial.println("Scan Id card that is to be blocked");
  printlcd(1,0,"Scan ID card");
  printlcd(0,1,"to be block");
  while (rfidread() ==""){}
  senddata(s);

  while(true){             
    int packetSize = LoRa.parsePacket();
    if (packetSize != 0) {
      while (LoRa.available()){        
        input = LoRa.readString(); 
        Serial.println("data recieved");
        Serial.println(input);                
      }
      if (input == "1"){
        Serial.println("Id already blocked");
        printlcd(1,0,"Id already blocked");                     
      }
      else if (input == "2"){ 
        Serial.println("Id does not exist");
        printlcd(1,0,"Id does not exist");        
      }
      else{
        Serial.println("Id blocked");
        printlcd(1,0,"Id blocked");        
      }
      delay(5000);
      break;                    
    }    
  }
}

void unblock(){
  senddata("4");
  Serial.println("Scan Id card that is to be Unblocked");
  printlcd(1,0,"Scan ID card");
  printlcd(0,1,"to be unblocked");
  while (rfidread() ==""){}
  senddata(s);

  while(true){             
    int packetSize = LoRa.parsePacket();
    if (packetSize != 0) {
      while (LoRa.available()){        
        input = LoRa.readString(); 
        Serial.println("data recieved");
        Serial.println(input);                
      }
      if (input == "1"){
        Serial.println("Id is not blocked");
        printlcd(1,0,"Id is not");
        printlcd(0,1,"blocked");                     
      }
      else{
        Serial.println("Id unblocked");
        printlcd(1,0,"Id unblocked");        
      }
      delay(5000);
      break;                    
    }    
  }   
}

void removeid(){
  senddata("5");
  Serial.println("Scan Id card that is to be removed");
  printlcd(1,0,"Scan ID card");
  printlcd(0,1,"to be removed");
  while (rfidread() ==""){}
  senddata(s);

  while(true){             
    int packetSize = LoRa.parsePacket();
    if (packetSize != 0) {
      while (LoRa.available()){        
        input = LoRa.readString(); 
        Serial.println("data recieved");
        Serial.println(input);                
      }
      if (input == "1"){
        Serial.println("Id is blocked");
        printlcd(1,0,"Id is blocked");
        printlcd(0,1,"unblock to remove");                     
      }
      else if (input == "2"){
        Serial.println("This Id does not exist.");
        printlcd(1,0,"This Id does");
        printlcd(0,1,"not exist");
      }
      else{
        Serial.println("Id removed");
        printlcd(1,0,"Id removed");        
      }
      delay(5000);
      break;                    
    }    
  }   
}

void emergency(){
  Serial.println("Choose Emergency");
  Serial.println("1. Fire Alarm");
  Serial.println("2. Medical Emergency");
  printlcd(1,0,"Choose Emergency");
  printlcd(0,1,"1.Fire 2.Medical");

  char input;

  while (true){
    if(Serial.available()){
      input = Serial.read();
      if (input != '1' && input != '2'){
        Serial.println("Enter the right command");      
      }
      else{
        if (input == '1'){ ///// fire ////////////////
          printlcd(1,0,"FIRE ALARM!!!!");
          printlcd(0,1,"Evacuate !!!");
          delay(2000);
          break;
        }

        else if (input == '2'){ ///// Add New Id ///////////
          printlcd(1,0,"Doctor is on");
          printlcd(0,1,"his way !!");
          delay(2000);
          break;
        }        
      }
    }    
  }
}

void staff(){
  Serial.println("Choose Staff");
  Serial.println("1. Electrician");
  Serial.println("2. plumber");
  Serial.println("3. Sweeper");
  Serial.println("4. Technician");
  printlcd(1,0,"Choose Staff");
  delay(1500);
  

  char input;

  while(!Serial.available()){
      
      printlcd(1,0,"1.Electrician");
      printlcd(0,1,"2.Plumber");
      delay(1500);
      printlcd(1,0,"3.Sweeper");
      printlcd(0,1,"4.Technician");
      delay(1500);
    
    }

  while (true){
    if(Serial.available()){
      input = Serial.read();
      if (input != '1' && input != '2' && input != '3' && input != '4'){
        Serial.println("Enter the right command");      
      }
      else{
        if (input == '1'){ ///// fire ////////////////
          printlcd(1,0,"Electrician");
          printlcd(0,1,"on his way");
          delay(2000);
          break;
        }

        else if (input == '2'){ ///// Add New Id ///////////
          printlcd(1,0,"Plumber");
          printlcd(0,1,"on his way");
          delay(2000);
          break;
        }

        else if (input == '3'){ ///// Add New Id ///////////
          printlcd(1,0,"Sweeper");
          printlcd(0,1,"on his way");
          delay(2000);
          break;
        }  

        else if (input == '4'){ ///// Add New Id ///////////
          printlcd(1,0,"Technician");
          printlcd(0,1,"on his way");
          delay(2000);
          break;
        }      
      }
    }    
  }
}
