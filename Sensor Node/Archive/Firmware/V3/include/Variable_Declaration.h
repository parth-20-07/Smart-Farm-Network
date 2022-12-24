#ifndef Variable_Declaration_h
#define Variable_Declaration_h

/* -------------------------------------------------------------------------- */
/*                            BASIC DEVICE DETAILS                            */
/* -------------------------------------------------------------------------- */
String node_type;
#define NODE_ID_SIZE 5
String NODE_ID;
#define SLEEP_TIME 10 // Sleep Time in S //TODO: Delete this line in final version
// #define SLEEP_TIME 300 // Sleep Time in S

/* -------------------------------------------------------------------------- */
/*                          BLE CONNECTION VARIABLES                          */
/* -------------------------------------------------------------------------- */
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID_RX "aca5483e-36e1-4688-b7f5-ea07361b26a7"
#define CHARACTERISTIC_UUID_TX "beb5483e-36e1-4688-b7f5-ea07361b26a8"
bool recieved_data_flag = false;
String incoming_ble_msg;

#endif