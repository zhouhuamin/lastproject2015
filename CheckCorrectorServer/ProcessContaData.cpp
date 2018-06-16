/* 
 * File:   ProcessContaData.cpp
 * Author: root
 * 
 * Created on 2015年6月8日, 下午3:19
 */

#include "ProcessContaData.h"
#include "BoxNumberCheckAlgo.h"
#include "MyLog.h"
#include <string>
#include <vector>

ProcessContaData::ProcessContaData()
{
}

ProcessContaData::ProcessContaData(const ProcessContaData& orig)
{
}

ProcessContaData::~ProcessContaData()
{
}

int ProcessContaData::GetContaIDCheckBit(JZ_ContaIDCheckBitRequest *pRequest, JZ_ContaIDCheckBitResponse *pResponse)
{
	if (pRequest == NULL)
		return -1;
	CMyLog::m_pLog->_XGSysLog("recv checkbit req %s! \n", pRequest->contaID);

	if (pRequest->contaID[0] == '\0')
	{
		return -1;
	}

	char chVerifyCode		= ' ';
	std::string strContaID	= pRequest->contaID;
	BoxNumberCheckAlgo checkAlgo;
	checkAlgo.GetBoxNumCheckbit(strContaID, chVerifyCode);

	JZ_ContaIDCheckBitResponse response = {0};
	strncpy(response.contaID, strContaID.c_str(), strContaID.size());
	response.VerifyCode		= chVerifyCode;
	*pResponse = response;

	CMyLog::m_pLog->_XGSysLog("checkbit result %s-%c\n", strContaID.c_str(), chVerifyCode);
	return 0;
}

int ProcessContaData::GetContaIDCorrected(JZ_ContaIDCorrectionRequest *pRequest, JZ_ContaIDCorrectionResponse *pResponse)
{
	if (pRequest == NULL)
		return -1;
	CMyLog::m_pLog->_XGSysLog("recv contaid correction req!\n");
	std::vector<structBoxNumberRecogResult> boxNumberVect;
	GetBoxNumberVect2(boxNumberVect, pRequest);
	BoxNumberCorrection(boxNumberVect, pResponse);
	CMyLog::m_pLog->_XGSysLog("correction result\n");


	return 0;
}

int ProcessContaData::GetContaIDCount(JZ_ContaIDCountRequest *pRequest, JZ_ContaCountResponse *pResponse)
{
	if (pRequest == NULL)
		return -1;

	CMyLog::m_pLog->_XGSysLog("recv contaid count req!\n");
	std::vector<structBoxNumberRecogResult> boxNumberVect;
	GetBoxNumberVect(boxNumberVect, pRequest);
	JudgeBoxType(boxNumberVect, pResponse);
	CMyLog::m_pLog->_XGSysLog("count result %d\n", pResponse->nCount);

	return 0;
}

int ProcessContaData::JudgeBoxType(const std::vector<structBoxNumberRecogResult> &boxNumberVect, JZ_ContaCountResponse *pResponse)
{
	if (boxNumberVect.size() == 0)
	{
		pResponse->nCount = 0;
		return 0;
	}

	// 根据对射触发判断长短箱流程-----single double judge procedure
	int nBoxType = 0;   // 1:长箱   2:双箱  3:单箱 4:短箱

	bool bModel = false;
	for (int i = 0; i < boxNumberVect.size(); ++i)
	{
		//if (boxNumberSet[i].strBoxModel != "" && boxNumberSet[i].strBoxNumber.size() > 5 && boxNumberSet[i].strBoxNumber[4] == '2')
		if (boxNumberVect[i].strBoxModel != "" && boxNumberVect[i].strBoxModel[0] == '2')
		{
			bModel      = true;
			nBoxType    = 2;
			break;
		}
		//if (boxNumberSet[i].strBoxModel != "" && boxNumberSet[i].strBoxNumber.size() > 5 && boxNumberSet[i].strBoxNumber[4] == '4')
		if (boxNumberVect[i].strBoxModel != "" && boxNumberVect[i].strBoxModel[0] == '4')
		{
			bModel      = true;
			nBoxType    = 1;
			break;
		}            
	}

	if (bModel && nBoxType == 2)
	{
		// 确定是双箱或长箱
		pResponse->nCount = 2;
	}
	else if (bModel && nBoxType == 1)
	{
		// 确定是双箱或长箱
		pResponse->nCount = 1;
	}
	else
	{
		string strFront         = "";
		string strFrontRight    = "";
		string strBack          = "";
		string strBackLeft      = "";
		string strFrontLeft     = "";
		string strBackRight     = "";
		string strFrontRightArrange  = "";
		string strBackLeftArrange   = "";
		string strFrontLeftColor = "";
		string strBackLeftColor  = "";
		for (int i = 0; i < boxNumberVect.size(); ++i)
		{
			if (boxNumberVect[i].direct == FRONT)
				strFront				= boxNumberVect[i].strBoxNumber;
			if (boxNumberVect[i].direct == FRONTRIGHT)
			{
				strFrontRight			= boxNumberVect[i].strBoxNumber;  
				strFrontRightArrange	= boxNumberVect[i].strArrangement;
			}
			if (boxNumberVect[i].direct == BACK)
				strBack        = boxNumberVect[i].strBoxNumber;
			if (boxNumberVect[i].direct == BACKLEFT)
			{
				strBackLeft         =   boxNumberVect[i].strBoxNumber;   
				strBackLeftArrange  =   boxNumberVect[i].strArrangement;
				strBackLeftColor    =   boxNumberVect[i].strBoxColor;
			}
			if (boxNumberVect[i].direct == FRONTLEFT)
			{
				strFrontLeft   = boxNumberVect[i].strBoxNumber;
				strFrontLeftColor   =   boxNumberVect[i].strBoxColor;
			}
			if (boxNumberVect[i].direct == BACKRIGHT)
				strBackRight   = boxNumberVect[i].strBoxNumber;                
		}
		// 前箱号前右箱号后4位相同么 && 后箱号与后左箱号后4位相同么
		if (strFront.size() == 11 && strBack.size() == 11 && strFront.substr(7, 4) == strBack.substr(7, 4)  && strFrontRight.substr(7, 4) == strBackLeft.substr(7, 4) )
		{
			// 确定是长箱
			nBoxType = 1;
		}           
		else
		{
			// 比较前左、后右有一个识别出来了么
			if (strFrontLeft != "" || strBackRight != "")
			{
				// 确定是双箱
				nBoxType = 2;
			}
			else
			{
				// 比较前右、后左箱号 排列方式是否不同
				if (strFrontRightArrange != strBackLeftArrange)
				{
					// 确定是双箱
					nBoxType = 2;
				}
				else
				{
					// 比较前左，后左箱体颜色，不同且差距大么
					if (strFrontLeftColor != strBackLeftColor)
					{
						// 确定是双箱
						nBoxType = 2;
					}
					else
					{
						// 自认为是单箱
						nBoxType = 3;
					}
				}
			}
		}
	}

	//structBoxNumberRecogResultCorrected resultCorrected;
	// 能确定 是 单箱 、双箱 么
	if (nBoxType == 2 || nBoxType == 3)
	{
		// 单箱
		if (nBoxType == 3)
		{
			// 挑选出4副图片识别结果
			int nPicNumber = 4;
			pResponse->nCount = 1;
		}
		else if (nBoxType == 2)// 双箱
		{
			// 每个箱子至少三个识别结果
			int nPicNumber = 6;
			pResponse->nCount = 2;
		}
	}
	else if (nBoxType == 4)
	{
		// 挑选出4副图片识别结果
		int nPicNumber = 4;
		pResponse->nCount = 1;
	}
	else
	{
		// 按长箱处理
		nBoxType = 1;
		// 挑选出4副图片识别结果
		int nPicNumber = 4;
		pResponse->nCount = 1;

	}
	return 0;
}

