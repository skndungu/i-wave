/*
* Title: iWave Battery Monitor Slave
*
* Author: Simon 
*
* Date: 19th, June, 2021
*
*/

/* Icludes ESP Wifi header */ 
#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "LoRa.h"
#include "bq78412.h"

#include <SPI.h> 
#include <LoRa.h>

void setup(){
	Serial.begin(9600);
	Serial.println("iWave Slave SLDC000001");


    WiFi.begin(ssid, password);
	  while (WiFi.status() != WL_CONNECTED) {
	    delay(500);
	    Serial.print(".");
  }
    Serial.println("");
  	Serial.print("Connected to WiFi network with IP Address: ");
  	Serial.println(WiFi.localIP());

    LoRa.setPins(ss, rst, dio0);
    Serial.println("LoRa Transmistter initialized");
    if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

}

void loop(){
    
    current_send_time = millis();
	// Fetch Data from the bq
	battery_voltage = bqDataRead(1); // passed value "1" fetches voltage as read from the bq
	battery_SoC = bqDataRead(2); // passed value "2" fetches SoC % as read from the bq
	battery_temp = bqDataRead(3); // passed value "3" fetches temp of the battery as read from the bq

	/* concatenate data payload to be sent over LoRa */
	v.concat(battery_voltage);
	s.concat(battery_SoC);
	t.concat(battery_temp);

    String bq_lora_msg = v + ", " + s + "," + t;
    
    /* Ensures to only send data only at specific intervals */
    if((unsigned long)(current_send_time - last_send_time) >= interval + 100){

    /* Invoke method to send the data to the Master device */
    send_bq_data(bq_lora_msg);

    Serial.println("Sending bq data: " message);   
    
    /* Takes a snapshot to set track of time until the next even */
    last_send_time = current_send_time; 

    }


}

double bqDataRead(int param){
	/* Host to Bq Voltage level UART Command */
	if(param == 1){
		Serial.write(B11111111); // initial command from host to bq 0xFF
		Serial.write(B00010110); 
		Serial.write(B00000100); 
		Serial.write(B00000000); 
		Serial.write(B00000001); 
		Serial.write(B00000000); 
		Serial.write(B00010011); 
	}/* Host to Bq SoC % UART Command */
	else if(param == 2){
        Serial.write(B11111111); // initial command from host to bq 0xFF
		Serial.write(B00010110); 
		Serial.write(B00010110); 
		Serial.write(B00000000); 
		Serial.write(B00000001); 
		Serial.write(B00000000); 
		Serial.write(B00000001); 
	} /* Host to Bq Temperature UART Command */
	else if(param == 3){
		Serial.write(B11111111); // initial command from host to bq 0xFF
		Serial.write(B00010110);
		Serial.write(B00000010);
		Serial.write(B00000000);
		Serial.write(B00000001);
		Serial.write(B00000000);
		Serial.write(B00010101);
	}

    /* loops for 10000 counts */
    while(timer < 10000){
    	/* Serial Data becomes available from bq to the Host */
    	if(Serial.available() == 1){
    		incoming_message_byte = Serial.read();
    		incoming_message_char = bq_message_byte;

    		/* next line appends the incmoming read bytes to the byte array "bq_message" */
            
            bq_message[bq_counter] = bq_message_byte; 

            /* reset */

            timer = 0;
            bq_counter ++;
    	} else {
    		// Increment timer
    		timer ++;
    	}
    }

    bq_final_data = parsebqArray(2,3, bq_message);
    if(param == 1){
       bq_final_data = bq_final_data / 1000; // returns the average 
    }

    return bq_final_data; 

}


/* Method to parse raw byte dats from the bq at the Host MCU */

double parsebqArray(int lsb, int msb, byte bq_message[]){
	// bq demands we use the lsb format 
	int i = lsb;

	// initialize counter to zero
	int byte_pos = 0;

	// results 
	double bq_results = 0;

	while(i <= msb ){
		bq_results = bq_results + bq_message[i] * pow(256,byte_pos);
		byte_pos ++;
		i ++;
	}
	return bq_results;
}


void send_bq_data(String bq_data){
 LoRa.beginPacket();  // starts a packet to send
 LoRa.Write(slave_addrr);  // adds specific slave ID/address
 LoRa.write(master_addrr); // adds master receiver ID/address
 LoRa.Write(lora_msg_count);// adds message counter
 LoRa.print(bq_data); // adds the data payload to be sent over
 LoRa.endPacket(); 
 lora_msg_count++; 
}






























