# DFR CAN Module

This CAN module is capable of transmitting 8 analog readings and up to 16 digital 
readings. The module will send up to two messages with analog data and up to 1 message 
with digital data depending on the amount of readings requested.

### Data Packaging:

#### Analog: 
- Analog pins read from 0-1023, which requires two bytes to transmit. Because the 
included CAN library requires data to be formatted as a uint8_t array 8 items long, analog 
readings are split into two values prior to transmission. The two values must be 
recombined on the receiving end. The high byte is sent first and the low byte is sent 
second.

#### Digital: 
- Digital readings are packaged into two uint8_t variables (first two items of the 
output array). The bits are set with bitwise operators and can be read back that way as 
well (with & instead of |).

### Configuration:

The module can be configured to send anywhere from 0 to 8 analog readings and 0 to 16 
digital readings with any valid message IDs via the macros at the top of the program file.

- ANALOG_QUANTITY defines the number of analog readings to be sent. The module will start by 
reading A0, and will sequentially read the analog pins until the quantity requested is 
satisfied. If this quantity is set to 0, no messages will be sent with analog data; if 
this quantity is in the range 1-4, one message will be sent with analog data; if this 
quantity is in the range 5-8, two messages will be sent with analog data. THIS QUANTITY 
MUST BE IN THE RANGE 0-8.
- DIGITAL_QUANTITY operates much the same way. It defines the number of digital readings to 
be sent. The module, as above, will start by reading D0, and will sequentially read the 
digital pins until the quantity requested is satisfied. If the quantity is 0, no message 
will be sent with digital data; if this quantity is in the range 1-16, one message will be 
sent with digital data. THIS QUANTITY MUST BE IN THE RANGE 0-16.
- ANALOG1_ID determines the message ID to be associated with the first message containing 
analog data. This message contains the first half of the requested readings. This must be 
given a valid value even if the message will not be sent (if no analog data was requested).
- ANALOG2_ID determines the message ID to be associated with the second message containing 
analog data. This message contains the second half of the requested readings. This must be 
given a valid value even if the message will not be sent (if less than 5 analog readings 
were requested).
- DIGITAL_ID determines the message ID of the message containing digital data. This must be 
given a valid value even if the message will not be sent (if no digital data was 
requested).
- TRANSMIT_FREQUENCY determines the delay between sets of messages in milliseconds. If this 
value is set to zero the module will send messages as fast as it can, potentially 
congesting the CAN network.
- MESSAGE_DELAY determines the delat between individual messages in a set in milliseconds. 
If this is set to zero, the messages will be sent back to back (depending on the receiving 
device, this may result in messages being lost). This does not impact the transmission 
frequency.
- CAN_BAUD_RATE sets the baud rate of the can port. The standard for automotive applications 
is 500000 (500kbps).

### Example:

```c++
#define ANALOG_QUANTITY         5
#define DIGITAL_QUANTITY        7
#define ANALOG1_ID              150
#define ANALOG2_ID              151
#define DIGITAL_ID              178
#define TRANSMIT_FREQUENCY      25
#define MESSAGE_DELAY           5
#define CAN_BAUD_RATE           500000
```

The above settings will result in 5 analog readings being sent and 7 digital readings. The 
message IDs 150, 151, and 178 will be used for the two analog messages and single digital 
message respectively.

The following message output can be expected:  
`id 150 ide 1 rtr 0 dlc 8 data 00 D0 02 65 01 FF 00 00` <- four analog readings  
`id 151 ide 1 rtr 0 dlc 8 data 00 0B 00 00 00 00 00 00` <- one analog reading with padding  
`id 178 ide 1 rtr 0 dlc 8 data D7 F8 00 00 00 00 00 00` <- 7 digital readings padded

These messages can be interpreted as follows:  
Message 150  
`00 D0 02 65 01 FF 00 00` <- pairs of hex values split in half  
`00D0 0265 01FF 0000` <- full hex values  
`208 613 511 0` <- analog readings  
`A0 = 208`; `A1 = 613`; `A2 = 511`; `A3 = 0`

Message 151  
`00 0b 00 00 00 00 00 00` <- pairs of hex values  
`000b 0000 0000 0000` <- full hex values  
`11` 0 0 0 <- one analog reading padded with zeroes  
`11` <- analog reading  
`A4 = 11`

Message 178  
`D7 F8 00 00 00 00 00 00` <- two bytes with digital readings padded by zeroes  
`D7 F8` <- two bytes with digital readings, represented as hex  
`11010110 00000000` <- seven digital values padded with zeroes  
`1101011` <- seven digital values  
`D0 = 1`; `D1 = 1`; `D2 = 0`; `D3 = 1`; `D4 = 0`; `D5 = 1`; `D6 = 1`

