/*
   ESP8266 (WeMosD1) WiFi Relay Control

   learnelectronics
   05 JUN 2017

   www.youtube.com/c/learnelectronics
   arduino0169@gmail.com
*/






#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Arduino.h>
#include <TM1637Display.h>
#include <ArduinoJson.h>

// Module connection pins (Digital Pins)
#define CLK D0
#define DIO D1

//MAX485
#define SSerialTxControl D6  //RS485 Direction control

#define RS485Transmit    HIGH
#define RS485Receive     LOW

const char* ssid = "Yossawin";
const char* password = "qwerty1234";

//const char* ssid = "3bb-2216";
//const char* password = "wiwvernia1234";

int ledPin = D5;

const int buttonPinAccept = D2;    // the number of the pushbutton pin
const int buttonPinReject = D7;
// Variables will change:
int ledState = HIGH;         // the current state of the output pin
int buttonState;             // the current reading from the input pin
int buttonState2;

WiFiServer server(80);
// The amount of time (in milliseconds) between tests
#define TEST_DELAY   2000

const uint8_t SEG_DONE[] = {
  SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,           // d
  SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,   // O
  SEG_C | SEG_E | SEG_G,                           // n
  SEG_A | SEG_D | SEG_E | SEG_F | SEG_G            // E
};

TM1637Display display(CLK, DIO);

int byteReceived;
int byteSend;

unsigned long currentMillis;
unsigned long previousMillis = 0;

void setup() {
  Serial.begin(9600);
  delay(10);

  pinMode(ledPin, OUTPUT);
  pinMode(buttonPinAccept, INPUT);
  pinMode(buttonPinReject, INPUT);
  pinMode(SSerialTxControl, OUTPUT);

  digitalWrite(SSerialTxControl, RS485Receive);  // Init Transceiver

  digitalWrite(ledPin, ledState);


  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  display.setBrightness(0x0f);
  if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

    HTTPClient http;  //Declare an object of class HTTPClient

    http.begin("http://139.59.251.210/get_work.php?userID=1");  //Specify request destination
    int httpCode = http.GET();                                                                  //Send the request

    if (httpCode > 0) { //Check the returning code
      String payload = http.getString();   //Get the request response payload
      Serial.println(payload);    //Print the response payload
      StaticJsonBuffer<200> jsonBuffer;
      JsonObject& root = jsonBuffer.parseObject(payload);
    
      if (!root.success()) 
      {
        Serial.println("parseObject() failed");
        // show error here
      }
      else
      {
        display.showNumberDec(root["output"]);
      }
    }
    http.end();   //Close connection
  }

}
void Sent_data_max485(String data) {
  digitalWrite(SSerialTxControl, RS485Transmit);
  delay(100);
  Serial.println(data);
  delay(10);
  digitalWrite(SSerialTxControl, RS485Receive);
}