int ProcessContaData::GetBoxNumberVect(std::vector<structBoxNumberRecogResult> &boxNumberVect, JZ_ContaIDCountRequest *pRequest)
{
	structBoxNumberRecogResult tmpBoxNumber;

	if (pRequest->FrontContaID.contaID[0] != '\0')
	{
		tmpBoxNumber.strBoxNumber	= pRequest->FrontContaID.contaID;
		tmpBoxNumber.strBoxModel	= pRequest->FrontContaID.Type;
		// RED, BLUE, WHITE, GRAY, GREEN, OTHER
		if (pRequest->FrontContaID.color	== RED)
			tmpBoxNumber.strBoxColor	=	"RED";
		if (pRequest->FrontContaID.color	== BLUE)
			tmpBoxNumber.strBoxColor	=	"BLUE";
		if (pRequest->FrontContaID.color	== WHITE)
			tmpBoxNumber.strBoxColor	=	"WHITE";
		if (pRequest->FrontContaID.color	== GRAY)
			tmpBoxNumber.strBoxColor	=	"GRAY";
		if (pRequest->FrontContaID.color	== GREEN)
			tmpBoxNumber.strBoxColor	=	"GREEN";
		if (pRequest->FrontContaID.color	== OTHER)
			tmpBoxNumber.strBoxColor	=	"OTHER";
		if (pRequest->FrontContaID.ali.Atype == H_Align)
			tmpBoxNumber.strArrangement = "H";
		if (pRequest->FrontContaID.ali.Atype == T_Align)
			tmpBoxNumber.strArrangement = "T";
		tmpBoxNumber.direct				= FRONT;
		boxNumberVect.push_back(tmpBoxNumber);
	}

	if (pRequest->FrontLeftContaID.contaID[0] != '\0')
	{
		tmpBoxNumber.strBoxNumber	= pRequest->FrontLeftContaID.contaID;
		tmpBoxNumber.strBoxModel	= pRequest->FrontLeftContaID.Type;
		// RED, BLUE, WHITE, GRAY, GREEN, OTHER
		if (pRequest->FrontLeftContaID.color	== RED)
			tmpBoxNumber.strBoxColor	=	"RED";
		if (pRequest->FrontLeftContaID.color	== BLUE)
			tmpBoxNumber.strBoxColor	=	"BLUE";
		if (pRequest->FrontLeftContaID.color	== WHITE)
			tmpBoxNumber.strBoxColor	=	"WHITE";
		if (pRequest->FrontLeftContaID.color	== GRAY)
			tmpBoxNumber.strBoxColor	=	"GRAY";
		if (pRequest->FrontLeftContaID.color	== GREEN)
			tmpBoxNumber.strBoxColor	=	"GREEN";
		if (pRequest->FrontLeftContaID.color	== OTHER)
			tmpBoxNumber.strBoxColor	=	"OTHER";
		if (pRequest->FrontLeftContaID.ali.Atype == H_Align)
			tmpBoxNumber.strArrangement = "H";
		if (pRequest->FrontLeftContaID.ali.Atype == T_Align)
			tmpBoxNumber.strArrangement = "T";
		tmpBoxNumber.direct				= FRONTLEFT;
		boxNumberVect.push_back(tmpBoxNumber);
	}

	if (pRequest->FrontRightContaID.contaID[0] != '\0')
	{
		tmpBoxNumber.strBoxNumber	= pRequest->FrontRightContaID.contaID;
		tmpBoxNumber.strBoxModel	= pRequest->FrontRightContaID.Type;
		// RED, BLUE, WHITE, GRAY, GREEN, OTHER
		if (pRequest->FrontRightContaID.color	== RED)
			tmpBoxNumber.strBoxColor	=	"RED";
		if (pRequest->FrontRightContaID.color	== BLUE)
			tmpBoxNumber.strBoxColor	=	"BLUE";
		if (pRequest->FrontRightContaID.color	== WHITE)
			tmpBoxNumber.strBoxColor	=	"WHITE";
		if (pRequest->FrontRightContaID.color	== GRAY)
			tmpBoxNumber.strBoxColor	=	"GRAY";
		if (pRequest->FrontRightContaID.color	== GREEN)
			tmpBoxNumber.strBoxColor	=	"GREEN";
		if (pRequest->FrontRightContaID.color	== OTHER)
			tmpBoxNumber.strBoxColor	=	"OTHER";
		if (pRequest->FrontRightContaID.ali.Atype == H_Align)
			tmpBoxNumber.strArrangement = "H";
		if (pRequest->FrontRightContaID.ali.Atype == T_Align)
			tmpBoxNumber.strArrangement = "T";
		tmpBoxNumber.direct				= FRONTRIGHT;
		boxNumberVect.push_back(tmpBoxNumber);
	}

	if (pRequest->BackContaID.contaID[0] != '\0')
	{
		tmpBoxNumber.strBoxNumber	= pRequest->BackContaID.contaID;
		tmpBoxNumber.strBoxModel	= pRequest->BackContaID.Type;
		// RED, BLUE, WHITE, GRAY, GREEN, OTHER
		if (pRequest->BackContaID.color	== RED)
			tmpBoxNumber.strBoxColor	=	"RED";
		if (pRequest->BackContaID.color	== BLUE)
			tmpBoxNumber.strBoxColor	=	"BLUE";
		if (pRequest->BackContaID.color	== WHITE)
			tmpBoxNumber.strBoxColor	=	"WHITE";
		if (pRequest->BackContaID.color	== GRAY)
			tmpBoxNumber.strBoxColor	=	"GRAY";
		if (pRequest->BackContaID.color	== GREEN)
			tmpBoxNumber.strBoxColor	=	"GREEN";
		if (pRequest->BackContaID.color	== OTHER)
			tmpBoxNumber.strBoxColor	=	"OTHER";
		if (pRequest->BackContaID.ali.Atype == H_Align)
			tmpBoxNumber.strArrangement = "H";
		if (pRequest->BackContaID.ali.Atype == T_Align)
			tmpBoxNumber.strArrangement = "T";
		tmpBoxNumber.direct				= BACK;
		boxNumberVect.push_back(tmpBoxNumber);
	}

	if (pRequest->BackLeftContaID.contaID[0] != '\0')
	{
		tmpBoxNumber.strBoxNumber	= pRequest->BackLeftContaID.contaID;
		tmpBoxNumber.strBoxModel	= pRequest->BackLeftContaID.Type;
		// RED, BLUE, WHITE, GRAY, GREEN, OTHER
		if (pRequest->BackLeftContaID.color	== RED)
			tmpBoxNumber.strBoxColor	=	"RED";
		if (pRequest->BackLeftContaID.color	== BLUE)
			tmpBoxNumber.strBoxColor	=	"BLUE";
		if (pRequest->BackLeftContaID.color	== WHITE)
			tmpBoxNumber.strBoxColor	=	"WHITE";
		if (pRequest->BackLeftContaID.color	== GRAY)
			tmpBoxNumber.strBoxColor	=	"GRAY";
		if (pRequest->BackLeftContaID.color	== GREEN)
			tmpBoxNumber.strBoxColor	=	"GREEN";
		if (pRequest->BackLeftContaID.color	== OTHER)
			tmpBoxNumber.strBoxColor	=	"OTHER";
		if (pRequest->BackLeftContaID.ali.Atype == H_Align)
			tmpBoxNumber.strArrangement = "H";
		if (pRequest->BackLeftContaID.ali.Atype == T_Align)
			tmpBoxNumber.strArrangement = "T";
		tmpBoxNumber.direct				= BACKLEFT;
		boxNumberVect.push_back(tmpBoxNumber);
	}

	if (pRequest->BackRightContaID.contaID[0] != '\0')
	{
		tmpBoxNumber.strBoxNumber	= pRequest->BackRightContaID.contaID;
		tmpBoxNumber.strBoxModel	= pRequest->BackRightContaID.Type;
		// RED, BLUE, WHITE, GRAY, GREEN, OTHER
		if (pRequest->BackRightContaID.color	== RED)
			tmpBoxNumber.strBoxColor	=	"RED";
		if (pRequest->BackRightContaID.color	== BLUE)
			tmpBoxNumber.strBoxColor	=	"BLUE";
		if (pRequest->BackRightContaID.color	== WHITE)
			tmpBoxNumber.strBoxColor	=	"WHITE";
		if (pRequest->BackRightContaID.color	== GRAY)
			tmpBoxNumber.strBoxColor	=	"GRAY";
		if (pRequest->BackRightContaID.color	== GREEN)
			tmpBoxNumber.strBoxColor	=	"GREEN";
		if (pRequest->BackRightContaID.color	== OTHER)
			tmpBoxNumber.strBoxColor	=	"OTHER";
		if (pRequest->BackRightContaID.ali.Atype == H_Align)
			tmpBoxNumber.strArrangement = "H";
		if (pRequest->BackRightContaID.ali.Atype == T_Align)
			tmpBoxNumber.strArrangement = "T";
		tmpBoxNumber.direct				= BACKRIGHT;
		boxNumberVect.push_back(tmpBoxNumber);
	}

	return 0;
}

