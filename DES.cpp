#include "DES.h"
#include <cstring>

int32_t DES::HandleData(uint32_t * left, uint8_t choice)
{
	uint32_t *right = &left[0];
	uint32_t oldright = *right;
	uint32_t tmpbuf[2] = { 0 };

	int32_t number = 0, j = 0;
	uint32_t tmp = 0;

	for (j = 0; j < 64; j++) {
		if (j < 32) {
			if (ip[j] > 32) {
				if (*right&pc_by_bit[ip[j] - 1]) {
					tmpbuf[0] |= pc_by_bit[j];
				}
			}
			else {
				if (*left&pc_by_bit[ip[j] - 1]) {
					tmpbuf[0] |= pc_by_bit[j];
				}
			}
		}
		else {
			if (ip[j] > 32) {
				if (*right&pc_by_bit[ip[j] - 1]) {
					tmpbuf[1] |= pc_by_bit[j];
				}
			}
			else {
				if (*left&pc_by_bit[ip[j] - 1]) {
					tmpbuf[1] |= pc_by_bit[j];
				}
			}
		}
	}
	*left = tmpbuf[0];
	*right = tmpbuf[1];


	for (int i = 0; i < 16; i++)
		MakeData(left, right, i);


	for (j = 0; j < 64; j++) {
		if (j < 32) {
			if (fp[j] > 32) {
				if (*right&pc_by_bit[fp[j] - 1]) {
					tmpbuf[0] |= pc_by_bit[j];
				}
			}
			else {
				if (*left&pc_by_bit[fp[j] - 1]) {
					tmpbuf[0] |= pc_by_bit[j];
				}
			}
		}
		else {
			if (fp[j] > 32) {
				if (*right&pc_by_bit[fp[j] - 1]) {
					tmpbuf[1] |= pc_by_bit[j];
				}
			}
			else {
				if (*left&pc_by_bit[fp[j] - 1]) {
					tmpbuf[1] |= pc_by_bit[j];
				}
			}
		}
	}
	*left = tmpbuf[0];
	*right = tmpbuf[1];


}

int32_t DES::MakeData(uint32_t * left, uint32_t * right, uint32_t number)
{
	uint32_t oldright = *right;
	uint32_t exdes_P[2] = { 0 };
	for (int j = 0; j < 48; j++) {
		if (j < 24) {
			if (*right&pc_by_bit[des_E[j] - 1]) {
				exdes_P[0] |= pc_by_bit[j];
			}
		}
		else {
			if (*right&pc_by_bit[des_E[j] - 1]) {
				exdes_P[1] |= pc_by_bit[j - 24];
			}
		}
	}
	for (int j = 0; j < 2; j++) {
		exdes_P[j] ^= g_outkey[number][j];
	}

	uint8_t rexpbuf[8];
	exdes_P[1] >>= 8;
	rexpbuf[7] = (uint8_t)(exdes_P[1] & 0x0000003fL);
	exdes_P[1] >>= 6;
	rexpbuf[6] = (uint8_t)(exdes_P[1] & 0x0000003fL);
	exdes_P[1] >>= 6;
	rexpbuf[5] = (uint8_t)(exdes_P[1] & 0x0000003fL);
	exdes_P[1] >>= 6;
	rexpbuf[4] = (uint8_t)(exdes_P[1] & 0x0000003fL);
	exdes_P[0] >>= 8;
	rexpbuf[3] = (uint8_t)(exdes_P[0] & 0x0000003fL);
	exdes_P[0] >>= 6;
	rexpbuf[2] = (uint8_t)(exdes_P[0] & 0x0000003fL);
	exdes_P[0] >>= 6;
	rexpbuf[1] = (uint8_t)(exdes_P[0] & 0x0000003fL);
	exdes_P[0] >>= 6;
	rexpbuf[0] = (uint8_t)(exdes_P[0] & 0x0000003fL);
	exdes_P[0] = 0;
	exdes_P[1] = 0;
	*right = 0;
	for (int j = 0; j < 7; j++) {
		*right |= des_S[j][rexpbuf[j]];
		*right <<= 4;
	}
	*right |= des_S[7][rexpbuf[7]];
	uint32_t datatmp = 0;
	for (int j = 0; j < 32; j++) {
		if (*right&pc_by_bit[des_P[j] - 1]) {
			datatmp |= pc_by_bit[j];
		}
	}
	*right = datatmp;
	*right ^= *left;
	*left = oldright;
	return 0;
}

