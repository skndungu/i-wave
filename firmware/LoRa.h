int lora_msg_count = 0; // keeps track of all messages sent 
char slave_addrr = "SLDC000001"; // address of the slave dev ---> update this value for different slave device
char master_addrr = "Mas000001";// address of the Master dev ---> This does not change as it's one

long current_send_time = 0;
long last_send_time = 0;
int interval  = 6000; // interval between different sends ---> udpate value to suit intervals


#define ss 5
#define rst 14
#define dio0 2