int ProcessContaData::BoxNumberCorrection(const std::vector<structBoxNumberRecogResult>& boxNumberSet, JZ_ContaIDCorrectionResponse *pResponse)
{
	//for (size_t i = 0; i < boxNumberSet.size(); ++i)
	//{
	//	syslog(LOG_DEBUG, "%s,%s,%s,%s,%d\n", boxNumberSet[i].strBoxNumber.c_str(),boxNumberSet[i].strBoxModel.c_str(),boxNumberSet[i].strBoxColor.c_str(), \
	//		boxNumberSet[i].strArrangement.c_str(),(int)boxNumberSet[i].direct);
	//}

	// 4副图片 前、前右，后、后左识别结果
	// 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
	//structBoxNumberRecogResultCorrected resultCorrected;
	if (boxNumberSet.size() <= 0)
	{
		pResponse->contaIDAfter.contaID[0]	= '\0';
		pResponse->contaIDAfter.Type[0]		= '\0';
		return 0;
	}

	int nRet1 = 0;
	int nRet2 = 0;
	int nRet3 = 0;
	int nRet4 = 0;
	int nRet5 = 0;
	int nRet6 = 0;
	char chVerifyCode1   = ' ';
	char chVerifyCode2   = ' ';
	char chVerifyCode3   = ' ';
	char chVerifyCode4   = ' ';
	char chVerifyCode5   = ' ';
	char chVerifyCode6   = ' ';    
	string strBoxNumber = "";
	BoxNumberCheckAlgo check;
	string strFront         = "";
	string strRight    		= "";
	string strBack          = "";
	string strLeft     		= "";
	string strBoxModel = "";
	
	for (int i = 0; i < boxNumberSet.size(); ++i)
	{
		if (boxNumberSet[i].direct == FRONT)
		{
			strFront        = boxNumberSet[i].strBoxNumber;
			if (boxNumberSet[i].strBoxModel != "")
				strBoxModel = boxNumberSet[i].strBoxModel;
		}
		if (boxNumberSet[i].direct == FRONTRIGHT || boxNumberSet[i].direct == BACKRIGHT)
		{
			strRight   = boxNumberSet[i].strBoxNumber; 
			if (boxNumberSet[i].strBoxModel != "")
				strBoxModel = boxNumberSet[i].strBoxModel;            
		}
		if (boxNumberSet[i].direct == BACK)
		{
			strBack        = boxNumberSet[i].strBoxNumber;
			if (boxNumberSet[i].strBoxModel != "")
				strBoxModel = boxNumberSet[i].strBoxModel;   
		}
		if (boxNumberSet[i].direct == BACKLEFT || boxNumberSet[i].direct == FRONTLEFT)
		{
			strLeft         =   boxNumberSet[i].strBoxNumber;   
			if (boxNumberSet[i].strBoxModel != "")
				strBoxModel = boxNumberSet[i].strBoxModel;            
		}
	}   

	if (boxNumberSet.size() > 0)
	{
		nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
		nRet2 = check.GetBoxNumCheckbit(strLeft, chVerifyCode2);
		nRet3 = check.GetBoxNumCheckbit(strRight, chVerifyCode3);		
		nRet4 = check.GetBoxNumCheckbit(strBack, chVerifyCode4);

		if (nRet1 == 1)
		{
			strncpy(pResponse->contaIDAfter.contaID, strFront.c_str(), strFront.size());
			strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
			pResponse->contaIDAfter.contaID[11]	= '\0';
			pResponse->contaIDAfter.Type[4]		= '\0';
			return 1;
		}
		if (nRet2 == 1)
		{
			strncpy(pResponse->contaIDAfter.contaID, strLeft.c_str(), strLeft.size());
			strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
			pResponse->contaIDAfter.contaID[11]	= '\0';
			pResponse->contaIDAfter.Type[4]		= '\0';
			return 1;
		}
		if (nRet3 == 1)
		{
			strncpy(pResponse->contaIDAfter.contaID, strRight.c_str(), strRight.size());
			strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
			pResponse->contaIDAfter.contaID[11]	= '\0';
			pResponse->contaIDAfter.Type[4]		= '\0';
			return 1;
		}    
		if (nRet4 == 1)
		{
			strncpy(pResponse->contaIDAfter.contaID, strBack.c_str(), strBack.size());
			strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
			pResponse->contaIDAfter.contaID[11]	= '\0';
			pResponse->contaIDAfter.Type[4]		= '\0';
			return 1;
		}   
	}

	if (boxNumberSet.size() > 0)
	{
		// 确定箱主代码
		vector<structContaOwnerData> contaOwnerVect;
		GetContaOwnerData(contaOwnerVect);
		bool bFind = false;
		for (int i = 0; i < contaOwnerVect.size(); ++i)
		{
			if (strFront.substr(0,3) == contaOwnerVect[i].strCompanyCode)
			{
				bFind = true;
				break;
			}
			if (strLeft.substr(0,3) == contaOwnerVect[i].strCompanyCode)
			{
				bFind = true;
				break;
			}
			if (strRight.substr(0,3) == contaOwnerVect[i].strCompanyCode)
			{
				bFind = true;
				break;
			}
			if (strBack.substr(0,3) == contaOwnerVect[i].strCompanyCode)
			{
				bFind = true;
				break;
			}		
		}
		// call interface
		// bFind = xxxxx;
		// 不存在箱主代码么
		if (!bFind)
		{
			// 修改易错字母、并重新校验- front
			string	strFrontTmp = strFront;
			vector<int> posVect;
			for (int i = 0; i < strFrontTmp.size(); ++i)
			{
				string::size_type nFrontPos = strFrontTmp.find('?');
				if (nFrontPos != string::npos)
				{
					posVect.push_back(nFrontPos);
					string strSub 	= strFrontTmp.substr(nFrontPos + 1);
					strFrontTmp 	= strSub;
					i               = 0;
				}
				else
				{
					break;
				}
			}
			strFrontTmp = strFront;
			for (int i = 0; i < posVect.size() && i < strLeft.size(); ++i)
			{
				if (posVect[i] < strFrontTmp.size() && posVect[i] < strLeft.size())
				{
					if (strLeft[posVect[i]] != '?')
						strFrontTmp[posVect[i]] = strLeft[posVect[i]];
				}
			}			
			for (int i = 0; i < posVect.size() && i < strRight.size(); ++i)
			{
				if (posVect[i] < strFrontTmp.size() && posVect[i] < strRight.size())
				{
					if (strRight[posVect[i]] != '?')
						strFrontTmp[posVect[i]] = strRight[posVect[i]];
				}
			}			

			// modify back
			string	strBackTmp = strBack;
			vector<int> posVect4;
			for (int i = 0; i < strBackTmp.size(); ++i)
			{
				string::size_type nFrontPos = strBackTmp.find('?');
				if (nFrontPos != string::npos)
				{
					posVect4.push_back(nFrontPos);
					string strSub 	= strBackTmp.substr(nFrontPos + 1);
					strBackTmp 	= strSub;
					i               = 0;
				}
				else
				{
					break;
				}
			}
			strBackTmp = strLeft;
			for (int i = 0; i < posVect4.size() && i < strLeft.size(); ++i)
			{
				if (posVect4[i] < strBackTmp.size() && posVect4[i] < strLeft.size())
				{
					if (strLeft[posVect4[i]] != '?')
						strBackTmp[posVect4[i]] = strLeft[posVect4[i]];
				}
			} 
			for (int i = 0; i < posVect4.size() && i < strRight.size(); ++i)
			{
				if (posVect4[i] < strBackTmp.size() && posVect4[i] < strRight.size())
				{
					if (strRight[posVect4[i]] != '?')
						strBackTmp[posVect4[i]] = strRight[posVect4[i]];
				}
			} 
			
			// modify Left
			string	strLeftTmp = strLeft;
			vector<int> posVect3;
			for (int i = 0; i < strLeftTmp.size(); ++i)
			{
				string::size_type nFrontPos = strLeftTmp.find('?');
				if (nFrontPos != string::npos)
				{
					posVect3.push_back(nFrontPos);
					string strSub 	= strLeftTmp.substr(nFrontPos + 1);
					strLeftTmp 	= strSub;
					i               = 0;
				}
				else
				{
					break;
				}
			}
			strLeftTmp = strLeft;
			for (int i = 0; i < posVect3.size() && i < strFront.size(); ++i)
			{
				if (posVect3[i] < strLeftTmp.size() && posVect3[i] < strFront.size())
				{
					if (strFront[posVect3[i]] != '?')
						strLeftTmp[posVect3[i]] = strFront[posVect3[i]];
				}
			} 
			for (int i = 0; i < posVect3.size() && i < strRight.size(); ++i)
			{
				if (posVect3[i] < strLeftTmp.size() && posVect3[i] < strRight.size())
				{
					if (strRight[posVect3[i]] != '?')
						strLeftTmp[posVect3[i]] = strRight[posVect3[i]];
				}
			}
			for (int i = 0; i < posVect3.size() && i < strBack.size(); ++i)
			{
				if (posVect3[i] < strLeftTmp.size() && posVect3[i] < strBack.size())
				{
					if (strBack[posVect3[i]] != '?')
						strLeftTmp[posVect3[i]] = strBack[posVect3[i]];
				}
			} 

			// modify right
			string	strRightTmp = strRight;
			vector<int> posVect2;
			for (int i = 0; i < strRightTmp.size(); ++i)
			{
				string::size_type nFrontPos = strRightTmp.find('?');
				if (nFrontPos != string::npos)
				{
					posVect2.push_back(nFrontPos);
					string strSub 	= strRightTmp.substr(nFrontPos + 1);
					strRightTmp 	= strSub;
					i               = 0;
				}
				else
				{
					break;
				}
			}
			strRightTmp = strRight;
			for (int i = 0; i < posVect2.size() && i < strFront.size(); ++i)
			{
				if (posVect2[i] < strRightTmp.size() && posVect2[i] < strFront.size())
				{
					if (strFront[posVect2[i]] != '?')
						strRightTmp[posVect2[i]] = strFront[posVect2[i]];
				}
			}
			for (int i = 0; i < posVect2.size() && i < strLeft.size(); ++i)
			{
				if (posVect2[i] < strRightTmp.size() && posVect2[i] < strLeft.size())
				{
					if (strLeft[posVect2[i]] != '?')
						strRightTmp[posVect2[i]] = strLeft[posVect2[i]];
				}
			}
			for (int i = 0; i < posVect2.size() && i < strBack.size(); ++i)
			{
				if (posVect2[i] < strRightTmp.size() && posVect2[i] < strBack.size())
				{
					if (strBack[posVect2[i]] != '?')
						strRightTmp[posVect2[i]] = strBack[posVect2[i]];
				}
			}			
            
			// 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
			nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);
			nRet2 = check.GetBoxNumCheckbit(strRightTmp, chVerifyCode2);
			nRet3 = check.GetBoxNumCheckbit(strLeftTmp, chVerifyCode3);
			nRet4 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode4);
			if (nRet1 == 1)
			{
				strncpy(pResponse->contaIDAfter.contaID, strFrontTmp.c_str(), strFrontTmp.size());
				strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
				pResponse->contaIDAfter.contaID[11]	= '\0';
				pResponse->contaIDAfter.Type[4]		= '\0';
				return 1;
			}
			if (nRet2 == 1)
			{
				strncpy(pResponse->contaIDAfter.contaID, strRightTmp.c_str(), strRightTmp.size());
				strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
				pResponse->contaIDAfter.contaID[11]	= '\0';
				pResponse->contaIDAfter.Type[4]		= '\0';
				return 1;
			}
			if (nRet3 == 1)
			{
				strncpy(pResponse->contaIDAfter.contaID, strLeftTmp.c_str(), strLeftTmp.size());
				strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
				pResponse->contaIDAfter.contaID[11]	= '\0';
				pResponse->contaIDAfter.Type[4]		= '\0';
				return 1;
			}    
			if (nRet4 == 1)
			{
				strncpy(pResponse->contaIDAfter.contaID, strBackTmp.c_str(), strBackTmp.size());
				strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
				pResponse->contaIDAfter.contaID[11]	= '\0';
				pResponse->contaIDAfter.Type[4]		= '\0';
				return 1;
			}                 


			// 有校验正确的么
			if (nRet1 == 1 || nRet2 == 1 || nRet3 == 1 || nRet4 == 1)
			{
				// never go here
				// 获取正确的识别结果、箱型、长短箱标志输出
			}            
			else
			{
				nRet1 = 0;
				nRet2 = 0;
				nRet3 = 0;
				nRet4 = 0;
				
				// 从箱号中挑选出一个正确的校验位,再次进行校验
				// 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
				// 1. supposing strFront right
				strLeftTmp			= strLeft;
				strRightTmp			= strRight;
				strFrontTmp 		= strFront;
				if (strRightTmp != "" && strFrontTmp != "" && strRightTmp.size() == strFrontTmp.size())
					strRightTmp[strRightTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
				nRet1 = check.GetBoxNumCheckbit(strRightTmp, chVerifyCode1);
				if (strLeftTmp != "" && strFrontTmp != "" && strLeftTmp.size() == strFrontTmp.size())
					strLeftTmp[strLeftTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
				nRet2 = check.GetBoxNumCheckbit(strLeftTmp, chVerifyCode2);
				
				if (nRet1 == 1)
				{
					strncpy(pResponse->contaIDAfter.contaID, strRightTmp.c_str(), strRightTmp.size());
					strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
					pResponse->contaIDAfter.contaID[11]	= '\0';
					pResponse->contaIDAfter.Type[4]		= '\0';
					return 1;
				}
				
				if (nRet2 == 1)
				{
					strncpy(pResponse->contaIDAfter.contaID, strLeftTmp.c_str(), strLeftTmp.size());
					strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
					pResponse->contaIDAfter.contaID[11]	= '\0';
					pResponse->contaIDAfter.Type[4]		= '\0';
					return 1;
				}
				
				
				// 3. supposing strBackTmp right
				strLeftTmp			= strLeft;
				strRightTmp			= strRight;
				strBackTmp 			= strBack;
				if (strLeftTmp != "" && strBackTmp != "" && strLeftTmp.size() == strBackTmp.size())
					strLeftTmp[strLeftTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
				nRet1 = check.GetBoxNumCheckbit(strLeftTmp, chVerifyCode1);				
				if (strRightTmp != "" && strBackTmp != "" && strRightTmp.size() == strBackTmp.size())
					strRightTmp[strRightTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
				nRet2 = check.GetBoxNumCheckbit(strRightTmp, chVerifyCode2);	

				if (nRet1 == 1)
				{
					strncpy(pResponse->contaIDAfter.contaID, strLeftTmp.c_str(), strLeftTmp.size());
					strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
					pResponse->contaIDAfter.contaID[11]	= '\0';
					pResponse->contaIDAfter.Type[4]		= '\0';
					return 1;
				}
				
				if (nRet2 == 1)
				{
					strncpy(pResponse->contaIDAfter.contaID, strRightTmp.c_str(), strRightTmp.size());
					strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
					pResponse->contaIDAfter.contaID[11]	= '\0';
					pResponse->contaIDAfter.Type[4]		= '\0';
					return 1;
				}
				
				
				
				
				// 2. supposing strRight right
				strFrontTmp 		= strFront;
				strLeftTmp			= strLeft;
				strBackTmp 			= strBack;
				strRightTmp	        = strRight;
				if (strRightTmp != "" && strFrontTmp != "" && strRightTmp.size() == strFrontTmp.size())
					strFrontTmp[strRightTmp.size() - 1] = strRightTmp[strFrontTmp.size() - 1];
				nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);	
				if (strRightTmp != "" && strLeftTmp != "" && strRightTmp.size() == strLeftTmp.size())
					strLeftTmp[strRightTmp.size() - 1] = strRightTmp[strLeftTmp.size() - 1];
				nRet2 = check.GetBoxNumCheckbit(strLeftTmp, chVerifyCode2);					
				if (strRightTmp != "" && strBackTmp != "" && strRightTmp.size() == strBackTmp.size())
					strBackTmp[strRightTmp.size() - 1] = strRightTmp[strBackTmp.size() - 1];
				nRet3 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode3);	

				if (nRet1 == 1)
				{
					strncpy(pResponse->contaIDAfter.contaID, strFrontTmp.c_str(), strFrontTmp.size());
					strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
					pResponse->contaIDAfter.contaID[11]	= '\0';
					pResponse->contaIDAfter.Type[4]		= '\0';
					return 1;
				}
				
				if (nRet2 == 1)
				{
					strncpy(pResponse->contaIDAfter.contaID, strLeftTmp.c_str(), strLeftTmp.size());
					strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
					pResponse->contaIDAfter.contaID[11]	= '\0';
					pResponse->contaIDAfter.Type[4]		= '\0';
					return 1;
				}

				if (nRet3 == 1)
				{
					strncpy(pResponse->contaIDAfter.contaID, strBackTmp.c_str(), strBackTmp.size());
					strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
					pResponse->contaIDAfter.contaID[11]	= '\0';
					pResponse->contaIDAfter.Type[4]		= '\0';
					return 1;
				}
				
				
				
				// 4. supposing strLeftTmp right
				strFrontTmp 		= strFront;
				strBackTmp 			= strBack;
				strRightTmp	        = strRight;
				strLeftTmp			= strLeft;
				
				if (strLeftTmp != "" && strFrontTmp != "" && strLeftTmp.size() == strFrontTmp.size())
					strFrontTmp[strLeftTmp.size() - 1] = strLeftTmp[strFrontTmp.size() - 1];								
				nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1); 				
				
				if (strLeftTmp != "" && strBackTmp != "" && strLeftTmp.size() == strBackTmp.size())
					strBackTmp[strLeftTmp.size() - 1] = strLeftTmp[strBackTmp.size() - 1];								
				nRet2 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode2); 
				
				//sldjflsjdfkldsxxxxxxx xuyao xiugai 
				if (strLeftTmp != "" && strRightTmp != "" && strLeftTmp.size() == strRightTmp.size())
					strRightTmp[strLeftTmp.size() - 1] = strLeftTmp[strRightTmp.size() - 1];								
				nRet3 = check.GetBoxNumCheckbit(strRightTmp, chVerifyCode3); 				
				
				if (nRet1 == 1)
				{
					strncpy(pResponse->contaIDAfter.contaID, strFrontTmp.c_str(), strFrontTmp.size());
					strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
					pResponse->contaIDAfter.contaID[11]	= '\0';
					pResponse->contaIDAfter.Type[4]		= '\0';
					return 1;
				}
				
				if (nRet2 == 1)
				{
					strncpy(pResponse->contaIDAfter.contaID, strBackTmp.c_str(), strBackTmp.size());
					strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
					pResponse->contaIDAfter.contaID[11]	= '\0';
					pResponse->contaIDAfter.Type[4]		= '\0';
					return 1;
				}

				if (nRet3 == 1)
				{
					strncpy(pResponse->contaIDAfter.contaID, strRightTmp.c_str(), strRightTmp.size());
					strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
					pResponse->contaIDAfter.contaID[11]	= '\0';
					pResponse->contaIDAfter.Type[4]		= '\0';
					return 1;
				}

				// 有校验正确的么
				if (nRet1 == 1 || nRet2 == 1 || nRet3 == 1)
				{

				}                 
				else
				{
					// 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
					// 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么

					string str1	= strFront;
					string str2	= strRight;
					string str3	= strBack;
					string str4	= strLeft;
					string str1tmp	= "";
					int nMixSize = 0;
					if (str1.size() > 0)
						nMixSize	= str1.size();
					if (str2.size() > 0)
						nMixSize	= str2.size();
					if (str3.size() > 0)
						nMixSize	= str3.size();
					if (str4.size() > 0)
						nMixSize	= str4.size();
					//for (size_t i = 0; i < str1.size() && i < str2.size() && i < str3.size() && i < str4.size(); ++i)
					for (size_t i = 0; i < nMixSize; ++i)
					{
						map<char,int> statisticsMap;
						map<char,int>::iterator mapIter;
						if (str1.size() > 0 && i < str1.size())
						{
							char ch1	= str1[i];
							mapIter = statisticsMap.find(ch1);
							if (mapIter != statisticsMap.end() && mapIter->first != '?')
							{
								++(mapIter->second);
							}
							else if (mapIter != statisticsMap.end() && mapIter->first == '?')
							{
								mapIter->second = 0;
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch1, nCount));
							}
						}
						if (str2.size() > 0 && i < str2.size())
						{
							char ch2	= str2[i];
							mapIter = statisticsMap.find(ch2);
							if (mapIter != statisticsMap.end() && mapIter->first != '?')
							{
								++(mapIter->second);
							}
							else if (mapIter != statisticsMap.end() && mapIter->first == '?')
							{
								mapIter->second = 0;
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch2, nCount));
							}
						}
						if (str3.size() > 0 && i < str3.size())
						{
							char ch3	= str3[i];
							mapIter = statisticsMap.find(ch3);
							if (mapIter != statisticsMap.end() && mapIter->first != '?')
							{
								++(mapIter->second);
							}
							else if (mapIter != statisticsMap.end() && mapIter->first == '?')
							{
								mapIter->second = 0;
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch3, nCount));
							}
						}
						if (str4.size() > 0 && i < str4.size())
						{
							char ch4	= str4[i];
							mapIter = statisticsMap.find(ch4);
							if (mapIter != statisticsMap.end() && mapIter->first != '?')
							{
								++(mapIter->second);
							}
							else if (mapIter != statisticsMap.end() && mapIter->first == '?')
							{
								mapIter->second = 0;
							}
							else
							{
								int nCount = 1;
								statisticsMap.insert(make_pair(ch4, nCount));
							}
						}	
						int nMax = -1;		
						char tmpCh = ' ';
						for (map<char,int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
						{			
							//if ((*it).second > nMax)
							//{
							//	nMax	= (*it).second;
							//	tmpCh	= (*it).first;
							//}
							if ((*it).second > nMax)
							{
								nMax	= (*it).second;
								if (it->first != '?')
									tmpCh	= (*it).first;
								else
									tmpCh	= '?';
							}
						}
						str1tmp += tmpCh;		
					}                   
					nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
					// 有校验正确的么
					if (nRet1 == 1)
					{
						// 获取正确的识别结果、箱型、长短箱标志输出
						strncpy(pResponse->contaIDAfter.contaID, str1tmp.c_str(), str1tmp.size());
						strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
						pResponse->contaIDAfter.contaID[11]	= '\0';
						pResponse->contaIDAfter.Type[4]		= '\0';
						return 1;
					}                     
					else
					{
						// 通过算法算出校验位
						nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
						// 产生不确定的箱号、箱型、长短箱标志
						strncpy(pResponse->contaIDAfter.contaID, str1tmp.c_str(), str1tmp.size());
						strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
						pResponse->contaIDAfter.contaID[11]	= '\0';
						pResponse->contaIDAfter.Type[4]		= '\0';
						return 0;
					}                        
				}
			}
		}
		else
		{/*
		 // 从箱号中挑选出一个正确的校验位,再次进行校验
		 if (前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么)
		 {
		 // 获取正确的识别结果、箱型、长短箱标志输出                
		 }
		 else
		 {
		 // 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
		 if (前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么)
		 {
		 // 获取正确的识别结果、箱型、长短箱标志输出                
		 }
		 else
		 {
		 // 通过算法算出校验位
		 // 产生不确定的箱号、箱型、长短箱标志
		 }
		 }*/
				// 从箱号中挑选出一个正确的校验位,再次进行校验
				// 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
				// 从箱号中挑选出一个正确的校验位,再次进行校验
				// 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么
			nRet1 = 0;
			nRet2 = 0;
			nRet3 = 0;
			
			// 1. supposing strFront right
			std::string strLeftTmp			= strLeft;
			std::string strRightTmp			= strRight;
			std::string strFrontTmp 		= strFront;
			std::string strBackTmp 			= strBack;
			if (strRightTmp != "" && strFrontTmp != "" && strRightTmp.size() == strFrontTmp.size())
				strRightTmp[strRightTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
			nRet1 = check.GetBoxNumCheckbit(strRightTmp, chVerifyCode1);
			if (strLeftTmp != "" && strFrontTmp != "" && strLeftTmp.size() == strFrontTmp.size())
				strLeftTmp[strLeftTmp.size() - 1] = strFrontTmp[strFrontTmp.size() - 1];
			nRet2 = check.GetBoxNumCheckbit(strLeftTmp, chVerifyCode2);
			
			if (nRet1 == 1)
			{
				strncpy(pResponse->contaIDAfter.contaID, strRightTmp.c_str(), strRightTmp.size());
				strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
				pResponse->contaIDAfter.contaID[11]	= '\0';
				pResponse->contaIDAfter.Type[4]		= '\0';
				return 1;
			}
			
			if (nRet2 == 1)
			{
				strncpy(pResponse->contaIDAfter.contaID, strLeftTmp.c_str(), strLeftTmp.size());
				strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
				pResponse->contaIDAfter.contaID[11]	= '\0';
				pResponse->contaIDAfter.Type[4]		= '\0';
				return 1;
			}
			
			
			// 3. supposing strBackTmp right
			strLeftTmp			= strLeft;
			strRightTmp			= strRight;
			strBackTmp 			= strBack;
			if (strLeftTmp != "" && strBackTmp != "" && strLeftTmp.size() == strBackTmp.size())
				strLeftTmp[strLeftTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
			nRet1 = check.GetBoxNumCheckbit(strLeftTmp, chVerifyCode1);				
			if (strRightTmp != "" && strBackTmp != "" && strRightTmp.size() == strBackTmp.size())
				strRightTmp[strRightTmp.size() - 1] = strBackTmp[strBackTmp.size() - 1];
			nRet2 = check.GetBoxNumCheckbit(strRightTmp, chVerifyCode2);	

			if (nRet1 == 1)
			{
				strncpy(pResponse->contaIDAfter.contaID, strLeftTmp.c_str(), strLeftTmp.size());
				strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
				pResponse->contaIDAfter.contaID[11]	= '\0';
				pResponse->contaIDAfter.Type[4]		= '\0';
				return 1;
			}
			
			if (nRet2 == 1)
			{
				strncpy(pResponse->contaIDAfter.contaID, strRightTmp.c_str(), strRightTmp.size());
				strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
				pResponse->contaIDAfter.contaID[11]	= '\0';
				pResponse->contaIDAfter.Type[4]		= '\0';
				return 1;
			}

			
			// 2. supposing strRight right
			strFrontTmp 		= strFront;
			strLeftTmp			= strLeft;
			strBackTmp 			= strBack;
			strRightTmp	        = strRight;
			if (strRightTmp != "" && strFrontTmp != "" && strRightTmp.size() == strFrontTmp.size())
				strFrontTmp[strRightTmp.size() - 1] = strRightTmp[strFrontTmp.size() - 1];
			nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1);	
			if (strRightTmp != "" && strLeftTmp != "" && strRightTmp.size() == strLeftTmp.size())
				strLeftTmp[strRightTmp.size() - 1] = strRightTmp[strLeftTmp.size() - 1];
			nRet2 = check.GetBoxNumCheckbit(strLeftTmp, chVerifyCode2);					
			if (strRightTmp != "" && strBackTmp != "" && strRightTmp.size() == strBackTmp.size())
				strBackTmp[strRightTmp.size() - 1] = strRightTmp[strBackTmp.size() - 1];
			nRet3 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode3);	

			if (nRet1 == 1)
			{
				strncpy(pResponse->contaIDAfter.contaID, strFrontTmp.c_str(), strFrontTmp.size());
				strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
				pResponse->contaIDAfter.contaID[11]	= '\0';
				pResponse->contaIDAfter.Type[4]		= '\0';
				return 1;
			}
			
			if (nRet2 == 1)
			{
				strncpy(pResponse->contaIDAfter.contaID, strLeftTmp.c_str(), strLeftTmp.size());
				strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
				pResponse->contaIDAfter.contaID[11]	= '\0';
				pResponse->contaIDAfter.Type[4]		= '\0';
				return 1;
			}

			if (nRet3 == 1)
			{
				strncpy(pResponse->contaIDAfter.contaID, strBackTmp.c_str(), strBackTmp.size());
				strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
				pResponse->contaIDAfter.contaID[11]	= '\0';
				pResponse->contaIDAfter.Type[4]		= '\0';
				return 1;
			}
			
			// 4. supposing strLeftTmp right
			strFrontTmp 		= strFront;
			strBackTmp 			= strBack;
			strRightTmp	        = strRight;
			strLeftTmp			= strLeft;
			
			if (strLeftTmp != "" && strFrontTmp != "" && strLeftTmp.size() == strFrontTmp.size())
				strFrontTmp[strLeftTmp.size() - 1] = strLeftTmp[strFrontTmp.size() - 1];								
			nRet1 = check.GetBoxNumCheckbit(strFrontTmp, chVerifyCode1); 				
			
			if (strLeftTmp != "" && strBackTmp != "" && strLeftTmp.size() == strBackTmp.size())
				strBackTmp[strLeftTmp.size() - 1] = strLeftTmp[strBackTmp.size() - 1];								
			nRet2 = check.GetBoxNumCheckbit(strBackTmp, chVerifyCode2); 
			
			//sldjflsjdfkldsxxxxxxx xuyao xiugai 
			if (strLeftTmp != "" && strRightTmp != "" && strLeftTmp.size() == strRightTmp.size())
				strRightTmp[strLeftTmp.size() - 1] = strLeftTmp[strRightTmp.size() - 1];								
			nRet3 = check.GetBoxNumCheckbit(strRightTmp, chVerifyCode3); 				
			
			if (nRet1 == 1)
			{
				strncpy(pResponse->contaIDAfter.contaID, strFrontTmp.c_str(), strFrontTmp.size());
				strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
				pResponse->contaIDAfter.contaID[11]	= '\0';
				pResponse->contaIDAfter.Type[4]		= '\0';
				return 1;
			}
			
			if (nRet2 == 1)
			{
				strncpy(pResponse->contaIDAfter.contaID, strBackTmp.c_str(), strBackTmp.size());
				strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
				pResponse->contaIDAfter.contaID[11]	= '\0';
				pResponse->contaIDAfter.Type[4]		= '\0';
				return 1;
			}

			if (nRet3 == 1)
			{
				strncpy(pResponse->contaIDAfter.contaID, strRightTmp.c_str(), strRightTmp.size());
				strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
				pResponse->contaIDAfter.contaID[11]	= '\0';
				pResponse->contaIDAfter.Type[4]		= '\0';
				return 1;
			}
			
			// 有校验正确的么
			if (nRet1 == 1 || nRet2 == 1 || nRet3 == 1)
			{
				
			}                 
			else
			{
				// 从箱号中按位挑选出比例最高的箱号数字，再次进行校验
				// 前、前右箱号、后、后左箱号 分别进行校验, 有校验正确的么

				string str1	= strFront;
				string str2	= strRight;
				string str3	= strBack;
				string str4	= strLeft;
				string str1tmp	= "";
				int nMixSize = 0;
				if (str1.size() > 0)
					nMixSize	= str1.size();
				if (str2.size() > 0)
					nMixSize	= str2.size();
				if (str3.size() > 0)
					nMixSize	= str3.size();
				if (str4.size() > 0)
					nMixSize	= str4.size();
				//for (size_t i = 0; i < str1.size() && i < str2.size() && i < str3.size() && i < str4.size(); ++i)
				for (size_t i = 0; i < nMixSize; ++i)
				{
					map<char,int> statisticsMap;
					map<char,int>::iterator mapIter;
					if (str1.size() > 0 && i < str1.size())
					{
						char ch1	= str1[i];
						mapIter = statisticsMap.find(ch1);
						if (mapIter != statisticsMap.end() && mapIter->first != '?')
						{
							++(mapIter->second);
						}
						else if (mapIter != statisticsMap.end() && mapIter->first == '?')
						{
							mapIter->second = 0;
						}
						else
						{
							int nCount = 1;
							statisticsMap.insert(make_pair(ch1, nCount));
						}
					}
					if (str2.size() > 0 && i < str2.size())
					{
						char ch2	= str2[i];
						mapIter = statisticsMap.find(ch2);
						if (mapIter != statisticsMap.end() && mapIter->first != '?')
						{
							++(mapIter->second);
						}
						else if (mapIter != statisticsMap.end() && mapIter->first == '?')
						{
							mapIter->second = 0;
						}
						else
						{
							int nCount = 1;
							statisticsMap.insert(make_pair(ch2, nCount));
						}
					}
					if (str3.size() > 0 && i < str3.size())
					{
						char ch3	= str3[i];
						mapIter = statisticsMap.find(ch3);
						if (mapIter != statisticsMap.end() && mapIter->first != '?')
						{
							++(mapIter->second);
						}
						else if (mapIter != statisticsMap.end() && mapIter->first == '?')
						{
							mapIter->second = 0;
						}
						else
						{
							int nCount = 1;
							statisticsMap.insert(make_pair(ch3, nCount));
						}
					}
					if (str4.size() > 0 && i < str4.size())
					{
						char ch4	= str4[i];
						mapIter = statisticsMap.find(ch4);
						if (mapIter != statisticsMap.end() && mapIter->first != '?')
						{
							++(mapIter->second);
						}
						else if (mapIter != statisticsMap.end() && mapIter->first == '?')
						{
							mapIter->second = 0;
						}
						else
						{
							int nCount = 1;
							statisticsMap.insert(make_pair(ch4, nCount));
						}
					}	
					int nMax = -1;		
					char tmpCh = ' ';
					for (map<char,int>::iterator it = statisticsMap.begin(); it != statisticsMap.end(); ++it)
					{			
						if ((*it).second > nMax)
						{
							nMax	= (*it).second;
							if (it->first != '?')
								tmpCh	= (*it).first;
							else
								tmpCh	= '?';
						}
					}
					str1tmp += tmpCh;		
				}                    
				nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
				// 有校验正确的么
				if (nRet1 == 1)
				{
					// 获取正确的识别结果、箱型、长短箱标志输出
					strncpy(pResponse->contaIDAfter.contaID, str1tmp.c_str(), str1tmp.size());
					strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
					pResponse->contaIDAfter.contaID[11]	= '\0';
					pResponse->contaIDAfter.Type[4]		= '\0';
					return 1;
				}                     
				else
				{
					// 通过算法算出校验位
					nRet1 = check.GetBoxNumCheckbit(strFront, chVerifyCode1);
					if (str1tmp.size() == 11 && str1tmp[10] == '?')
						str1tmp[10]	= chVerifyCode1;
					int nTmpIndex = 0;
					for (int i = 4; i < str1tmp.size(); ++i)
					{
						if (str1tmp[i] == '?')
						{
							nTmpIndex = i;
							break;
						}
					}
					if (nTmpIndex != 0)
					{
						for (int i = 0; i < 10; ++i)
						{
							str1tmp[nTmpIndex]	= '0' + i;
							nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
							if (nRet1 == 1)
							{
								break;
							}
						}
					}
					nRet1 = check.GetBoxNumCheckbit(str1tmp, chVerifyCode1);
					if (str1tmp.size() == 11 && str1tmp[10] != chVerifyCode1)
					{
						str1tmp[10] = chVerifyCode1;
					}
					// 产生不确定的箱号、箱型、长短箱标志
					strncpy(pResponse->contaIDAfter.contaID, str1tmp.c_str(), str1tmp.size());
					strncpy(pResponse->contaIDAfter.Type, strBoxModel.c_str(), strBoxModel.size());
					pResponse->contaIDAfter.contaID[11]	= '\0';
					pResponse->contaIDAfter.Type[4]		= '\0';
					return 0;
				}                        
			}
		}          
	}

	return 0;
}

