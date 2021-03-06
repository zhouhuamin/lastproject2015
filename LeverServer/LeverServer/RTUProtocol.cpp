/* 
 * File:   RTUProtocol.cpp
 * Author: root
 * 
 * Created on 2015年1月31日, 下午4:29
 */
#include <string.h>
#include <stdio.h>
#include "RTUProtocol.h"
//字地址 0 - 255 (只取低8位)   
//位地址 0 - 255 (只取低8位)   

/* CRC 高位字节值表 */    
const uint8 auchCRCHi[] = {    
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,    
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,    
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,    
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,    
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,    
	0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,    
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,    
	0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,    
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,    
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,    
	0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,    
	0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,    
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,    
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,    
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,    
	0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,    
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,    
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,    
	0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,    
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,    
	0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,    
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40,    
	0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1,    
	0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,    
	0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,    
	0x80, 0x41, 0x00, 0xC1, 0x81, 0x40    
} ;    
/* CRC低位字节值表*/    
const uint8 auchCRCLo[] = {    
	0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06,    
	0x07, 0xC7, 0x05, 0xC5, 0xC4, 0x04, 0xCC, 0x0C, 0x0D, 0xCD,    
	0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,    
	0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A,    
	0x1E, 0xDE, 0xDF, 0x1F, 0xDD, 0x1D, 0x1C, 0xDC, 0x14, 0xD4,    
	0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,    
	0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3,    
	0xF2, 0x32, 0x36, 0xF6, 0xF7, 0x37, 0xF5, 0x35, 0x34, 0xF4,    
	0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,    
	0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29,    
	0xEB, 0x2B, 0x2A, 0xEA, 0xEE, 0x2E, 0x2F, 0xEF, 0x2D, 0xED,    
	0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,    
	0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60,    
	0x61, 0xA1, 0x63, 0xA3, 0xA2, 0x62, 0x66, 0xA6, 0xA7, 0x67,    
	0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,    
	0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68,    
	0x78, 0xB8, 0xB9, 0x79, 0xBB, 0x7B, 0x7A, 0xBA, 0xBE, 0x7E,    
	0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,    
	0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71,    
	0x70, 0xB0, 0x50, 0x90, 0x91, 0x51, 0x93, 0x53, 0x52, 0x92,    
	0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,    
	0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B,    
	0x99, 0x59, 0x58, 0x98, 0x88, 0x48, 0x49, 0x89, 0x4B, 0x8B,    
	0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,    
	0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42,    
	0x43, 0x83, 0x41, 0x81, 0x80, 0x40    
} ;   


uint16 RTUProtocol::crc16(BYTE *puchMsg, uint16 usDataLen)    
{    
	uint8 uchCRCHi = 0xFF ; /* 高CRC字节初始化 */    
	uint8 uchCRCLo = 0xFF ; /* 低CRC 字节初始化 */    
	uint32 uIndex ; /* CRC循环中的索引 */    
	while (usDataLen--) /* 传输消息缓冲区 */    
	{    
		uIndex = uchCRCHi ^ *puchMsg++ ; /* 计算CRC */    
		uchCRCHi = uchCRCLo ^ auchCRCHi[uIndex] ;    
		uchCRCLo = auchCRCLo[uIndex] ;    
	}    
	return (uchCRCHi << 8 | uchCRCLo) ;    
}//uint16 crc16(uint8 *puchMsg, uint16 usDataLen)  



using namespace std;
RTUProtocol::RTUProtocol()
{
}

RTUProtocol::RTUProtocol(const RTUProtocol& orig)
{
}

RTUProtocol::~RTUProtocol()
{
}

unsigned short RTUProtocol::CalCRC(const std::vector<BYTE> &dataVect)
{
	if (dataVect.size() <= 0)
		return 0;
	//int nCRC = 0;
	//for (int i = 0; i < dataVect.size(); ++i)
	//	nCRC += dataVect[i];
	//return nCRC;

	return crc16(const_cast<BYTE *>(&dataVect[0]), (uint16)dataVect.size());
}


