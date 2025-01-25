#pragma once
#ifndef __TSPACKET_H__
#define __TSPACKET_H__
#include <string.h>
#include <list>

#define DEBUG_OUTPUT_SEND_RECEIVE_TEXTURE_ERROR 0 // 送受信の情報を出力する

// TSPacket defualt value
#define DEFAULT_1ST_BYTE_OF_PACKET_HEADER	0x47	// Sync byte Bit pattern of 0x47
#define DEFAULT_2ND_BYTE_OF_PACKET_HEADER	0x40	// 1.Transport error indicator:0(1bit) 2.Payload unit start indicator (PUSI):0(1bit) 3.Transport priority:0(1bit) 4.PID:0 0000(5bit high)
#define DEFAULT_3RD_BYTE_OF_PACKET_HEADER	0x40	// PID:0100 0000(8bit) total13bit
#define DEFAULT_4TH_BYTE_OF_PACKET_HEADER	0x10	// 1.Transport scrambling control:00(2bit) 2.Adaptation field control:01(2bit) 3.Continuity counter:0000(4bit)

// TSPacket siaze
#define SIZE_OF_TS_PACKET					188		// TSPacketのサイズ
#define SIZE_OF_TS_PACKET_HEADER			4		// TSPacketのHeader部分のサイズ
#define SIZE_OF_TS_PACKET_PAYLOAD			184		// TSPacketのpayload(Data)部分のサイズ
#define SIZE_OF_TS_PACKET_PAYLOAD_FRAME		4		// payloadのframe部分のサイズ
#define SIZE_OF_TS_PACKET_PAYLOAD_COUNTER	1		// payloadのcounter部分のサイズ
#define SIZE_OF_TS_PACKET_TEXTURE (SIZE_OF_TS_PACKET_PAYLOAD - SIZE_OF_TS_PACKET_PAYLOAD_FRAME - SIZE_OF_TS_PACKET_PAYLOAD_COUNTER) // payloadの質感情報のサイズ
#define SIZE_OF_TS_PACKET_TEXTURE_SIZE		2		// 質感情報のサイズを示す
#define PID_OF_TS_PACKET					0x40	// PID
#define SIZE_OF_RECEIVEPACKET_LIST			10		// 受信リストサイズ

// ErrorCode
#define ERR_CONCAT_LIST_SIZE				-1		// Listサイズ確認エラー
#define ERR_CONCAT_PID						-2		// PIDが0x40ではないエラー
#define ERR_CONCAT_PAYLOAD					-3		// payloadフラグ0
#define ERR_CONCAT_FRAME_SKIP				-4		// frameスキップされる
#define ERR_CONCAT_INDEX					-5		// Indexエラー


typedef struct _SEND_PACKET {
	unsigned char header[SIZE_OF_TS_PACKET_HEADER]		= { '0' };
	unsigned int frame			= 0;// number of frame
	unsigned char counter		= 0;// number of packet
	unsigned char texture[SIZE_OF_TS_PACKET_TEXTURE]	= { '0' };	// texture data. has size or dummy
}SEND_PACKET;// 送信用TSPacket構造体（注意sizeofの使用）

typedef struct _RECEIVE_PACKET {
	unsigned int frame		= 0;// frame number
	unsigned char counter	= 0;// 受信できたTSPacketの数
	unsigned short size		= 0;// 質感情報のサイズ
	unsigned short number	= 0;// sizeから計算されたTSPacketの数
	unsigned char* texture	= NULL;// size cmpBufferSize (ALLSEND 1:65535 0:9280)
}RECEIVE_PACKET;// 受信用TSPacket構造体（注意sizeofの使用）

typedef struct _RECEIVE_INDEX {
	int index		= 0;	// RECEIVE_PACKETのリストのindex
	bool isUsing	= false;// 使用しているフラグ
}RECEIVE_INDEX;

// TSPacket
int setSyncByte(unsigned char* header, int syncByte);
int getSyncByte(unsigned char* header, int* syncByte);
int setTransportErrorIndicator(unsigned char* header, int value);
int getTransportErrorIndicator(unsigned char* header, int* value);
int setPayloadUnitStartIndicator(unsigned char* header, int value);
int getPayloadUnitStartIndicator(unsigned char* header, int* value);
int setTransportPriority(unsigned char* header, int value);
int getTransportPriority(unsigned char* header, int* value);
int setPID(unsigned char* header, int pid);
int getPID(unsigned char* header,int* pid);
int setTransportScramblingControl(unsigned char* header, int value);
int getTransportScramblingControl(unsigned char* header, int* value);
int setAdaptationFieldControl(unsigned char* header, int value);
int getAdaptationFieldControl(unsigned char* header, int* value);
int setContinuityCounte(unsigned char* header, int value);
int getContinuityCounte(unsigned char* header, int* value);

// SendPacket
// sendPacketListの初期化
void initSendPacketList(SEND_PACKET* sendPacketList, int size);
// headerの初期化
void initSendPacketHeader(unsigned char* header);
void changeStartIndicator(SEND_PACKET* sendPacket, bool isStart);
void setHeaderCounte(SEND_PACKET* sendPacket, int value);
// サイズ分割されるTSPacketサイズを計算
int calcSizeSplitTextureSendInformation(int sizeTextureSend);
/*
*
* param
*   sendPacket			list of packet
*   textureSendBuf      texture buffer
*   sizeTextureSend		size
*   frame				number of frame
*   counter				counter of header
* return
* comment
*
*/
int splitTextureSendInformation(SEND_PACKET* sendPacket, const char* textureSendBuf, int sizeTextureSend, int frame, int* counterHeader);
// 構造体をバッファにコピー(sizoef(SEND_PACKET)=193)
void copySendPacketToBuffer(unsigned char* dist, SEND_PACKET* sendPacket);

// SIZE_OF_RECEIVEPACKET_LISTで初期化
void initReceivePacketList(RECEIVE_PACKET* receivePacketList, std::list<RECEIVE_INDEX>* indexList, int rcvTextureBufSize);
void finalReceivePacketList(RECEIVE_PACKET* receivePacketList);
/*
*
* param
*   receivePacketList       receive packet list
*   textureReceiveBuf       buffer
*   indexList				index list
*   rcvBuffer				recieve texture of one frame
* return
*   > 0 : copy buffer size
*   = 0 : tspacket insert success (no copy)
*   < 0 : error
* comment
*   
*/
int concatTextureReceiveInformation(RECEIVE_PACKET* receivePacketList, unsigned char* textureReceiveBuf, std::list<RECEIVE_INDEX>* indexList, unsigned char* rcvBuffer);
#endif // __TSPACKET_H__