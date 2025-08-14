# TSPacket Library
A C++ library for handling Transport Stream (TS) packets with texture data transmission functionality.

<div align="center">
  <br/>
  <div>
      <a href="./README.md">English</a> | <a href="./README.ja-JP.md">日本語</a>
  </div>
  <br/>
</div>



## Overview

This library provides functionality to:
- Create and manipulate TS packet headers according to MPEG-2 Transport Stream standard
- Split large texture data into multiple TS packets for transmission
- Receive and reassemble TS packets back into complete texture data
- Manage packet ordering and continuity

## Key Features

### TS Packet Structure
- **Packet Size**: 188 bytes total
- **Header Size**: 4 bytes
- **Payload Size**: 184 bytes
- **Default PID**: 0x40
- **Sync Byte**: 0x47

### Header Field Functions
The library provides getter/setter functions for all TS packet header fields:
- Sync byte
- Transport error indicator
- Payload unit start indicator
- Transport priority
- PID (Packet Identifier)
- Transport scrambling control
- Adaptation field control
- Continuity counter

## Data Structures

### SEND_PACKET
Structure for outgoing packets containing:
- 4-byte header
- Frame number (4 bytes)
- Packet counter (1 byte)
- Texture data (179 bytes)

### RECEIVE_PACKET
Structure for incoming packets containing:
- Frame number
- Received packet count
- Total texture size
- Expected packet count
- Texture data buffer

### RECEIVE_INDEX
Index management structure for packet ordering:
- Array index reference
- Usage flag

## Main Functions

### Transmission Functions
```cpp
// Calculate number of packets needed for texture data
int calcSizeSplitTextureSendInformation(int sizeTextureSend);

// Split texture data into multiple TS packets
int splitTextureSendInformation(SEND_PACKET* sendPacket, 
                               const char* textureSendBuf, 
                               int sizeTextureSend, 
                               int frame, 
                               int* counterHeader);

// Copy packet structure to buffer for transmission
void copySendPacketToBuffer(unsigned char* dist, SEND_PACKET* sendPacket);
```

### Reception Functions
```cpp
// Initialize receive packet list and index management
void initReceivePacketList(RECEIVE_PACKET* receivePacketList, 
                          std::list<RECEIVE_INDEX>* indexList, 
                          int rcvTextureBufSize);

// Process received packets and reassemble texture data
int concatTextureReceiveInformation(RECEIVE_PACKET* receivePacketList, 
                                   unsigned char* textureReceiveBuf,
                                   std::list<RECEIVE_INDEX>* indexList, 
                                   unsigned char* rcvBuffer);
```

## Error Codes

| Code | Description |
|------|-------------|
| -1 | ERR_CONCAT_LIST_SIZE - List size verification error |
| -2 | ERR_CONCAT_PID - PID is not 0x40 |
| -3 | ERR_CONCAT_PAYLOAD - Payload flag error |
| -4 | ERR_CONCAT_FRAME_SKIP - Frame skip detected |
| -5 | ERR_CONCAT_INDEX - Index error |

## Usage Example

### Sending Texture Data
1. Calculate required packet count
2. Initialize send packet list
3. Split texture data into packets
4. Copy packets to transmission buffer

### Receiving Texture Data
1. Initialize receive packet list and index management
2. Process incoming packets with `concatTextureReceiveInformation`
3. Function returns positive value when complete texture is reassembled
4. Clean up with `finalReceivePacketList`

## Configuration

- **Receive packet list size**: 10 packets (configurable via SIZE_OF_RECEIVEPACKET_LIST)
- **Debug output**: Enable/disable via DEBUG_OUTPUT_SEND_RECEIVE_TEXTURE_ERROR macro

## Dependencies

- Standard C++ library
- `<string.h>` for memory operations
- `<list>` for STL list container
