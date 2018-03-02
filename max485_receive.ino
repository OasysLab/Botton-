/*-----( Import needed libraries )-----*/
#include <SoftwareSerial.h>
#include <TM1637Display.h>

// Module connection pins (Digital Pins)
#define CLK 5
#define DIO 4

/*-----( Declare Constants and Pin Numbers )-----*/
#define SSerialRX        10  //Serial Receive pin
#define SSerialTX        11  //Serial Transmit pin

#define SSerialTxControl 3   //RS485 Direction control
#define RS485Transmit    HIGH
#define RS485Receive     LOW

#define Pin13LED         13

/*-----( Declare objects )-----*/
SoftwareSerial RS485Serial(SSerialRX, SSerialTX); // RX, TX

/*-----( Declare Variables )-----*/
int byteReceived;
int byteSend;

const int buttonPinAccept = 7;    // the number of the pushbutton pin
int buttonState;             // the current reading from the input pin

const int buttonPinReject = 8;    // the number of the pushbutton pin
int buttonState2;             // the current reading from the input pin

TM1637Display display(CLK, DIO);

unsigned long currentMillis;
unsigned long previousMillis = 0;

void setup()   /****** SETUP: RUNS ONCE ******/
{
  // Start the built-in serial port, probably to Serial Monitor
  Serial.begin(9600);
  Serial.println("SerialRemote");  // Can be ignored

  pinMode(buttonPinAccept, INPUT);
  pinMode(Pin13LED, OUTPUT);
  pinMode(SSerialTxControl, OUTPUT);

  digitalWrite(SSerialTxControl, RS485Receive);  // Init Transceiver

  // Start the software serial port, to another device
  RS485Serial.begin(9600);   // set the data rate
  display.setBrightness(0x0f);
display.showNumberDec(10);
  digitalWrite(SSerialTxControl, RS485Transmit);
  RS485Serial.write("Init=2E");
  delay(10);
  digitalWrite(SSerialTxControl, RS485Receive);
}


void loop()
{
  int reading = digitalRead(buttonPinAccept);

  if (reading == HIGH) {
    buttonState = HIGH;
  }
  else {
    if (buttonState == HIGH) {
      buttonState = LOW;
      digitalWrite(SSerialTxControl, RS485Transmit);
      RS485Serial.write("Update=2E");
      Serial.println("beep");
      delay(10);
      digitalWrite(SSerialTxControl, RS485Receive);
    }
  }

  int reading2 = digitalRead(buttonPinReject);

  if (reading2 == HIGH) {
    buttonState2 = HIGH;
  }
  else {
    if (buttonState2 == HIGH) {
      buttonState2 = LOW;
      digitalWrite(SSerialTxControl, RS485Transmit);
      RS485Serial.write("Reject=2E");
      Serial.println("beep");
      delay(10);
      digitalWrite(SSerialTxControl, RS485Receive);
    }
  }

  if (RS485Serial.available()) { //Look for data from other Arduino
    digitalWrite(Pin13LED, HIGH);  // Show activity
    String tmp = RS485Serial.readStringUntil('E');    // Read received byte
    Serial.println(tmp);        // Show on Serial Monitor
    delay(10);
    digitalWrite(Pin13LED, LOW);  // Show activity
    if (tmp.startsWith("data="))  {
      Serial.println('a');
      display.showNumberDec(tmp.substring(tmp.indexOf('=') + 1).toInt());
    }
    if (tmp.startsWith("Reject="))  {
      Serial.println('b');
      display.showNumberDec(tmp.substring(tmp.indexOf('=') + 1).toInt());
      previousMillis = millis();
    }
  }

  currentMillis = millis();
  if (currentMillis - previousMillis > 10000 && previousMillis != 0) {
    previousMillis = 0;
    digitalWrite(SSerialTxControl, RS485Transmit);
    RS485Serial.write("Get=2E");
    Serial.println("beep");
    delay(10);
    digitalWrite(SSerialTxControl, RS485Receive);    
  }
}
