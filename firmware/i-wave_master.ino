#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include "time.h"
#include <SPI.h>
#include <LoRa.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//OTA libs 
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

const char* ssid  = "your wifi ssid";
const char* password = "your wifi password";
const char* dataUrl = "";//data url endpoint
const char* ntpServer = "pool.ntp.org"; //ntp timeserver for payload timestamps

//time set the gmtoffset according and the daylight offset according to your time zone
const long gmtoffsetSec = 3;
const int daylightOffsetSec = 10800;

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

AsyncWebServer server(80);

// Variables to save date and time
String formattedDate;

//LoRa
#define ss 5
#define rst 14
#define dio0 2

byte msgCount = 0;            // count of incoming messages
byte masterAddress = 0x00;    // address of the Master
unsigned long timeNow = 0;
bool loraReceived = false;
String slavePayload = ""; //payload from the slave

void setup() {
  Serial.println("I-wave Master");
  Serial.begin(115200);
  //wifi config
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
   server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi, this is the iWave Slave.");
      AsyncElegantOTA.begin(&server);    // Start ElegantOTA
	  server.begin();
	  Serial.println("HTTP server started");
  });
  
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  // Initialize a NTPClient to get time
  timeClient.begin();
  timeClient.setTimeOffset(10800);
  //initialize LoRa
  LoRa.setPins(ss, rst, dio0);
  Serial.println("LoRa Receiver initialized");
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed");
    while (1);
  }
}

void loop() {
  	/* OTA updates enabled */
  AsyncElegantOTA.loop(); // executes when we have available OTA
  
  String timeStamp = getTimestamp(); //get timestamp
  //wait to receive loRa packet form the slaveBoard
  onReceive(LoRa.parsePacket());
  if (slavePayload.length() > 0) {
    //decode the data
    int n = slavePayload.length();
    char sensorArray[n + 1];
    strcpy(sensorArray, slavePayload.c_str());

    char *Words[10];
    byte word_count = 0; //number of words
    char * item = strtok (sensorArray, " ,");
    while (item != NULL) {
      if (word_count >= 10) {
        break;
      }
      Words[word_count] = item;
      item = strtok (NULL, " ,"); //getting subsequence word
      word_count++;
    }
    //send slave payload once received
    sendSlaveData(Words[2], Words[1], Words[0], timeStamp);
  }
  slavePayload = "";
}


String getTimestamp() {
  while (!timeClient.update()) {
    timeClient.forceUpdate();
  }
  formattedDate = timeClient.getFormattedDate();
  return formattedDate;
}


void sendSlaveData(String batTemp, String batSoc, String batVoltage, String timeStamp) {
  if (WiFi.status() == WL_CONNECTED) {
    //post data to anabi server
    Serial.println("Sending aux board data");
    HTTPClient http;
    http.begin(dataUrl);
    http.addHeader("Content-Type", "application/json");
    //add the necessary header for authorization if present here
    int httpResponseCode = http.POST("");//post data to the endpoint
    Serial.println("Sending data 1.Battery Temperature: " + String(batTemp) + " Battery SoC: " + String(batSoc) + "Battery Voltage: " + String(batVoltage) + "");
    Serial.print("Post data resp code: ");
    Serial.println(httpResponseCode);//print http request response code
    String payload = http.getString();
    JSONVar myobject = JSON.parse(payload);
    Serial.println(myobject);//print server response
  }
  else {
    Serial.println("WIFI disconnected!!");
  }
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  byte sender = LoRa.read();            // sender(slave)address
  byte recipient = LoRa.read();          // recipient(master)address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }
  if (incomingLength != incoming.length()) {// check length for error
    Serial.println("error: message length does not match length");
    return;
  }
  // if the recipient isn't this device or broadcast,
  if (recipient != masterAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;
  }
  slavePayload = incoming;

  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();
}
