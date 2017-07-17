#include "SoftwareSerial.h"
#include <IRremote.h>
//irb led :Pin 3
const int RECV_PIN = 11;
IRrecv irrecv(RECV_PIN);
IRsend irsend;
decode_results results;
SoftwareSerial bluetooth(9, 10);
String command = "";
String buttonId = "";        // During Tranmit Mode this the HEX value of the Signal
String brand = "";
bool blueSig = false;
const int ledPin =  6; 

void setup() {
  // put your setup code here, to run once:
  bluetooth.begin(38400);
  Serial.begin(9600);

  irrecv.enableIRIn(); // Start the receiver
  irrecv.blink13(true);
  pinMode(ledPin, OUTPUT);    // Alert LED initialization
  blinkTransmit();
  
}
void loop() {
  // put your main code here, to run repeatedly:

  
  int bits=32;
  int splitIndex1 = 0;
  int splitIndex2 = 0;
  int splitIndex3 = 0;
  if(bluetooth.available()){                          // Decode the command if Bluetooth is available


     String bluetoothString = bluetooth.readString();
     Serial.println("Command Recieved : " + bluetoothString); 
     if(bluetoothString.length() >50){
     
               bluetooth.println("+ERR;5;ERR;5;ERR^");
               Serial.println("+ERR;5;ERR;5;ERR^");
               blueSig = false;
               blinkError();
     }   
        for(int i = 0 ; i < bluetoothString.length()-1; i ++){
        if (bluetoothString.substring(i, i+1) == ";"){
          splitIndex1 = i;
          break;
          }
        }
        for(int i = splitIndex1 + 1; i < bluetoothString.length() ; i ++){
          if (bluetoothString.substring(i, i+1) == ";"){
            splitIndex2 = i;
          break;
          }
        }
        for(int i = splitIndex2 + 1; i < bluetoothString.length() ; i ++){
        if (bluetoothString.substring(i, i+1) == ";"){
          splitIndex3 = i;
        break;
          }
        }
      command = bluetoothString.substring(0, splitIndex1);
      buttonId = bluetoothString.substring(splitIndex1+1, splitIndex2);
      brand = bluetoothString.substring(splitIndex2+1,splitIndex3);
      bits = bluetoothString.substring(splitIndex3+1).toInt();
      
      Serial.print("buttonId :"+buttonId);
      Serial.print("   , Brand  : " + brand);
      Serial.print("  , bits :");
      Serial.println(bits);
      
      
      blueSig = true;
         
  }

  if(command == "LEARN"){                  // Inside Learning Mode
    
     if (irrecv.decode(&results)) {
       
          command = "";
                    
          String hexValue = String(results.value,DEC);
          Serial.print("hexValue In Raw :" + results.value);
          Serial.print("hexValue :");
          Serial.print(hexValue+" ");
          Serial.println(results.bits);
          
          
          if (brand == "NEC" || brand == "SONY" || brand == "RC5" || brand == "RC6" || brand == "SAMSUNG"){
            bluetooth.println("+LEARN;"+buttonId+";"+hexValue+";"+String(results.bits)+";"+brand+"^");
            Serial.println("String Sent to Phone  :  +LEARN;"+buttonId+";"+hexValue+";"+String(results.bits)+";"+brand+"^");
            blinkLearn();
          }
          else if (brand == "UNKNOWN"){
            String data = "";
            for (int i = 1; i<results.rawlen-1;i++){
              data+=String(results.rawbuf[i]*50);
              data+=",";
            }
            data+=String(results.rawbuf[results.rawlen-1]*50);
            bluetooth.println("+LEARN;"+buttonId+";"+data+";"+String(results.rawlen)+";"+brand+"^");
            Serial.println("+LEARN;"+buttonId+";"+data+";"+String(results.rawlen)+";"+brand+"^");
            blinkLearn(); 
          }
          else{
            bluetooth.println("+ERR;5;ERR;5;ERR^");
            Serial.println("+ERR;5;ERR;5;ERR^");
            blueSig = false;
            blinkError();
          }
          
          irrecv.resume();
          
      }  
      blueSig = false;      
  }
  
  else if(command == "TRANS"){                              // Device In the Transmission Mode
    //Serial.println("TRANS Mode");
      command = "";
      blueSig = false;
      Serial.println("Recieved the TRANS to be sent");    
      unsigned long intButtonId = buttonId.toInt();
    
      if (brand == "NEC") {
        Serial.println("NEC to be sent");
        irsend.sendNEC(intButtonId, bits);   
        Serial.println("IR successfully sent");
        blinkTransmit();
        
      } else if (brand == "SONY") {
        Serial.println("SONY to be sent");
        irsend.sendSony(intButtonId, bits);  
        Serial.println("IR successfully sent");
        blinkTransmit();
        
      } else if (brand == "RC5") {
        Serial.println("RC5 to be sent");
        irsend.sendRC5(intButtonId, bits);  
        Serial.println("IR successfully sent");

      } else if (brand == "RC6") {
        Serial.println("RC6 to be sent");
        irsend.sendRC6(intButtonId, bits);
        Serial.println("IR successfully sent");
        blinkTransmit();
        
      } 
        else if (brand == "SAMSUNG") {
        Serial.println("Sending SAMSUNG");
        Serial.println("BUtton ID"+buttonId);   
        Serial.println(intButtonId);
        irsend.sendSAMSUNG(intButtonId, 32);
        Serial.println("IR successfully sent");
        blinkTransmit();
        
      }else if (brand == "UNKNOWN") {
        Serial.println("UNKNOWN to be sent");
        int count = 0;
        unsigned int sendData[bits];
        for (int i=0;i<buttonId.length();i++){
          if(buttonId.substring(i,i+1)==","){
            sendData[count] = buttonId.substring(count,i).toInt();
            count++;
          }
          sendData[count] = buttonId.substring(i).toInt();
        }
        for (int i=0;i<bits;i++){
           Serial.println(sendData[i]);
        }
        irsend.sendRaw(sendData,bits,38);
        Serial.println("IR successfully sent");
        blinkTransmit();
      }
      else{
          bluetooth.println("+ERR;5;ERR;5;ERR^");
          Serial.println("+ERR;5;ERR;5;ERR^");
          blueSig = false;
          blinkError();  
        
      }
      
  }
  else if(blueSig){                                                       //Signal Recieved with Error Charactor
      bluetooth.println("+ERR;5;ERR;5;ERR^");
      Serial.println("+ERR;5;ERR;5;ERR^");
      blueSig = false;
      blinkError();
  }
}

