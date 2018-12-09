/*
 * CAN_Module | Pedal Pack
 * Dartmouth Formula Racing
 *
 * Author: Jai K. Smith
 * Date: December, 2018
 */

#include <ASTCanLib.h>  

#define ANALOG_QUANTITY     8       // Number of analog pins to send
#define DIGITAL_QUANTITY    16      // Number of digital pins to send
#define ANALOG1_ID          254     // Message ID of first analog packet
#define ANALOG2_ID          255     // Message ID of second analog packet
#define DIGITAL_ID          256     // Message ID of digital packet
#define TRANSMIT_FREQUENCY  25      // Delay between sets of messages in milliseconds
#define MESSAGE_DELAY       5       // Delay between individual messages in milliseconds
#define CAN_BAUD_RATE       500000  // CAN port baud rate

#define MESSAGE_PROTOCOL    1       // CAN protocol (0: CAN 2.0A, 1: CAN 2.0B)
#define MESSAGE_LENGTH      8       // Message length in bytes
#define MESSAGE_RTR         0       // rtr bit

// Delay between CAN messages
const uint16_t DELAY_DURATION = TRANSMIT_FREQUENCY;

// CAN message object
st_cmd_t txMsg;

// Array of data to send
uint8_t sendData[8] = {};
// Transmit buffer
uint8_t txBuffer[8] = {};

void setup() {
  canInit(CAN_BAUD_RATE);            // Initialise CAN port. must be before Serial.begin
  Serial.begin(1000000);             // start serial port
  txMsg.pt_data = &txBuffer[0];      // reference message data to transmit buffer

  // confirm successful initiation over serial
  Serial.println("setup complete, transmitting...");
}

void loop() {
  // only send messages with analog data if it was requested
  if(ANALOG_QUANTITY > 0) {
    // instantiate two uint8_t arrays to hold analog message content
    uint8_t analogData1[8];
    uint8_t analogData2[8];
  
    // load packaged data from getAnalogData() function
    getAnalogData(analogData1, analogData2);
  
    // send first analog message with ID specified in macro and first half of data
    sendMessage(ANALOG1_ID, analogData1);
    delay(MESSAGE_DELAY); // wait 5 ms for receiving end to process message before continuing

    // only send second message if more than four readings were requested
    if(ANALOG_QUANTITY > 4) {
      // send second analog message with ID specified in macro and second half of data
      sendMessage(ANALOG2_ID, analogData2);
      delay(MESSAGE_DELAY); //wait 5 ms
    }
  }

  // only send messages with digital data if it was requested
  if(DIGITAL_QUANTITY > 0) {
    // instantiate uint8_t array to hold digital message content, only first to values of array are actually used
    uint8_t digitalData[8];
  
    // load packaged data from getDigitalData() function
    getDigitalData(digitalData);
  
    // send message with digital data and ID specified in macro
    sendMessage(DIGITAL_ID, digitalData);
  }

  // wait a bit before sending the next round of messages
  delay(DELAY_DURATION);
}

// load and package analog data, pass two uint8_t parameters to hold output
void getAnalogData(uint8_t sendData[8], uint8_t sendData2[8]) {
  // instantiate data array to temporarily hold data from pins
  uint16_t data[8];

  // iterate through number of requested analog readings, and load them into the data array
  for(int i = 0; i < ANALOG_QUANTITY; i++) {
    data[i] = analogRead(i);
  }

  // instantiate variables to hold upper and lower bytes of data points once separated
  uint16_t buf;
  uint8_t upperByte;
  uint8_t lowerByte;

  // separate each reading into a high and a low byte (to send over CAN)
  for(int i = 0; i < ANALOG_QUANTITY; i++) {
      // load data into buffer variable
      buf = data[i];

      // get high byte
      upperByte = (uint8_t) (buf >> 8);

      //get lower byte
      lowerByte = (uint8_t) buf;

      // store first 4 pairs in first output array, rest in second output variable
      if(i < 4) {
        sendData[i * 2] = upperByte;
        sendData[i * 2 + 1] = lowerByte;
      } else {
        sendData2[(i - 4) * 2] = upperByte;
        sendData2[(i - 4) * 2 + 1] = lowerByte;
      }
  }

  //set the remaining values in the list to zero (analog pins that were not requested)
  for(int i = ANALOG_QUANTITY; i < 8; i++) {
    if(i < 4) {
      sendData[i * 2] = 0;
      sendData[i * 2 + 1] = 0;
    } else {
      sendData2[(i - 4) * 2] = 0;
      sendData2[(i - 4) * 2 + 1] = 0;
    }
  }
}

// load and package digital data, pass uint8_t array to hold output
void getDigitalData(uint8_t sendData[8]) {
  // instantiate array to hold digital readings
  uint8_t data[16];

  // iterate through the number of requested digital readings, and load them into the data array
  for(int i = 0; i < DIGITAL_QUANTITY; i++) {
    data[i] = digitalRead(i);
  }

  // write all the digital readings as bits, takes up two bytes in the output array
  for(int i = 0; i < DIGITAL_QUANTITY; i++) {
    if(i < 8) {
      sendData[0] &= ~(1 << (7 - i));
      sendData[0] |= data[i] << (7 - i);
    } else {
      sendData[1] &= ~(1 << (15 - i));
      sendData[1] |= data[i] << (15 - i);
    }
  }

  for(int i = DIGITAL_QUANTITY; i < 16; i++) {
    if(i < 8) {
      sendData[0] &= ~(1 << (7 - i));
    } else {
      sendData[1] &= ~(1 << (15 - i));
    }
  }
  
  // fill the rest of the output array with zeroes
  for(int i = 2; i < 8; i++) {
    sendData[i] = 0;
  }
}

// send message over CAN, takes ID and uint8_t array of length 8 as parameters
void sendMessage(uint16_t id, uint8_t data[8]) {
  //  load data into tx buffer
  for (int i = 0; i < 8; i++){
    txBuffer[i] = data[i];
  }
  
  // Setup CAN packet.
  txMsg.ctrl.ide = MESSAGE_PROTOCOL;  // Set CAN protocol (0: CAN 2.0A, 1: CAN 2.0B)
  txMsg.id.ext   = id;                // Set message ID
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
