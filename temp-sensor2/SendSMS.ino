#include <WiFi.h>
#include <WiFiClient.h>

#include <Temboo.h>
#include <limits.h>
#include "TembooAccount.h" // Contains Temboo account information

WiFiClient client;


// The number of times to trigger the action if the condition is met
// We limit this so you won't use all of your Temboo calls while testing
int maxCalls = 10;

// The number of times this Choreo has been run so far in this sketch
int calls = 0;

int inputPin = 2;

// Choreo execution interval in milliseconds
unsigned long choreoInterval = 3600000; 
// Store the time of the last Choreo execution
unsigned long lastChoreoRunTime = millis() - choreoInterval; 

void setup() {
  Serial.begin(9600);
  
  int wifiStatus = WL_IDLE_STATUS;

  // Determine if the WiFi Shield is present
  Serial.print("\n\nShield:");
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("FAIL");

    // If there's no WiFi shield, stop here
    while(true);
  }

  Serial.println("OK");

  // Try to connect to the local WiFi network
  while(wifiStatus != WL_CONNECTED) {
    Serial.print("WiFi:");
    wifiStatus = WiFi.begin(WIFI_SSID);
    if (wifiStatus == WL_CONNECTED) {
      Serial.println("OK");
    } else {
      Serial.println("FAIL");
    }
    delay(5000);
  }

  // Initialize condition pins
  pinMode(inputPin, INPUT);
  Serial.println("Setup complete.\n");
}

void loop() {
  if(millis() - lastChoreoRunTime >= choreoInterval) {
    int sensorValue = analogRead(inputPin);
    Serial.println("Sensor: " + String(sensorValue));
    if (sensorValue > 85) {
      if (calls < maxCalls) {
        Serial.println("\nTriggered! Calling SendSMS Choreo...");
        runSendSMS(sensorValue);
        lastChoreoRunTime = millis();
        calls++;
      } else {
        Serial.println("\nTriggered! Skipping to save Temboo calls. Adjust maxCalls as required.");
      }
    }
  }

  if (millis() - lastChoreoRunTime >= ULONG_MAX - 10000) {
    lastChoreoRunTime = millis() - choreoInterval;
  }
  delay(250);
}

void runSendSMS(int sensorValue) {
  TembooChoreo SendSMSChoreo(client);

  // Set Temboo account credentials
  SendSMSChoreo.setAccountName(TEMBOO_ACCOUNT);
  SendSMSChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
  SendSMSChoreo.setAppKey(TEMBOO_APP_KEY);

  // Set profile to use for execution
  SendSMSChoreo.setProfile("Tempsensor");
  // Identify the Choreo to run
  SendSMSChoreo.setChoreo("/Library/Twilio/SMSMessages/SendSMS");

  // Run the Choreo
  unsigned int returnCode = SendSMSChoreo.run(true);

  // Read and print the error message
  while (SendSMSChoreo.available()) {
    char c = SendSMSChoreo.read();
    Serial.print(c);
  }
  Serial.println();
  SendSMSChoreo.close();
}