void RTUProtocol::ReadRegisterRequest(std::vector<BYTE> &dataVect, BYTE bySeqNo, BYTE byRegCount)
{
	BYTE ADDR	= 0x01;
	BYTE CMD	= 0x03;

	WORD dwReg_addr = (WORD)1300 + (WORD)bySeqNo;
	BYTE Reg_addr[2 + 1] 	= {HIBYTE(dwReg_addr), LOBYTE(dwReg_addr)};
	BYTE Reg_len[2 + 1]     = {0x00, byRegCount};

	dataVect.push_back(ADDR);
	dataVect.push_back(CMD);
	dataVect.push_back(Reg_addr[0]);
	dataVect.push_back(Reg_addr[1]);
	dataVect.push_back(Reg_len[0]);
	dataVect.push_back(Reg_len[1]);

	WORD nCRC = (WORD)CalCRC(dataVect);
	
	dataVect.push_back(HIBYTE(nCRC));
	dataVect.push_back(LOBYTE(nCRC));
}

bool RTUProtocol::ReadRegisterResponse(const std::vector<BYTE> &dataVect, BYTE Length)
{
	BYTE ADDR	= 0x01;
	BYTE CMD	= 0x03;

	if (dataVect[0] == ADDR)
		;
	else
		return false;
	if (dataVect[1] == CMD)
		;
	else
		return false;

	if (dataVect[2] == Length)
		;
	else
		return false;

	WORD Reg1 = 0x00;
	WORD Reg2 = 0x00;
	WORD Reg3 = 0x00;
	WORD Reg4 = 0x00;

	if (Length >= 2)
		Reg1 = MAKEWORD(dataVect[4], dataVect[3]);
	if (Length >= 4)
		Reg2 = MAKEWORD(dataVect[6], dataVect[5]);
	if (Length >= 6)
		Reg3 = MAKEWORD(dataVect[8], dataVect[7]);
	if (Length >= 8)
		Reg4 = MAKEWORD(dataVect[10], dataVect[9]);

	WORD nCRC = (WORD)crc16(const_cast<BYTE *>(&dataVect[0]), (uint16)dataVect.size() - 2);

	//WORD wCRC = MAKEWORD(dataVect[11], dataVect[12]);
	WORD wCRC = MAKEWORD(dataVect[12], dataVect[11]);


	if (nCRC == wCRC)
		;
	else
		return false;

	return true;
}

void RTUProtocol::WriteRegisterRequest(std::vector<BYTE> &dataVect)
{
	BYTE ADDR	= 0x01;
	BYTE CMD	= 0x10;
	BYTE Reg_addr[2 + 1] = {0x03, 0xFF};
	BYTE Reg_len[2 + 1]	 = {0x00, 0x01};
	BYTE Len	= 0x02;
	BYTE Reg1[2 + 1]	 = {0x00, 0x03};
	//BYTE Reg2[2 + 1]	 = {0x00, 0x01};
	//BYTE Reg3[2 + 1]	 = {0x00, 0x01};
	//BYTE Reg4[2 + 1]	 = {0x00, 0x01};

	dataVect.push_back(ADDR);
	dataVect.push_back(CMD);
	dataVect.push_back(Reg_addr[0]);
	dataVect.push_back(Reg_addr[1]);
	dataVect.push_back(Reg_len[0]);
	dataVect.push_back(Reg_len[1]);
	dataVect.push_back(Len);

	dataVect.push_back(Reg1[0]);
	dataVect.push_back(Reg1[1]);

	//dataVect.push_back(Reg2[0]);
	//dataVect.push_back(Reg2[1]);

	//dataVect.push_back(Reg3[0]);
	//dataVect.push_back(Reg3[1]);

	//dataVect.push_back(Reg4[0]);
	//dataVect.push_back(Reg4[1]);

	WORD nCRC = (WORD)CalCRC(dataVect);

	
	dataVect.push_back(HIBYTE(nCRC));
	dataVect.push_back(LOBYTE(nCRC));
	
}

