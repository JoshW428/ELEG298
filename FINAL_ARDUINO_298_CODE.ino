/* MyProgram:   
   Author: Donald Duck
   Revision: 3/7/17 (ver 0.1)
   Description: This program controls an LED (pin 2) and a Digital Potentiometer (AD525BRUZ1-RL7) via I2C.
                Pin 2 goes to a 180 ohm resistor, then to the Digital Pot, then to the Anode of the LED, then to Ground.
*/

#include <EEPROM.h>
#include <Wire.h>

/* Output pin for LED */
int MyLED = 2;           // Note: pin 13 goes to the on-board LED
int output = 6;
int tonechange1 = A2;
int tonechange2 = A0;
int delaychange = A1;
int drum = A5;

/* Number of times Arduino has been booted */
int NumCycles = 0;

/* Variable used in serial communication */
char InBuff[10];
int BuffIndex = 0;
char EOL = 13;
bool ProcessCommand = false;
int Sum = 0;
int Value = 0;
float voltage = 0;
int resistance = 0;
int sound1 = 0;
int sound2 = 0;
int delays = 0;
bool base = 0;

/* ===================================  Start of main section ==========================================*/
void setup() {
  // initialize the serial port
  Serial.begin(115200);
    
  // Initialize the Relay output pins
  pinMode(MyLED,OUTPUT);
  pinMode(output, OUTPUT);
  pinMode(tonechange1, INPUT);
  pinMode(tonechange2, INPUT);
  pinMode(delaychange, INPUT);
  pinMode(drum, INPUT);
  digitalWrite(MyLED,LOW);
  base = analogRead(drum);

  delay(10);
  Serial.flush();
  delay(10);

  // increment the Number of power-on cycles by one
  EEPROM.get(0, NumCycles);
  NumCycles = NumCycles + 1;
  EEPROM.put(0,NumCycles);

  // Start-up the I2C bus (address is optionla for master)
  Wire.begin();
}


void loop() {
  //CheckforSerialData();
  //ProcessCommands();
  delay(10);
  delays = analogRead(delaychange)/2;
  delay(10);
  Serial.println(analogRead(drum));
  delay(10);
  if(analogRead(drum) > 800){
    tone(output, 1450);
    delay(100);
    tone(output, 0);
  }
  delay(10);
  sound1 = analogRead(tonechange1);
  tone(output, sound1);
  delay(delays);
  sound2 = analogRead(tonechange2);
  tone(output, sound2);
  delay(10);
}
/* ===================================  End of main section ==========================================*/



/* ----------------------------- Check for Serial Data ---------------------------- */
void CheckforSerialData() {
  // See if there is a character ready from the serial port
  if (Serial.available()>0)  {
    InBuff[BuffIndex] = byte(Serial.read());
     if (InBuff[BuffIndex] == EOL)  {
      BuffIndex = 0;
      ProcessCommand = true;
      delay(1);
      Serial.flush();
    } else {
      BuffIndex = BuffIndex + 1;
      if (BuffIndex > 9) BuffIndex = 0;   // Reset the input buffer (This should never happen!!!)
      ProcessCommand = false;
    }
  }  
}
/* ----------------------------- Check for Serial Data ---------------------------- */
  
/* ----------------------------- Process Commands --------------------------------- */
void ProcessCommands() {
  if (ProcessCommand)  {
    ProcessCommand = false;
    switch (InBuff[0]) {

      case 'F': case 'f':   // Flush the serial buffer (This is essentially a "do-nothing" command).
        Serial.flush();
        delay(10);
        break;
        
      case 'I': case 'i':     // Information
        // Manufacture,Model,SerialNumber,FirmwareRevision
        Serial.println("Arduino,ControlBoard,0,0.1");
        break;

      case 'L': case 'l':     // Set the LED; usage: L1 for LED ON, L0 for LED OFF, LT for LED Togle, L? to inquire state
        if (InBuff[1] == '?')  {
          Serial.println(digitalRead(MyLED));
        } else {
          if (InBuff[1] == '1') {
            digitalWrite(MyLED,HIGH);
          } else if (InBuff[1] == 'T') {  
            bool TestBit = true;
            bool NewBit = TestBit ^ digitalRead(MyLED);
            digitalWrite(MyLED,NewBit);  
          } else {
            digitalWrite(MyLED,LOW);            
          }
          Serial.println("OK");      
        }
        break;

      case 'N': case 'n':   // Read the number of power-on cycles stored in EEprom (Use "N0" to reset)
        if (InBuff[1] == '0') {
            NumCycles = 0;
            EEPROM.put(0,NumCycles);
            Serial.println("OK");
        } else {
          Serial.println(NumCycles);          
        }
        break;

      case 'P': case 'p':   // Set the Digital Pot (AD5252BRUZ1-RL7)
        Wire.beginTransmission(44); // transmit to device #44 (0x2c, assuming AD0 and AD1 are grounded)
        Wire.write(byte(0x01));       // sends instruction byte to write Pot value into RDAC1 (use 0x03 for RDAC3)
        Sum = ((InBuff[1]-48)*100)+((InBuff[2]-48)*10)+(InBuff[3]-48);
        if(Sum > 255){
          Sum = 255;
        }
        Wire.write(Sum);
        Wire.endTransmission();       // stop transmitting
        Serial.println(Sum);
        delay(1000);
        Value = analogRead(A0);
        voltage = (Value)*(5.0/1023.0);
        Serial.println("Voltage:");
        Serial.println(voltage);
        break;

        case 'Q': case 'q':
        Serial.println("Loop Test mode");
        for(int i = 0; i < 256 ; i++){
         Wire.beginTransmission(44); // transmit to device #44 (0x2c, assuming AD0 and AD1 are grounded)
         Wire.write(byte(0x01));  
         Wire.write(i);
         Wire.endTransmission(); 
         delay(100);//delay 50 ms
         Value = analogRead(A0);
         voltage = (Value)*(5.0/1023.0);
         resistance = map(i, 0, 255, 0, 1000);
         Serial.println(voltage);
        }
        Wire.endTransmission();
        break;


        
      default:    //  If a non-recognizable command occurs, output the command back to the Serial port
        bool EndofCommand = false;
        Serial.print(":");
        for (int i=0; i <= 9; i++) {
          if (InBuff[i] == EOL) EndofCommand = true;
          if (!EndofCommand)  {
            Serial.print(InBuff[i]);
          }
        }
        Serial.println(":");        
        break;
    }
  }
}   
/* ----------------------------- Process Commands --------------------------------- */




