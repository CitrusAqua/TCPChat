#pragma once

#include <stdint.h>

#define DESENCRY 0x0
#define DESDECRY 0x1

class DES {
private:
	uint32_t g_outkey[16][2];
	uint32_t m_arrBufKey[2];
	int32_t HandleData(uint32_t *left, uint8_t choice);
	int32_t MakeData(uint32_t  *left, uint32_t  *right, uint32_t number);
	int32_t MakeKey(uint32_t *keyleft, uint32_t *keyright, uint32_t number);
	int32_t MakeFirstKey(uint32_t *keyP);
public:
	DES();
	~DES();
	int32_t Encry(char* pPlaintext, int nPlaintextLength, char *pCipherBuffer, int &nCipherBufferLength, char *pKey, int nKeyLength);
	int32_t Decry(char* pCipher, int nCipherBufferLength, char *pPlaintextBuffer, int &nPlaintextBufferLength, char *pKey, int nKeyLength);


	static const uint8_t ip[64];
	static const uint8_t fp[64];
	static const uint32_t pc_by_bit[64];
	static const uint8_t des_P[32];
	static const uint8_t des_E[48];
	static const uint8_t des_S[8][64];
	static const uint8_t keyleft[28];
	static const uint8_t keyright[28];
	static const uint8_t lefttable[16];
	static const uint8_t keychoose[48];


};



