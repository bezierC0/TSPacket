#include <iostream>
#include "TSPacket.h"
#if DEBUG_OUTPUT_SEND_RECEIVE_TEXTURE_ERROR
unsigned int frame_before = 0;// frame number
unsigned char counter_before = 0;// 受信できたTSPacketの数
unsigned short number_before = 0;// sizeから計算されたTSPacketの数
#endif
int setSyncByte(unsigned char* header, int syncByte) {
	if (!header)
		return 1;
	if (syncByte >= 256 || syncByte < 0)
		return 2;
	header[0] = (unsigned char)syncByte;
	return 0;
}

int getSyncByte(unsigned char* header, int* syncByte) {
	if (!header)
		return 1;

	(*syncByte) = header[0];
	return 0;
}

int setTransportErrorIndicator(unsigned char* header, int value) {
	if (!header)
		return 1;
	if(value)
		header[1] = header[1] | 0x80;
	else
		header[1] = header[1] & 0x7f;
	return 0;
}

int getTransportErrorIndicator(unsigned char* header, int* value) {
	if (!header)
		return 1;

	(*value) = header[1] >> 7;
	return 0;
}

int setPayloadUnitStartIndicator(unsigned char* header, int value) {
	if (!header)
		return 1;
	if (value)
		header[1] = header[1] | 0x40;
	else
		header[1] = header[1] & 0xbf;
	return 0;
}

int getPayloadUnitStartIndicator(unsigned char* header, int* value) {
	if (!header)
		return 1;

	(*value) = (header[1] >> 6) & 0x01;
	return 0;
}

int setTransportPriority(unsigned char* header, int value) {
	if (!header)
		return 1;
	if (value)
		header[1] = header[1] | 0x20;
	else
		header[1] = header[1] & 0xdf;
	return 0;
}

int getTransportPriority(unsigned char* header, int* value) {
	if (!header)
		return 1;

	(*value) = (header[1] >> 5) & 0x01;
	return 0;
}

int setPID(unsigned char* header, int pid) {
	if (!header)
		return 1;
	header[1] = header[1] & 0xe0 | (pid >> 8);
	header[2] = header[2] & 0x00 | pid ;
	return 0;
}

int getPID(unsigned char* header,int* pid) {
	if (!header)
		return 1;

	(*pid) = (header[1] & 0x1F) << 8 | header[2];
	return 0;
}

int setTransportScramblingControl(unsigned char* header, int value) {
	if (!header)
		return 1;
	if (value < 4 && value >= 0)
		header[3] = header[3] | (value << 6);
	else
		return 2;
	return 0;
}

int getTransportScramblingControl(unsigned char* header, int* value) {
	if (!header)
		return 1;

	(*value) = (header[3] >> 6) & 0x03;
	return 0;
}

int setAdaptationFieldControl(unsigned char* header, int value) {
	if (!header)
		return 1;
	if (value < 4 && value >= 0)
		header[3] = (header[3] & 0xcf) | (value << 4);
	else
		return 2;
	return 0;
}

int getAdaptationFieldControl(unsigned char* header, int* value) {
	if (!header)
		return 1;

	(*value) = (header[3] >> 4) & 0x03;
	return 0;
}

int setContinuityCounte(unsigned char* header, int value) {
	if (!header)
		return 1;
	header[3] = (header[3] & 0xf0) | value;
	return 0;
}

int getContinuityCounte(unsigned char* header, int* value) {
	if (!header)
		return 1;

	(*value) = header[3] & 0x0f;
	return 0;
}

void initSendPacketList(SEND_PACKET* sendPacketList, int size) {
	if (!sendPacketList)
		return;
	for (int i = 0; i < size; i++) {
		initSendPacketHeader(sendPacketList[i].header);
	}
	
}

void initSendPacketHeader(unsigned char* header) {
	if (!header)
		return;
	header[0] = DEFAULT_1ST_BYTE_OF_PACKET_HEADER;
	header[1] = DEFAULT_2ND_BYTE_OF_PACKET_HEADER;
	header[2] = DEFAULT_3RD_BYTE_OF_PACKET_HEADER;
	header[3] = DEFAULT_4TH_BYTE_OF_PACKET_HEADER;
}

void changeStartIndicator(SEND_PACKET* sendPacket, bool isStart) {
	if (isStart)
		setPayloadUnitStartIndicator(sendPacket->header, 1);
	else
		setPayloadUnitStartIndicator(sendPacket->header, 0);
}