int ProcessContaData::GetBoxNumberVect2(std::vector<structBoxNumberRecogResult> &boxNumberVect, JZ_ContaIDCorrectionRequest *pRequest)
{
	structBoxNumberRecogResult tmpBoxNumber;

	if (pRequest->FrontContaID.contaID[0] != '\0')
	{
		tmpBoxNumber.strBoxNumber	= pRequest->FrontContaID.contaID;
		tmpBoxNumber.strBoxModel	= pRequest->FrontContaID.Type;
		// RED, BLUE, WHITE, GRAY, GREEN, OTHER
		if (pRequest->FrontContaID.color	== RED)
			tmpBoxNumber.strBoxColor	=	"RED";
		if (pRequest->FrontContaID.color	== BLUE)
			tmpBoxNumber.strBoxColor	=	"BLUE";
		if (pRequest->FrontContaID.color	== WHITE)
			tmpBoxNumber.strBoxColor	=	"WHITE";
		if (pRequest->FrontContaID.color	== GRAY)
			tmpBoxNumber.strBoxColor	=	"GRAY";
		if (pRequest->FrontContaID.color	== GREEN)
			tmpBoxNumber.strBoxColor	=	"GREEN";
		if (pRequest->FrontContaID.color	== OTHER)
			tmpBoxNumber.strBoxColor	=	"OTHER";
		if (pRequest->FrontContaID.ali.Atype == H_Align)
			tmpBoxNumber.strArrangement = "H";
		if (pRequest->FrontContaID.ali.Atype == T_Align)
			tmpBoxNumber.strArrangement = "T";
		tmpBoxNumber.direct				= FRONT;
		boxNumberVect.push_back(tmpBoxNumber);
	}

	if (pRequest->LeftContaID.contaID[0] != '\0')
	{
		tmpBoxNumber.strBoxNumber	= pRequest->LeftContaID.contaID;
		tmpBoxNumber.strBoxModel	= pRequest->LeftContaID.Type;
		// RED, BLUE, WHITE, GRAY, GREEN, OTHER
		if (pRequest->LeftContaID.color	== RED)
			tmpBoxNumber.strBoxColor	=	"RED";
		if (pRequest->LeftContaID.color	== BLUE)
			tmpBoxNumber.strBoxColor	=	"BLUE";
		if (pRequest->LeftContaID.color	== WHITE)
			tmpBoxNumber.strBoxColor	=	"WHITE";
		if (pRequest->LeftContaID.color	== GRAY)
			tmpBoxNumber.strBoxColor	=	"GRAY";
		if (pRequest->LeftContaID.color	== GREEN)
			tmpBoxNumber.strBoxColor	=	"GREEN";
		if (pRequest->LeftContaID.color	== OTHER)
			tmpBoxNumber.strBoxColor	=	"OTHER";
		if (pRequest->LeftContaID.ali.Atype == H_Align)
			tmpBoxNumber.strArrangement = "H";
		if (pRequest->LeftContaID.ali.Atype == T_Align)
			tmpBoxNumber.strArrangement = "T";
		if (pRequest->FrontContaID.contaID[0] != '\0')
			tmpBoxNumber.direct				= FRONTLEFT;
		else if (pRequest->BackContaID.contaID[0] != '\0')
			tmpBoxNumber.direct				= BACKLEFT;
		boxNumberVect.push_back(tmpBoxNumber);
	}

	if (pRequest->RightContaID.contaID[0] != '\0')
	{
		tmpBoxNumber.strBoxNumber	= pRequest->RightContaID.contaID;
		tmpBoxNumber.strBoxModel	= pRequest->RightContaID.Type;
		// RED, BLUE, WHITE, GRAY, GREEN, OTHER
		if (pRequest->RightContaID.color	== RED)
			tmpBoxNumber.strBoxColor	=	"RED";
		if (pRequest->RightContaID.color	== BLUE)
			tmpBoxNumber.strBoxColor	=	"BLUE";
		if (pRequest->RightContaID.color	== WHITE)
			tmpBoxNumber.strBoxColor	=	"WHITE";
		if (pRequest->RightContaID.color	== GRAY)
			tmpBoxNumber.strBoxColor	=	"GRAY";
		if (pRequest->RightContaID.color	== GREEN)
			tmpBoxNumber.strBoxColor	=	"GREEN";
		if (pRequest->RightContaID.color	== OTHER)
			tmpBoxNumber.strBoxColor	=	"OTHER";
		if (pRequest->RightContaID.ali.Atype == H_Align)
			tmpBoxNumber.strArrangement = "H";
		if (pRequest->RightContaID.ali.Atype == T_Align)
			tmpBoxNumber.strArrangement = "T";

		if (pRequest->FrontContaID.contaID[0] != '\0')
			tmpBoxNumber.direct				= FRONTRIGHT;
		else if (pRequest->BackContaID.contaID[0] != '\0')
			tmpBoxNumber.direct				= BACKRIGHT;
		boxNumberVect.push_back(tmpBoxNumber);
	}

	if (pRequest->BackContaID.contaID[0] != '\0')
	{
		tmpBoxNumber.strBoxNumber	= pRequest->BackContaID.contaID;
		tmpBoxNumber.strBoxModel	= pRequest->BackContaID.Type;
		// RED, BLUE, WHITE, GRAY, GREEN, OTHER
		if (pRequest->BackContaID.color	== RED)
			tmpBoxNumber.strBoxColor	=	"RED";
		if (pRequest->BackContaID.color	== BLUE)
			tmpBoxNumber.strBoxColor	=	"BLUE";
		if (pRequest->BackContaID.color	== WHITE)
			tmpBoxNumber.strBoxColor	=	"WHITE";
		if (pRequest->BackContaID.color	== GRAY)
			tmpBoxNumber.strBoxColor	=	"GRAY";
		if (pRequest->BackContaID.color	== GREEN)
			tmpBoxNumber.strBoxColor	=	"GREEN";
		if (pRequest->BackContaID.color	== OTHER)
			tmpBoxNumber.strBoxColor	=	"OTHER";
		if (pRequest->BackContaID.ali.Atype == H_Align)
			tmpBoxNumber.strArrangement = "H";
		if (pRequest->BackContaID.ali.Atype == T_Align)
			tmpBoxNumber.strArrangement = "T";
		tmpBoxNumber.direct				= BACK;
		boxNumberVect.push_back(tmpBoxNumber);
	}

	return 0;
}

