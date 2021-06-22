float battery_voltage = 0.0;
float battery_current = 0.0;
float battery_SoC = 0.0;
float battery_temp = 0.0;

char bq_temp;
int bq_counter;
byte bq_message[10] = {0};

byte incoming_message_byte;
char incoming_message_char;

double bq_final_data = 0.0;

String v,s,t;