void setHeaderCounte(SEND_PACKET* sendPackett, int value) {
	setContinuityCounte(sendPackett->header, value);
}

int calcSizeSplitTextureSendInformation(int sizeTextureSend) {
	int sizeSendPacketList = (SIZE_OF_TS_PACKET_TEXTURE_SIZE + sizeTextureSend) / SIZE_OF_TS_PACKET_TEXTURE;// 2byte
	if ((SIZE_OF_TS_PACKET_TEXTURE_SIZE + sizeTextureSend) % SIZE_OF_TS_PACKET_TEXTURE != 0)
		sizeSendPacketList++;
	return sizeSendPacketList;
}

int splitTextureSendInformation(SEND_PACKET* sendPacket,const char* textureSendBuf, 
	int sizeTextureSend, int frame, int* counterHeader) {

	int size = (SIZE_OF_TS_PACKET_TEXTURE_SIZE + sizeTextureSend) / SIZE_OF_TS_PACKET_TEXTURE;// 2byte
	int headerContinuityCounte = *counterHeader;
	
	// first packet
	changeStartIndicator(&sendPacket[0], true);
	setHeaderCounte(&sendPacket[0] , headerContinuityCounte++);
	if (headerContinuityCounte > 15)
		headerContinuityCounte = 0;
	
	// payload
	sendPacket[0].frame = frame;
	sendPacket[0].counter = 0;
	memcpy(&sendPacket[0].texture, &sizeTextureSend, SIZE_OF_TS_PACKET_TEXTURE_SIZE);
	memcpy((sendPacket[0].texture + SIZE_OF_TS_PACKET_TEXTURE_SIZE), textureSendBuf, SIZE_OF_TS_PACKET_TEXTURE - SIZE_OF_TS_PACKET_TEXTURE_SIZE);

	// middlle
	for (int i = 1; i < size ; i++) {
		changeStartIndicator(&sendPacket[i], false);
		setHeaderCounte(&sendPacket[i], headerContinuityCounte++);
		if (headerContinuityCounte > 15)
			headerContinuityCounte = 0;
		sendPacket[i].frame = frame;
		sendPacket[i].counter = i;
		memcpy(sendPacket[i].texture , textureSendBuf + SIZE_OF_TS_PACKET_TEXTURE * i - 2, SIZE_OF_TS_PACKET_TEXTURE);
	}

	// last packet
	int sizeOflastPacket = 0;
	if ((sizeOflastPacket = (SIZE_OF_TS_PACKET_TEXTURE_SIZE + sizeTextureSend) % SIZE_OF_TS_PACKET_TEXTURE)) {
		changeStartIndicator(&sendPacket[size], false);
		setHeaderCounte(&sendPacket[size], headerContinuityCounte++);
		if (headerContinuityCounte > 15)
			headerContinuityCounte = 0;
		sendPacket[size].frame = frame;
		sendPacket[size].counter = size;
		memcpy(sendPacket[size].texture, textureSendBuf + SIZE_OF_TS_PACKET_TEXTURE * size - SIZE_OF_TS_PACKET_TEXTURE_SIZE, sizeOflastPacket);
		size++;
	}
	*counterHeader = headerContinuityCounte;
	return size;
}
void initReceivePacketList(RECEIVE_PACKET* receivePacketList, std::list<RECEIVE_INDEX>* indexList, int rcvTextureBufSize) {
	if (!receivePacketList)
		return;
	if (!indexList)
		return;

	// TSパケット受信リスト初期化
	if(indexList->size()!= SIZE_OF_RECEIVEPACKET_LIST)
		indexList->resize(SIZE_OF_RECEIVEPACKET_LIST);

	std::list<RECEIVE_INDEX>::iterator it = indexList->begin();
	int count = 0;
	while (it != indexList->end()) {
		receivePacketList[count].texture = (unsigned char*)malloc(rcvTextureBufSize * sizeof(unsigned char));
		(*it).index = count++;
		(*it).isUsing = false;
		it++;
	}
}