bool RTUProtocol::WriteRegisterResponse(const std::vector<BYTE> &dataVect)
{
	BYTE ADDR	= 0x01;
	BYTE CMD	= 0x10;
	BYTE Reg_addr[2 + 1] = {0};
	BYTE Reg_len[2 + 1]	 = {0x00, 0x04};
	BYTE Len	= 0x08;
	BYTE Reg1[2 + 1]	 = {0x00, 0x00};
	BYTE Reg2[2 + 1]	 = {0x00, 0x00};
	BYTE Reg3[2 + 1]	 = {0x00, 0x00};
	BYTE Reg4[2 + 1]	 = {0x00, 0x00};
	if (dataVect[0] == ADDR)
		;
	else
		return false;
	if (dataVect[1] == CMD)
		;
	else
		return false;
	if (dataVect[2]	== Reg_addr[0])
		;
	else
		return false;
	if (dataVect[3]	== Reg_addr[1])
		;
	else
		return false;

	if (dataVect[4]	== Reg_len[0])
		;
	else
		return false;
	if (dataVect[5]	== Reg_len[1])
		;
	else
		return false;

	WORD nCRC = (WORD)crc16(const_cast<BYTE *>(&dataVect[0]), (uint16)dataVect.size() - 2);


	if (dataVect[6]	== HIBYTE(nCRC))
		;
	else
		return false;

	if (dataVect[7]	== LOBYTE(nCRC))
		;
	else
		return false;
	return true;
}

bool RTUProtocol::ReadUploadData(const std::vector<BYTE> &dataVect, std::pair<BYTE, structUploadData> &pairData)
{
	BYTE ADDR	= 0x01;
	BYTE CMD	= 0x53;

	if (dataVect[0] == ADDR)
		;
	else
	{
		//printf("1111111");
		return false;
	}
	if (dataVect[1] == CMD)
		;
	else
	{
		//printf("222222");
		return false;
	}

	structUploadData upLoadData = {0};
	if (dataVect.size() >= 8)
		memcpy(&upLoadData, &dataVect[2], sizeof(structUploadData));

	


	WORD nCRC = (WORD)crc16(const_cast<BYTE *>(&dataVect[0]), (uint16)dataVect.size() - 2);

	//printf("nCRCXXXXX=%X\n", nCRC);

	if (dataVect[8]	== HIBYTE(nCRC))
		;
	else
	{
		//printf("333333");
		return false;
	}

	if (dataVect[9]	== LOBYTE(nCRC))
		;
	else
	{
		//printf("444444");
		return false;
	}

	pairData = make_pair(upLoadData.IDX, upLoadData);
	return true;
}

void RTUProtocol::ReadInputDiscreteRequest(std::vector<BYTE> &dataVect)
{
	BYTE ADDR	= 0x01;
	BYTE CMD	= 0x02;
	BYTE DATA[4 + 1] = {0x00, 0x00, 0x00, 0x08, 0x00};
	
	dataVect.push_back(ADDR);
	dataVect.push_back(CMD);
	dataVect.push_back(DATA[0]);
	dataVect.push_back(DATA[1]);
	dataVect.push_back(DATA[2]);
	dataVect.push_back(DATA[3]);
	
	WORD nCRC = (WORD)CalCRC(dataVect);
		
	dataVect.push_back(HIBYTE(nCRC));
	dataVect.push_back(LOBYTE(nCRC));
}

bool RTUProtocol::ReadInputDiscreteResponse(const std::vector<BYTE> &dataVect)
{
	BYTE ADDR	= 0x01;
	BYTE CMD	= 0x02;
	BYTE DATA[2 + 1] = {0};

	if (dataVect[0] == ADDR)
		;
	else
		return false;
	if (dataVect[1] == CMD)
		;
	else
		return false;
	
	WORD nCRC = (WORD)crc16(const_cast<BYTE *>(&dataVect[0]), (uint16)dataVect.size() - 2);
	
	
	if (dataVect[4]	== HIBYTE(nCRC))
		;
	else
		return false;
	
	if (dataVect[5]	== LOBYTE(nCRC))
		;
	else
		return false;
	return true;
}

void RTUProtocol::WriteCoilRequestRising(std::vector<BYTE> &dataVect, BYTE bM, BYTE CH)
{
	BYTE ADDR	= 0x01;
	BYTE CMD	= 0x0F;
	//BYTE bM		= 0x08;
	BYTE LEN	= 0x04;
	//BYTE CH		= 0x07;
	WORD wTime	= 500; //2000;	// 2015.3.27
	BYTE high	= HIBYTE(wTime);
	BYTE low	= LOBYTE(wTime);
	BYTE val	= 0x01;

	BYTE DATA[9 + 1] = {0x02, 0x00, 0x00, bM, LEN, CH, high, low, val, 0x00};

	dataVect.push_back(ADDR);
	dataVect.push_back(CMD);
	dataVect.push_back(DATA[0]);
	dataVect.push_back(DATA[1]);
	dataVect.push_back(DATA[2]);
	dataVect.push_back(DATA[3]);
	dataVect.push_back(DATA[4]);
	dataVect.push_back(DATA[5]);
	dataVect.push_back(DATA[6]);
	dataVect.push_back(DATA[7]);
	dataVect.push_back(DATA[8]);

	WORD nCRC = (WORD)CalCRC(dataVect);

	dataVect.push_back(HIBYTE(nCRC));
	dataVect.push_back(LOBYTE(nCRC));
}

