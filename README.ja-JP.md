# TSPacket ライブラリ

テクスチャデータ送信機能を持つトランスポートストリーム（TS）パケット処理用のC++ライブラリです。

## 概要

このライブラリは以下の機能を提供します：
- MPEG-2トランスポートストリーム規格に従ったTSパケットヘッダーの作成・操作
- 大きなテクスチャデータを複数のTSパケットに分割して送信
- TSパケットを受信して完全なテクスチャデータに再組立
- パケットの順序管理と連続性制御

## 主要機能

### TSパケット構造
- **パケットサイズ**: 合計188バイト
- **ヘッダーサイズ**: 4バイト
- **ペイロードサイズ**: 184バイト
- **デフォルトPID**: 0x40
- **同期バイト**: 0x47

### ヘッダーフィールド関数
TSパケットヘッダーの全フィールドに対するgetter/setter関数を提供：
- 同期バイト
- 転送エラー指示子
- ペイロードユニット開始指示子
- 転送優先度
- PID（パケット識別子）
- 転送スクランブル制御
- 適応フィールド制御
- 連続性カウンター

## データ構造

### SEND_PACKET
送信パケット用構造体：
- 4バイトヘッダー
- フレーム番号（4バイト）
- パケットカウンター（1バイト）
- テクスチャデータ（179バイト）

### RECEIVE_PACKET
受信パケット用構造体：
- フレーム番号
- 受信済みパケット数
- テクスチャ全体サイズ
- 期待パケット数
- テクスチャデータバッファ

### RECEIVE_INDEX
パケット順序管理用インデックス構造体：
- 配列インデックス参照
- 使用中フラグ

## 主要関数

### 送信関数
```cpp
// テクスチャデータに必要なパケット数を計算
int calcSizeSplitTextureSendInformation(int sizeTextureSend);

// テクスチャデータを複数のTSパケットに分割
int splitTextureSendInformation(SEND_PACKET* sendPacket, 
                               const char* textureSendBuf, 
                               int sizeTextureSend, 
                               int frame, 
                               int* counterHeader);

// パケット構造体を送信用バッファにコピー
void copySendPacketToBuffer(unsigned char* dist, SEND_PACKET* sendPacket);
```

### 受信関数
```cpp
// 受信パケットリストとインデックス管理を初期化
void initReceivePacketList(RECEIVE_PACKET* receivePacketList, 
                          std::list<RECEIVE_INDEX>* indexList, 
                          int rcvTextureBufSize);

// 受信パケットを処理してテクスチャデータを再組立
int concatTextureReceiveInformation(RECEIVE_PACKET* receivePacketList, 
                                   unsigned char* textureReceiveBuf,
                                   std::list<RECEIVE_INDEX>* indexList, 
                                   unsigned char* rcvBuffer);
```

## エラーコード

| コード | 説明 |
|--------|------|
| -1 | ERR_CONCAT_LIST_SIZE - リストサイズ確認エラー |
| -2 | ERR_CONCAT_PID - PIDが0x40ではない |
| -3 | ERR_CONCAT_PAYLOAD - ペイロードフラグエラー |
| -4 | ERR_CONCAT_FRAME_SKIP - フレームスキップ検出 |
| -5 | ERR_CONCAT_INDEX - インデックスエラー |

## 使用例

### テクスチャデータ送信
1. 必要パケット数を計算
2. 送信パケットリストを初期化
3. テクスチャデータをパケットに分割
4. パケットを送信バッファにコピー

### テクスチャデータ受信
1. 受信パケットリストとインデックス管理を初期化
2. `concatTextureReceiveInformation`で受信パケットを処理
3. 完全なテクスチャが再組立されると正の値を返す
4. `finalReceivePacketList`でクリーンアップ

## 設定

- **受信パケットリストサイズ**: 10パケット（SIZE_OF_RECEIVEPACKET_LISTで設定可能）
- **デバッグ出力**: DEBUG_OUTPUT_SEND_RECEIVE_TEXTURE_ERRORマクロで有効/無効切替

## 依存関係

- 標準C++ライブラリ
- `<string.h>` メモリ操作用
- `<list>` STLリストコンテナ用