void loop() {
  if (Serial.available()) {
    String phrase;
    const char* c;
    
    String tmp = Serial.readStringUntil('E');
//    Serial.println(tmp);
    if (tmp.startsWith("Init=") || tmp.startsWith("Get=")) {
      String tmp2 = tmp.substring(tmp.indexOf("=") + 1);
//      Serial.println(tmp2);
      if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status
        
        HTTPClient http;  //Declare an object of class HTTPClient
                
        http.begin("http://139.59.251.210/get_work.php?userID=" + tmp2); //Specify request destination
        int httpCode = http.GET();                                                                  //Send the request
        
        if (httpCode > 0) { //Check the returning code        
          String payload = http.getString();   //Get the request response payload
//          Serial.println(payload);    //Print the response payload
          StaticJsonBuffer<200> jsonBuffer;
          JsonObject& root = jsonBuffer.parseObject(payload);
        
          if (!root.success()) 
          {
            Serial.println("parseObject() failed");
            // show error here
          }
          else
          {
             c = root["output"];
             phrase = String(phrase + c);
          }
          delay(10);
          Sent_data_max485("data=" + phrase + 'E');
        }        
        http.end();   //Close connection
      }
    }
    if (tmp.startsWith("Update=")) {
      String phrase;
      const char* c;
    
      String tmp2 = tmp.substring(tmp.indexOf("=") + 1);
//      Serial.println(tmp2);
      if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

        HTTPClient http;  //Declare an object of class HTTPClient

        String url = "http://139.59.251.210/update_work.php?userID=" + tmp2 + "&deviceID=" + tmp2;
        Serial.println(url);

        http.begin(url);  //Specify request destination
        int httpCode = http.GET();                                                                  //Send the request

        if (httpCode > 0) { //Check the returning code
          String payload = http.getString();   //Get the request response payload
//          Serial.println(payload);    //Print the response payload
          if (payload == "Success")
          {
            http.begin("http://139.59.251.210/get_work.php?userID=" + tmp2); //Specify request destination
            int httpCode = http.GET();                                                                  //Send the request

            if (httpCode > 0) { //Check the returning code        
              String payload = http.getString();   //Get the request response payload
//              Serial.println(payload);    //Print the response payload
              StaticJsonBuffer<200> jsonBuffer;
              JsonObject& root = jsonBuffer.parseObject(payload);
            
              if (!root.success()) 
              {
                Serial.println("parseObject() failed");
                // show error here
              }
              else
              {
                 c = root["output"];
                 phrase = String(phrase + c);
              }
              delay(10);
              Sent_data_max485("data=" + phrase + 'E');
            }
          }
        }
        http.end();   //Close connection
      }
    }
    if (tmp.startsWith("Reject=")) {
      String phrase;
      const char* c;
      
      String tmp2 = tmp.substring(tmp.indexOf("=") + 1);
//      Serial.println(tmp2);
      if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

        HTTPClient http;  //Declare an object of class HTTPClient

        String url = "http://139.59.251.210/reject_work.php?userID=" + tmp2 + "&deviceID=" + tmp2;
        Serial.println(url);

        http.begin(url);  //Specify request destination
        int httpCode = http.GET();                                                                  //Send the request

        if (httpCode > 0) { //Check the returning code
          String payload = http.getString();   //Get the request response payload
//          Serial.println(payload);    //Print the response payload
          if (payload == "Success")
          {
            http.begin("http://139.59.251.210/get_work.php?userID=" + tmp2); //Specify request destination
            int httpCode = http.GET();                                                                  //Send the request

            if (httpCode > 0) { //Check the returning code        
              String payload = http.getString();   //Get the request response payload
//              Serial.println(payload);    //Print the response payload
              StaticJsonBuffer<200> jsonBuffer;
              JsonObject& root = jsonBuffer.parseObject(payload);
            
              if (!root.success()) 
              {
                Serial.println("parseObject() failed");
                // show error here
              }
              else
              {
                 c = root["reject"];
                 phrase = String(phrase + c);
              }
              delay(10);
              Sent_data_max485("Reject=" + phrase + 'E');
            }
          }
        }
        http.end();   //Close connection
      }
    }
  }

  int reading = digitalRead(buttonPinAccept);
  //  Serial.println(reading);

  if (reading == HIGH) {
    buttonState = HIGH;
  }
  else {
    if (buttonState == HIGH) {
      buttonState = LOW;
      ledState = !ledState;
      if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

        HTTPClient http;  //Declare an object of class HTTPClient

        http.begin("http://139.59.251.210/update_work.php?userID=1&deviceID=1");  //Specify request destination
        int httpCode = http.GET();                                                                  //Send the request

        if (httpCode > 0) { //Check the returning code
          String payload = http.getString();   //Get the request response payload
//          Serial.println(payload);    //Print the response payload
          if (payload == "Success")
          {
            http.begin("http://139.59.251.210/get_work.php?userID=1");  //Specify request destination
            int httpCode = http.GET();                                                                  //Send the request

            if (httpCode > 0) { //Check the returning code
              String payload = http.getString();   //Get the request response payload
              Serial.println(payload);    //Print the response payload
              StaticJsonBuffer<200> jsonBuffer;
              JsonObject& root = jsonBuffer.parseObject(payload);
            
              if (!root.success()) 
              {
                Serial.println("parseObject() failed");
                // show error here
              }
              else
              {
                display.showNumberDec(root["output"]);
              }
            }
          }
        }
        http.end();   //Close connection
      }
    }
  }

  int reading2 = digitalRead(buttonPinReject);
//    Serial.println(reading2);

  if (reading2 == HIGH) {
    buttonState2 = HIGH;
  }
  else {
    if (buttonState2 == HIGH) {
      buttonState2 = LOW;
      if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

        HTTPClient http;  //Declare an object of class HTTPClient

        http.begin("http://139.59.251.210/reject_work.php?userID=1&deviceID=1");  //Specify request destination
        int httpCode = http.GET();                                                                  //Send the request

        if (httpCode > 0) { //Check the returning code
          String payload = http.getString();   //Get the request response payload
//          Serial.println(payload);    //Print the response payload
          if (payload == "Success")
          {
            http.begin("http://139.59.251.210/get_work.php?userID=1");  //Specify request destination
            int httpCode = http.GET();                                                                  //Send the request

            if (httpCode > 0) { //Check the returning code
              String payload = http.getString();   //Get the request response payload
              Serial.println(payload);    //Print the response payload
              StaticJsonBuffer<200> jsonBuffer;
              JsonObject& root = jsonBuffer.parseObject(payload);
            
              if (!root.success()) 
              {
                Serial.println("parseObject() failed");
                // show error here
              }
              else
              {
                display.showNumberDec(root["reject"]);
                previousMillis = millis();
              }
            }
          }
        }
        http.end();   //Close connection
      }
    }
  }
  currentMillis = millis();
  if (currentMillis - previousMillis > 10000 && previousMillis != 0) {
    if (WiFi.status() == WL_CONNECTED) { //Check WiFi connection status

      HTTPClient http;  //Declare an object of class HTTPClient

      http.begin("http://139.59.251.210/get_work.php?userID=1");  //Specify request destination
      int httpCode = http.GET();                                                                  //Send the request

      if (httpCode > 0) { //Check the returning code
        String payload = http.getString();   //Get the request response payload
        Serial.println(payload);    //Print the response payload
        StaticJsonBuffer<200> jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(payload);
      
        if (!root.success()) 
        {
          Serial.println("parseObject() failed");
          // show error here
        }
        else
        {
          display.showNumberDec(root["output"]);
        }
      }
      http.end();   //Close connection
    }
    previousMillis = 0;
  }
  digitalWrite(ledPin, ledState);
}
