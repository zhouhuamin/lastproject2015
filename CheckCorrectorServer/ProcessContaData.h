/* 
 * File:   ProcessContaData.h
 * Author: root
 *
 * Created on 2015年6月8日, 下午3:19
 */

#ifndef PROCESSCONTADATA_H
#define	PROCESSCONTADATA_H

#include "SysMessage.h"
#include <vector>
#include <string>

typedef enum jz_enumBoxNumberDirection
{
	FRONT       = 0,
	FRONTLEFT   = 1,
	FRONTRIGHT  = 2,
	BACK        = 3,
	BACKLEFT    = 4,
	BACKRIGHT   = 5
}BoxNumberDirection;

struct structBoxNumberRecogResult
{
	std::string strSeqNo;
	std::string strBoxNumber;
	std::string strBoxModel;
	std::string strBoxColor;
	std::string strArrangement;
	BoxNumberDirection  direct;
};

struct structContaOwnerData
{
	int nLineNo;
	std::string strCompanyCode;
	std::string strCompanyName;
};

class ProcessContaData {
public:
    ProcessContaData();
    ProcessContaData(const ProcessContaData& orig);
    virtual ~ProcessContaData();


public:
	int GetContaIDCheckBit(JZ_ContaIDCheckBitRequest *pRequest,		JZ_ContaIDCheckBitResponse *pResponse);
	int GetContaIDCorrected(JZ_ContaIDCorrectionRequest *pRequest,	JZ_ContaIDCorrectionResponse *pResponse);
	int GetContaIDCount(JZ_ContaIDCountRequest *pRequest,			JZ_ContaCountResponse *pResponse);

private:
	int JudgeBoxType(const std::vector<structBoxNumberRecogResult> &boxNumberVect, JZ_ContaCountResponse *pResponse);
	int GetBoxNumberVect(std::vector<structBoxNumberRecogResult> &boxNumberVect, JZ_ContaIDCountRequest *pRequest);
	int BoxNumberCorrection(const std::vector<structBoxNumberRecogResult> &boxNumberSet, JZ_ContaIDCorrectionResponse *pResponse);
	int GetBoxNumberVect2(std::vector<structBoxNumberRecogResult> &boxNumberVect, JZ_ContaIDCorrectionRequest *pRequest);
	void	GetContaOwnerData(std::vector<structContaOwnerData> &contaOwnerVect);
};

#endif	/* PROCESSCONTADATA_H */

