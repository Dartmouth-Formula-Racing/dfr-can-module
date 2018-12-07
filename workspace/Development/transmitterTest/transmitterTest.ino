/*
 * CAN port receiver example
 * Repeatedly transmits an array of test data to the CAN port
 */

#include <ASTCanLib.h>  

#define ANALOG_QUANTITY   4       // Number of analog readings
#define DIGITAL_QUANTITY  4       // Number of digital readings
#define ANALOG1_ID        143     // Message ID of first analog packet
#define ANALOG2_ID        144     // Message ID of second analog packet
#define DIGITAL_ID        212     // Message ID of digital packet

#define MESSAGE_PROTOCOL  1                        // CAN protocol (0: CAN 2.0A, 1: CAN 2.0B)
#define MESSAGE_LENGTH    8                        // Data length: 8 bytes
#define MESSAGE_RTR       0                        // rtr bit

// MESSAGE CONTENT
// MESSAGE IDs
const uint16_t ID[2] = {255, 256};

// DATA TYPES (0 - analog, 1 - digital)
// outer list corresponds to message IDs, inner list to individual data points
// transmissions of analog data are limited to 4 values
const uint8_t TYPE[2][4] = {{0, 0, 0, 0}, {0, 0, 0, 0}};

// PIN SELECTION
// indexing works the same as above
const uint8_t PIN[2][4] = {{0, 1, 2, 3}, {4, 5, 6, 7}};

// CALCULATE LENGTH OF ABOVE ARRAYS
const uint8_t ID_length = sizeof ID / sizeof *ID;
const uint8_t TYPE_length = sizeof TYPE / sizeof *TYPE;
const uint8_t PIN_length = sizeof PIN / sizeof **PIN;

// Delay between CAN messages
const uint16_t DELAY_DURATION = 50;

// CAN message object
st_cmd_t txMsg;

// Array of data to send
uint8_t sendData[8] = {};
// Transmit buffer
uint8_t txBuffer[8] = {};

void setup() {
  canInit(500000);                  // Initialise CAN port. must be before Serial.begin
  Serial.begin(1000000);             // start serial port
  txMsg.pt_data = &txBuffer[0];      // reference message data to transmit buffer

  Serial.println("setup complete");
}

void loop() {
  uint16_t data[ID_length][PIN_length];
  getData(data);
    
  for(int i = 0; i < ID_length; i++) {
    // prepare data (split values into two bytes)
    uint16_t buf;
    uint8_t upperByte;
    uint8_t lowerByte;
    
    for (int j = 0; j < 4; j++) {
        buf = data[i][j];
  
        upperByte = (uint8_t) (buf >> 8);
        lowerByte = (uint8_t) buf;
  
        sendData[j * 2] = upperByte;
        sendData[j * 2 + 1] = lowerByte;
    }
    
    sendMessage(ID[i], sendData);

    delay(DELAY_DURATION);
  }
}

void getData(uint16_t data[ID_length][PIN_length]) { //pass array to hold data
  for(int id = 0; id < ID_length; id++) {
    for(int pin = 0; pin < PIN_length; pin++) {
      switch(TYPE[id][pin]) {
        case 0:
          data[id][pin] = analogRead(PIN[id][pin]);
          break;
          
        case 1:
          data[id][pin] = digitalRead(PIN[id][pin]);
      }
    }
  }
}

void sendMessage(uint16_t id, uint8_t data[8]) {
  //  load data into tx buffer
  for (int i = 0; i < 8; i++){
    txBuffer[i] = data[i];
  }
  
  // Setup CAN packet.
  txMsg.ctrl.ide = MESSAGE_PROTOCOL;  // Set CAN protocol (0: CAN 2.0A, 1: CAN 2.0B)
  txMsg.id.ext   = id;        // Set message ID
  txMsg.dlc      = MESSAGE_LENGTH;    // Data length: 8 bytes
  txMsg.ctrl.rtr = MESSAGE_RTR;       // Set rtr bit
  
  // Send command to the CAN port controller
  txMsg.cmd = CMD_TX_DATA;       // send message
  // Wait for the command to be accepted by the controller
  while(can_cmd(&txMsg) != CAN_CMD_ACCEPTED);
  // Wait for command to finish executing
  while(can_get_status(&txMsg) == CAN_STATUS_NOT_COMPLETED);
  // Transmit is now complete
}

