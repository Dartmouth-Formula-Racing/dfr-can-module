/*
 * CAN port receiver example
 * Receives data on the CAN buss and prints to the serial port
 */

#include <ASTCanLib.h>              

#define MESSAGE_ID        0       // Message ID
#define MESSAGE_PROTOCOL  1       // CAN protocol (0: CAN 2.0A, 1: CAN 2.0B)
#define MESSAGE_LENGTH    8       // Data length: 8 bytes
#define MESSAGE_RTR       0       // rtr bit

// Function prototypes
void serialPrintData(st_cmd_t *msg);

// CAN message object
st_cmd_t Msg;

// Buffer for CAN data
uint8_t Buffer[8] = {};

void setup() {
  canInit(500000);            // Initialise CAN port. must be before Serial.begin
  Serial.begin(1000000);       // start serial port
  Msg.pt_data = &Buffer[0];    // reference message data to buffer
  
  // Initialise CAN packet.
  // All of these will be overwritten by a received packet
  Msg.ctrl.ide = MESSAGE_PROTOCOL;  // Set CAN protocol (0: CAN 2.0A, 1: CAN 2.0B)
  Msg.id.ext   = MESSAGE_ID;        // Set message ID
  Msg.dlc      = MESSAGE_LENGTH;    // Data length: 8 bytes
  Msg.ctrl.rtr = MESSAGE_RTR;       // Set rtr bit

  Serial.println("setup complete");
}

void loop() {
  // Clear the message buffer
  clearBuffer(&Buffer[0]);
  
  // Send command to the CAN port controller
  Msg.cmd = CMD_RX_DATA;
  
  // Wait for the command to be accepted by the controller
  while(can_cmd(&Msg) != CAN_CMD_ACCEPTED);
  // Wait for command to finish executing
  while(can_get_status(&Msg) == CAN_STATUS_NOT_COMPLETED);
  // Data is now available in the message object
  // Print received data to the terminal
  serialPrintData(&Msg);
}

void serialPrintData(st_cmd_t *msg){
  char textBuffer[50] = {0};
  if (msg->ctrl.ide>0){
    sprintf(textBuffer,"id %d ",msg->id.ext);
  }
  else
  {
    sprintf(textBuffer,"id %04x ",msg->id.std);
  }
  Serial.print(textBuffer);
  
  //  IDE
  sprintf(textBuffer,"ide %d ",msg->ctrl.ide);
  Serial.print(textBuffer);
  //  RTR
  sprintf(textBuffer,"rtr %d ",msg->ctrl.rtr);
  Serial.print(textBuffer);
  //  DLC
  sprintf(textBuffer,"dlc %d ",msg->dlc);
  Serial.print(textBuffer);
  //  Data
  sprintf(textBuffer,"data ");
  Serial.print(textBuffer);
  
  for (int i =0; i<msg->dlc; i++){
    sprintf(textBuffer,"%02X ",msg->pt_data[i]);
    Serial.print(textBuffer);
  }

  // process data (combine bytes and display in base 10)
  char values[8][3] = {};
  for (int i = 0; i < 8; i++) {
     sprintf(values[i], "%02X", msg->pt_data[i]);
  }

  if(msg->id.ext != 212) {
    uint16_t data[4] = {};
    uint8_t upperByte;
    uint8_t lowerByte;
    
    for (int i = 0; i < 4; i++) {
        upperByte = msg->pt_data[i * 2];
        lowerByte = msg->pt_data[i * 2 + 1];
  
        data[i] = ((uint16_t) upperByte << 8) | lowerByte;
    }
  
    Serial.print("|| processed data: ");
    
    for (int i = 0; i < 4; i++) {
      char buf[5];
      sprintf(buf, "%04d", data[i]);
      Serial.print(buf);
      Serial.print(" ");
    }
  } else {
    Serial.print("|| processed data: ");
    uint8_t data[2] = {msg->pt_data[0], msg->pt_data[1]};
    for(int i = 0; i < 2; i++) {
      print_uint8_t(data[i]);
    }
  }

  Serial.print("\r\n");
}

void print_uint8_t(uint8_t n) {
  int i;
  for (i = 7; i >= 0; i--) {
    char buf[9];
    sprintf(buf, "%d", (n & (1<<i)) >> i);
    Serial.print(buf);
  }
  putchar('\n');
}