void blinkError(){
  
   digitalWrite(ledPin, HIGH);
   delay(250);
   digitalWrite(ledPin, LOW);
   delay(250);
   digitalWrite(ledPin, HIGH);
   delay(250);
   digitalWrite(ledPin, LOW);
   delay(250);
   digitalWrite(ledPin, HIGH);
   delay(250);
   digitalWrite(ledPin, LOW);
   delay(250);
   digitalWrite(ledPin, HIGH);
   delay(250);
   digitalWrite(ledPin, LOW);
   delay(250);
   digitalWrite(ledPin, HIGH);
   delay(250);
   digitalWrite(ledPin, LOW);
   delay(250);
   digitalWrite(ledPin, HIGH);
   delay(250);
   digitalWrite(ledPin, LOW);
   delay(250);
   digitalWrite(ledPin, HIGH);
   delay(250);
   digitalWrite(ledPin, LOW);
   delay(250);
   digitalWrite(ledPin, HIGH);
   delay(250);
   digitalWrite(ledPin, LOW);
   delay(250);
  
}

void blinkTransmit(){

   digitalWrite(ledPin, HIGH);
   delay(500);
   digitalWrite(ledPin, LOW);
   delay(500);
   digitalWrite(ledPin, HIGH);
   delay(500);
   digitalWrite(ledPin, LOW);
   delay(500);  
}

void blinkLearn(){

   digitalWrite(ledPin, HIGH);
   delay(500);
   digitalWrite(ledPin, LOW);
   delay(500);
   digitalWrite(ledPin, HIGH);
   delay(500);
   digitalWrite(ledPin, LOW);
   delay(500); 
      digitalWrite(ledPin, HIGH);
   delay(250);
   digitalWrite(ledPin, LOW);
   delay(250);
   digitalWrite(ledPin, HIGH);
   delay(250);
   digitalWrite(ledPin, LOW);
   delay(250);
   
}
