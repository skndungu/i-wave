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
---
### Communications Interface
---
The `bq78412` provides a `UART` communications interface for parameter initialization during system config and test. This interface also provides real-time measurement capability and acess to stored battery performance data.

- To access the stored battery data from the `BQ78412`, default address is `0xFF` - it can be changed in parameter flash\
- Communication to the `BQ78412` is via messages
- The communication interface has a fixed data config 
    - 9600 or 1200 baud rate(set by DevConfig1[11])
    - 8 bits
    - No Parity 
    - 1 stop bit
    - No flow control
### Command Set & Status Reporting 
--- 
General Command Format Host to b17842
---
Address | ID | Param0 | Param1| Param2 | Param3 | Checksum 
------- | ---- | -------- | ------------- | ------- | ---- | --------
- The address is a hexadecimal number that distinguishes between target bq78412 devices. The default address is 0xFF
- The header ID is a hexadecimal number that distinguishes between individual commands.
- Checksum is XOR `"^"` of all bytes (excluding checksum) including header ID = 0xFF XOR Address XOR ID XOR
Param0 …..XOR Param3 

    - The bq78412 uses the `"!"` character as the `ACK` response code. Its value is `0x21`.
    - The bq78412 uses `0x15` as `NACK` response code.
 
Read Device Type and Version
---
Host to bq78412
Address | 0x12 | 0x01 | 0x00| 0x00 | 0x00 | Checksum 
------- | ---- | -------- | ------------- | ------- | ---- | --------

Response from bq78412
Address | ACK | "b" | "q"| "7" | "8" | "4" | "1" | "2" | Ver | Rev | Build| Checksum 
------- | ---- | -------- | ------------- | ------- | ---- | -------- |------- | ---- | -------- | ------------- | ------- | ---- 

`Note: Firmware version, revision & build are reported as hexadecimal numbers`

