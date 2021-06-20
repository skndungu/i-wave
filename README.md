# i-wave
The repo contains firmware codes for both master and slave devices that monitor battery SoH among other parameters needed from a UPS set. 
Everything on how to program and what the codes does is documented sequentially in this repo.
The data from the slaves to the master is transferred wirelessly via LoRa where it's then logged to some server.
## Battery Monitoring System 
## Master
The master consists of an ESP32 as the main MCU and the WiFi gateway for server communications. It also got a LoRa module that receives the data wirelessly from the slave modules the do the monitoring of the individual batteries.
<img src="images/i-wave_master_4.png" height="400"></img> 

# Slave
The slave also makes use of the ESP32 as the main controller unit, it makes use of a dedicated IC [bq78412](https://www.ti.com/lit/ds/symlink/bq78412.pdf?ts=1624178578744&ref_url=https%253A%252F%252Fwww.ti.com%252Fproduct%252FBQ78412) to perform the battery states monitoring, once all important data has been extracted it will be sent to the master via LoRa
<img src="images/i-wave_slave.png" height="400"></img> 