int32_t DES::MakeKey(uint32_t * keyleft, uint32_t * keyright, uint32_t number)
{
	uint32_t *Poutkey = (uint32_t*)&g_outkey[number];

	*keyleft = (*keyleft << lefttable[number]) | (*keyleft >> (28 - lefttable[number]));
	*keyright = (*keyright << lefttable[number]) | (*keyright >> (28 - lefttable[number]));
	*keyleft &= 0xfffffff0;
	*keyright &= 0xfffffff0;

	for (int j = 0; j < 48; j++) {
		if (j < 24) {
			if (*keyleft&pc_by_bit[keychoose[j] - 1]) {
				Poutkey[0] |= pc_by_bit[j];
			}
		}
		else /*j>=24*/ {
			if (*keyright&pc_by_bit[(keychoose[j] - 28)]) {
				Poutkey[1] |= pc_by_bit[j - 24];
			}
		}
	}

	return 0;
}

int32_t DES::MakeFirstKey(uint32_t * keyP)
{

	uint32_t left, right;
	uint32_t * pleft, *pright;
	pleft = &left;
	pright = &right;
	uint32_t firstkey[2] = { 0 };

	for (int i = 0; i < 28; i++) {
		if (*pleft&pc_by_bit[keyleft[i] - 1]) {
			firstkey[0] |= pc_by_bit[i];
		}
		if (*pright&pc_by_bit[keyright[i] - 1]) {
			firstkey[1] |= pc_by_bit[i];
		}
	}

	for (int i = 0; i < 16; i++) {
		MakeKey(&firstkey[0], &firstkey[1], i);
	}


	return 0;
}

DES::DES()
{
	memset(g_outkey, 0, sizeof(uint32_t) * 32);
	memset(m_arrBufKey, 0, sizeof(uint32_t) * 2);
}

DES::~DES()
{
}

int32_t DES::Encry(char * pPlaintext, int nPlaintextLength, char * pCipherBuffer, int & nCipherBufferLength, char * pKey, int nKeyLength)
{
	if (nKeyLength != 8) {
		return 0;
	}
	MakeFirstKey((uint32_t *)pKey);
	int nLenthofLong = ((nPlaintextLength + 7) / 8) * 2;
	if (nCipherBufferLength < nLenthofLong * 4) {
		//out put buffer is not enough
		nCipherBufferLength = nLenthofLong * 4;
		return 0;
	}
	memset(pCipherBuffer, 0, nCipherBufferLength);
	uint32_t *pOutPutSpace = (uint32_t *)pCipherBuffer;
	uint32_t * pSource;
	if (nPlaintextLength != sizeof(uint32_t)*nLenthofLong) {
		pSource = new uint32_t[nLenthofLong];
		memset(pSource, 0, sizeof(uint32_t)*nLenthofLong);
		memcpy(pSource, pPlaintext, nPlaintextLength);
	}
	else {
		pSource = (uint32_t *)pPlaintext;
	}

	uint32_t gp_msg[2] = {0,0};

	for (int i = 0; i < (nLenthofLong / 2); i++) {
		gp_msg[0] = pSource[2 * i];
		gp_msg[1] = pSource[2 * i + 1];
		HandleData(gp_msg, DESENCRY);
		pOutPutSpace[2 * i] = gp_msg[0];
		pOutPutSpace[2 * i + 1] = gp_msg[1];
	}
	if (pPlaintext != (char *)pSource) {
		delete[]pSource;
	}
	return 0;
}

int32_t DES::Decry(char * pCipher, int nCipherBufferLength, char * pPlaintextBuffer, int & nPlaintextBufferLength, char * pKey, int nKeyLength)
{
	if (nKeyLength != 8) {
		return 0;
	}
	MakeFirstKey((uint32_t *)pKey);
	int nLenthofLong = ((nCipherBufferLength + 7) / 8) * 2;
	if (nPlaintextBufferLength < nLenthofLong * 4) {
		//out put buffer is not enough
		nPlaintextBufferLength = nLenthofLong * 4;
		return 0;
	}
	memset(pPlaintextBuffer, 0, nPlaintextBufferLength);
	uint32_t *pOutPutSpace = (uint32_t *)pPlaintextBuffer;
	uint32_t * pSource;
	if (nCipherBufferLength != sizeof(uint32_t)*nLenthofLong) {
		pSource = new uint32_t[nLenthofLong];
		memset(pSource, 0, sizeof(uint32_t)*nLenthofLong);
		memcpy(pSource, pCipher, nCipherBufferLength);
	}
	else {
		pSource = (uint32_t *)pCipher;
	}

	uint32_t gp_msg[2] = { 0,0 };

	for (int i = 0; i < (nLenthofLong / 2); i++) {
		gp_msg[0] = pSource[2 * i];
		gp_msg[1] = pSource[2 * i + 1];
		HandleData(gp_msg, DESENCRY);
		pOutPutSpace[2 * i] = gp_msg[0];
		pOutPutSpace[2 * i + 1] = gp_msg[1];
	}
	if (pCipher != (char *)pSource) {
		delete[]pSource;
	}
	return 0;
}