bool RTUProtocol::WriteCoilResponse(const std::vector<BYTE> &dataVect)
{
	BYTE ADDR	= 0x01;
	BYTE CMD	= 0x0F;
	BYTE bM		= 0x08;
	BYTE DATA[4 + 1] = {0x02, 0x00, 0x00, bM, 0x00};

	if (dataVect[0] == ADDR)
		;
	else
		return false;
	if (dataVect[1] == CMD)
		;
	else
		return false;

	WORD nCRC = (WORD)crc16(const_cast<BYTE *>(&dataVect[0]), (uint16)dataVect.size() - 2);


	if (dataVect[4]	== HIBYTE(nCRC))
		;
	else
		return false;

	if (dataVect[5]	== LOBYTE(nCRC))
		;
	else
		return false;
	return true;
}

void RTUProtocol::WriteCoilRequestOff(std::vector<BYTE> &dataVect, BYTE bM, BYTE CH)
{
	BYTE ADDR	= 0x01;
	BYTE CMD	= 0x0F;
	// BYTE bM		= 0x08;
	BYTE LEN	= 0x04;
	// BYTE CH		= 0x08;
	WORD wTime	= 500; // 2000;	//2015-3-27
	BYTE high	= HIBYTE(wTime);
	BYTE low	= LOBYTE(wTime);
	BYTE val	= 0x01;

	BYTE DATA[9 + 1] = {0x02, 0x00, 0x00, bM, LEN, CH, high, low, val, 0x00};

	dataVect.push_back(ADDR);
	dataVect.push_back(CMD);
	dataVect.push_back(DATA[0]);
	dataVect.push_back(DATA[1]);
	dataVect.push_back(DATA[2]);
	dataVect.push_back(DATA[3]);
	dataVect.push_back(DATA[4]);
	dataVect.push_back(DATA[5]);
	dataVect.push_back(DATA[6]);
	dataVect.push_back(DATA[7]);
	dataVect.push_back(DATA[8]);

	WORD nCRC = (WORD)CalCRC(dataVect);

	dataVect.push_back(HIBYTE(nCRC));
	dataVect.push_back(LOBYTE(nCRC));
}

void RTUProtocol::WriteRegisterRequest(std::vector<BYTE> &dataVect, BYTE val)
{
	BYTE ADDR	= 0x01;
	BYTE CMD	= 0x10;
	BYTE Reg_addr[2 + 1] = {0x03, 0xFE};
	BYTE Reg_len[2 + 1]	 = {0x00, 0x01};
	BYTE Len	= 0x02;
	BYTE Reg1[2 + 1]	 = {0x00, val};
	//BYTE Reg2[2 + 1]	 = {0x00, 0x01};
	//BYTE Reg3[2 + 1]	 = {0x00, 0x01};
	//BYTE Reg4[2 + 1]	 = {0x00, 0x01};

	dataVect.push_back(ADDR);
	dataVect.push_back(CMD);
	dataVect.push_back(Reg_addr[0]);
	dataVect.push_back(Reg_addr[1]);
	dataVect.push_back(Reg_len[0]);
	dataVect.push_back(Reg_len[1]);
	dataVect.push_back(Len);

	dataVect.push_back(Reg1[0]);
	dataVect.push_back(Reg1[1]);

	//dataVect.push_back(Reg2[0]);
	//dataVect.push_back(Reg2[1]);

	//dataVect.push_back(Reg3[0]);
	//dataVect.push_back(Reg3[1]);

	//dataVect.push_back(Reg4[0]);
	//dataVect.push_back(Reg4[1]);

	WORD nCRC = (WORD)CalCRC(dataVect);


	dataVect.push_back(HIBYTE(nCRC));
	dataVect.push_back(LOBYTE(nCRC));

}
