#include "SoftwareSerial.h"
#include <string.h>
#include <sstream.h>
#include <IRremote.h>

const int RECV_PIN = 11;

IRrecv irrecv(RECV_PIN);
IRsend irsend;

decode_results results;

SoftwareSerial bluetooth(9, 10);
String command = "";
String buttonId = "";        // During Tranmit Mode this the HEX value of the Signal
String brand = "";

void setup() {
  // put your setup code here, to run once:
  bluetooth.begin(38400);
  Serial.begin(9600);

  irrecv.enableIRIn(); // Start the receiver
  irrecv.blink13(true);
  
}
void loop() {
  // put your main code here, to run repeatedly:

  
  int bits;
  int splitIndex1 = 0;
  int splitIndex2 = 0;
  int splitIndex3 = 0;
  if(bluetooth.available()){                          // Decode the command if Bluetooth is available


     String bluetoothString = bluetooth.readString();
     Serial.println("Command Recieved : " + bluetoothString); 
     
     
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
      
      Serial.print("buttonId :");
      Serial.print(buttonId + ",");
      Serial.print("bits :");
      Serial.println(bits);
         
  }

  if(command == "LEARN"){
    //Serial.println("LEARN Mode");
     if (irrecv.decode(&results)) {
    
          String hexValue = String(results.value,HEX);
          Serial.print("hexValue In Raw :" + results.value);
          Serial.print("hexValue :");
          Serial.print(hexValue+" ");
          Serial.println(results.bits);
          
          
          if (brand == "NEC" || brand == "SONY" || brand == "RC5" || brand == "RC6" || brand == "SAMSUNG"){
            bluetooth.println("+LEARN;"+buttonId+";"+hexValue+";"+String(results.bits)+"^");
          }
          else if (brand == "UNKNOWN"){
            String data = "";
            for (int i = 1; i<results.rawlen-1;i++){
              data+=String(results.rawbuf[i]*50);
              data+=",";
            }
            data+=String(results.rawbuf[results.rawlen-1]*50);
            bluetooth.println("+LEARN;"+buttonId+";"+data+";"+String(results.bits)+"^");
            Serial.println("data :" + data);
            
          }
          command = "";
          
          irrecv.resume();
         
      }  
  }
  
  else if(command == "TRANS"){
    //Serial.println("TRANS Mode");
      Serial.println("Recieved the command to be sent");    
    
      if (brand == "NEC") {
        Serial.println("NEC to be sent");
        irsend.sendNEC(buttonId.toInt(), bits);   
        Serial.println("IR successfully sent");
        
      } else if (brand == "SONY") {
        Serial.println("SONY to be sent");
        irsend.sendSony(buttonId.toInt(), bits);  
        Serial.println("IR successfully sent");
        
      } else if (brand == "RC5") {
        Serial.println("RC5 to be sent");
        irsend.sendRC5(buttonId.toInt(), bits);  
        Serial.println("IR successfully sent");

      } else if (brand == "RC6") {
        Serial.println("RC6 to be sent");
        irsend.sendRC6(buttonId.toInt(), bits);
        Serial.println("IR successfully sent");
        
      } 
        else if (brand == "SAMSUNG") {
        Serial.println("SAMSUNG to be sent");
        Serial.println("SAMSUNG_buttonID :"+("0x"+buttonId).toInt());
        Serial.println("SAMSUNG_buttonID :"+ (buttonId,HEX));
        irsend.sendSAMSUNG(buttonId.toInt(), bits);
        
        Serial.println("IR successfully sent");
        
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
        
      }
      command = "";
  }
  else{
      bluetooth.println("+ERR;5;ERR;5^");
      Serial.println("+ERR;5;ERR;5^");
  }
}