void copySendPacketToBuffer(unsigned char* dist, SEND_PACKET* sendPacket) {
	if (!dist || !sendPacket)
		return;
	int offset = 0;
	int copySize = SIZE_OF_TS_PACKET_HEADER;// header(4byte)
	memcpy(dist, sendPacket->header, copySize);

	offset += copySize;
	copySize = sizeof(unsigned int);// frame(4byte)
	memcpy(dist + offset, (unsigned char*)&sendPacket->frame, copySize);

	offset += copySize;
	copySize = sizeof(unsigned char);// counter(1byte)
	memcpy(dist + offset, &sendPacket->counter, copySize);

	offset += copySize;
	copySize = SIZE_OF_TS_PACKET_TEXTURE;// texture(179byte)
	memcpy(dist + offset, sendPacket->texture, copySize);
}

void finalReceivePacketList(RECEIVE_PACKET* receivePacketList) {
	if (!receivePacketList)
		return;
	
	for (int i = 0; i < SIZE_OF_RECEIVEPACKET_LIST; i++) {
		free(receivePacketList[i].texture);
	}
}

int concatTextureReceiveInformation(RECEIVE_PACKET* receivePacketList, unsigned char* textureReceiveBuf,
	std::list<RECEIVE_INDEX>* indexList, unsigned char* rcvBuffer) {
	
	unsigned char* tsHeader = &textureReceiveBuf[0];
	unsigned char* tsData = &textureReceiveBuf[SIZE_OF_TS_PACKET_HEADER];
	int PID = 0;
	int payloadUnitStartIndicator = 0;// 先頭パケットフラグ
	unsigned int frame = 0;
	unsigned char counter = 0;
	unsigned short receiveSize = 0;// textureサイズ
	int packetSize = 0; // 計算されたパケット数

	// indexListサイズ確認
	if (indexList->size() != SIZE_OF_RECEIVEPACKET_LIST) 
		return ERR_CONCAT_LIST_SIZE;

	// HeaderからPIDを読み込み、PID==0x40の比較
	if (getPID(tsHeader, &PID) || PID != PID_OF_TS_PACKET)
		return ERR_CONCAT_PID;

	// payload_unit_start_indicatorの値を確認
	if (getPayloadUnitStartIndicator(tsHeader, &payloadUnitStartIndicator))
		return ERR_CONCAT_PAYLOAD;

	// 新たなフレームが先頭パケットが受信される
	if (payloadUnitStartIndicator == 1) {
		// パケットのdata部分からsizeを読み込む
		memcpy(&receiveSize, tsData + SIZE_OF_TS_PACKET_PAYLOAD_FRAME + SIZE_OF_TS_PACKET_PAYLOAD_COUNTER, SIZE_OF_TS_PACKET_TEXTURE_SIZE);
		packetSize = (SIZE_OF_TS_PACKET_TEXTURE_SIZE + receiveSize) / SIZE_OF_TS_PACKET_TEXTURE;
		if ((SIZE_OF_TS_PACKET_TEXTURE_SIZE + receiveSize) % SIZE_OF_TS_PACKET_TEXTURE)
			packetSize++;
	}

	// Data部分からSendPacketのframeとSendPacketのcounterを読み込む
	memcpy(&frame, tsData, SIZE_OF_TS_PACKET_PAYLOAD_FRAME);
	memcpy(&counter, tsData + SIZE_OF_TS_PACKET_PAYLOAD_FRAME, SIZE_OF_TS_PACKET_PAYLOAD_COUNTER);

	// 挿入インデックス検索処理
	std::list<RECEIVE_INDEX>::iterator insertItr = indexList->end();// 挿入位置（frameあり）
	std::list<RECEIVE_INDEX>::iterator unusedItr = indexList->end();// はじめの未使用位置
	std::list<RECEIVE_INDEX>::iterator nextIndex = indexList->end();// 挿入位置（frame無し）

	std::list<RECEIVE_INDEX>::iterator it = indexList->begin();

	for (int i = 0; i < indexList->size(); i++) {
		if ((*it).isUsing == true) {
			if (frame == receivePacketList[(*it).index].frame) {
				insertItr = it;
				break;
			}
			else if(frame < receivePacketList[(*it).index].frame && nextIndex == indexList->end()) {
				nextIndex = it;
			}
		}
		else {
			if (unusedItr == indexList->end()) {
				unusedItr = it;
			}
		}
		++it;
	}

	// 挿入インデックスがないを確認
	if (insertItr == indexList->end()) {
		if (unusedItr == indexList->end()) {
			// frame番号はリスト中全てframe番号と比べて小さい
			if (nextIndex == indexList->begin()) 
				return ERR_CONCAT_FRAME_SKIP;

			// add front of list to back
			RECEIVE_INDEX receiveIndex;
			receiveIndex.index = (*indexList->begin()).index;
			receiveIndex.isUsing = true;
			indexList->pop_front();
			if (nextIndex != indexList->end()) {
				insertItr = indexList->insert(nextIndex, receiveIndex);
			}
			else {
				indexList->push_back(receiveIndex);
				insertItr = indexList->end();
				insertItr--;
			}

			// insertIndexのReceivePacketを0でリセット
			receivePacketList[receiveIndex.index].frame = 0;
			receivePacketList[receiveIndex.index].counter = 0;
			receivePacketList[receiveIndex.index].size = 0;
			receivePacketList[receiveIndex.index].number = 0;
		}
		else {

			if (nextIndex != indexList->end()) {
				RECEIVE_INDEX receiveIndex;
				receiveIndex.index = (*unusedItr).index;
				receiveIndex.isUsing = true;
				insertItr = indexList->insert(nextIndex, receiveIndex);
				indexList->erase(unusedItr);
			}
			else {
				insertItr = unusedItr;
				(*insertItr).isUsing = true;
			}
		}
		receivePacketList[(*insertItr).index].frame = frame;
	}
	// insertIndexエラー
	int insertIndex = (*insertItr).index;
	if (insertIndex > SIZE_OF_RECEIVEPACKET_LIST || insertIndex < 0)
		return ERR_CONCAT_INDEX;
	
	// 先頭パケット
	if (payloadUnitStartIndicator) {
		// パケット数をReceivePacketの変数numberにセット
		receivePacketList[insertIndex].size = receiveSize;
		receivePacketList[insertIndex].number = static_cast<unsigned short>(packetSize);
	}
	
	// texture追加
	if (counter == 0)
		memcpy(receivePacketList[insertIndex].texture, tsData + SIZE_OF_TS_PACKET_PAYLOAD_FRAME + SIZE_OF_TS_PACKET_PAYLOAD_COUNTER + SIZE_OF_TS_PACKET_TEXTURE_SIZE, SIZE_OF_TS_PACKET_TEXTURE - SIZE_OF_TS_PACKET_TEXTURE_SIZE);// frame(4)+counter(1)+size(2)
	else
		memcpy(receivePacketList[insertIndex].texture + SIZE_OF_TS_PACKET_TEXTURE * counter - SIZE_OF_TS_PACKET_TEXTURE_SIZE, tsData + SIZE_OF_TS_PACKET_PAYLOAD_FRAME + SIZE_OF_TS_PACKET_PAYLOAD_COUNTER, SIZE_OF_TS_PACKET_TEXTURE);

	// ReceivePacketの変数counterインクリメント
	receivePacketList[insertIndex].counter++;
#if DEBUG_OUTPUT_SEND_RECEIVE_TEXTURE_ERROR
	if (counter_before + 1 != number_before) {
		if (counter_before + 1 != counter) {
			std::cout << "TSPacket: " << frame_before << "-" << (int)counter_before;
			std::cout << " -> " << frame << "-" << (int)counter << std::endl;
		}
	}
	else {
		if (frame_before + 1 != frame) {
			std::cout << "TSPacket: " << frame_before << "-" << (int)counter_before;
			std::cout << " -> " << frame << "-" << (int)counter << std::endl;
		}
	}
	counter_before = counter;
	frame_before = frame;
	number_before = receivePacketList[insertIndex].number;
#endif
	if (receivePacketList[insertIndex].number) {
		// すべてパケット受信できたかチェック
		if (receivePacketList[insertIndex].number == receivePacketList[insertIndex].counter) {
			int copySize = receivePacketList[insertIndex].size;
			// 解凍処理用のバッファにコピー
			memcpy(rcvBuffer, receivePacketList[insertIndex].texture, receivePacketList[insertIndex].size);

			// ReceivePacketカウント用変数をリセット
			receivePacketList[insertIndex].frame = 0;
			receivePacketList[insertIndex].counter = 0;
			receivePacketList[insertIndex].size = 0;
			receivePacketList[insertIndex].number = 0;

			// 該当するIndexを最後に移動
			RECEIVE_INDEX receiveIndex;
			receiveIndex.index = (*insertItr).index;
			indexList->push_back(receiveIndex);
			indexList->erase(insertItr);

			return copySize;
		}
	}

	return 0;
}