void ProcessContaData::GetContaOwnerData(std::vector<structContaOwnerData> &contaOwnerVect)
{
	int i = 0;
	char szLine[256] = {0};
	FILE *pFile = fopen("/etc/ContaOwnerData.txt", "r");
	if (pFile == NULL)
	{
		return ;
	}
	else
	{
		do 
		{
			memset(szLine, 0x00, sizeof(szLine));
			if (fgets(szLine, 255, pFile) == NULL)
			{
				break;
			}
			else
			{
				if (szLine[0] == '\n')
				{
					memset(szLine, 0x00, sizeof(szLine));
					continue;
				}
				char *p = NULL;
				char *pStart = szLine;
				if (pStart != NULL)
					p = strchr(pStart, ',');
				else
					break;
				if (p != NULL)
				{
					structContaOwnerData tmpData;
					*p = '\0';
					if (i == 0)
					{
						tmpData.nLineNo = atoi(p - 1);
					}
					else
					{
						tmpData.nLineNo = atoi(pStart);
					}
					pStart = p + 1;
					p = strchr(pStart, ',');
					if (p != NULL)
					{
						char *pTemp = NULL;
						pTemp = strchr(pStart, ',');
						if (pTemp != NULL)
						{
							*pTemp = '\0';
							tmpData.strCompanyCode = pStart;
						}
						*p = '\0';
						pStart = p + 1;
						p = strchr(pStart, '\n');
						if (p != NULL)
						{
							*p = '\0';
							tmpData.strCompanyName = pStart;
						}
					}
					contaOwnerVect.push_back(tmpData);
					//printf("%d, %s, %s\n", tmpData.nLineNo, tmpData.strCompanyCode.c_str(), tmpData.strCompanyName.c_str());
				}
				i++;
			}
		} while(1);
	}
	fclose(pFile);
}

