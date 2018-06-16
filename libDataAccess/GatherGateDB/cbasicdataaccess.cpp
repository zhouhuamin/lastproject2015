/* 
 * File:   cbasicdataaccess.cpp
 * Author: root
 * 
 * Created on 2014å¹?1??2?? ä¸??9:59
 */

#include "cbasicdataaccess.h"

CBasicDataAccess::CBasicDataAccess()
{
}

CBasicDataAccess::~CBasicDataAccess()
{
}

int CBasicDataAccess::RecordPassVehicleInfo(CppMySQL3DB* pDataabse, char* szXML)
{
    TiXmlDocument xml;
    xml.Parse(szXML);

    TiXmlElement *root = xml.RootElement();
    if(root == NULL)
    {
        return -1;
    }

    char* pAreaID = (char*) root->Attribute("AREA_ID");
    char* pChannelNO = (char*) root->Attribute("CHNL_NO");
    char* pSquenceNo = (char*) root->Attribute("SEQ_NO");
    char* pIEType = (char*) root->Attribute("I_E_TYPE");

    if(!pAreaID || !pChannelNO || !pSquenceNo || !pIEType)
    {
        return -1;
    }

    pDataabse->SetCharsetName("GB2312");

    if(szXML)
    {
        handleAll(pAreaID, pChannelNO, pIEType, pSquenceNo, pDataabse, szXML);
    }


    TiXmlElement *item_ic_tag = root->FirstChildElement("IC");

    if(item_ic_tag)
    {
        handleICTag(pAreaID, pChannelNO, pIEType, pSquenceNo, pDataabse, item_ic_tag);
    }



    TiXmlElement *item_weight_tag = root->FirstChildElement("WEIGHT");

    if(item_weight_tag)
    {
        handleWeightTag(pAreaID, pChannelNO, pIEType, pSquenceNo, pDataabse, item_weight_tag);
    }


    TiXmlElement *item_car_tag = root->FirstChildElement("CAR");

    if(item_car_tag)
    {
        handleCarTag(pAreaID, pChannelNO, pIEType, pSquenceNo, pDataabse, item_car_tag);
    }


    TiXmlElement *item_trailer_tag = root->FirstChildElement("TRAILER");

    if(item_trailer_tag)
    {
        handleTrailerTag(pAreaID, pChannelNO, pIEType, pSquenceNo, pDataabse, item_trailer_tag);
    }

    TiXmlElement *item_conta_tag = root->FirstChildElement("CONTA");

    if(item_conta_tag)
    {

        handleContaTag(pAreaID, pChannelNO, pIEType, pSquenceNo, pDataabse, item_conta_tag);
    }


    TiXmlElement *item_seal_tag = root->FirstChildElement("SEAL");
    if(item_seal_tag)
    {
        handleSealTag(pAreaID, pChannelNO, pIEType, pSquenceNo, pDataabse, item_seal_tag);
    }

    return 0;
}

int CBasicDataAccess::RecordPassResult(CppMySQL3DB* pDataabse, char* szXML)
{
    TiXmlDocument xml;
    xml.Parse(szXML);

    TiXmlElement *root = xml.RootElement();
    if(root == NULL)
    {
        return -1;
    }

    char* pAreaID = (char*) root->Attribute("AREA_ID");
    char* pChannelNO = (char*) root->Attribute("CHNL_NO");
    char* pSquenceNo = (char*) root->Attribute("SEQ_NO");
    char* pIEType = (char*) root->Attribute("I_E_TYPE");

    if(!pAreaID || !pChannelNO || !pSquenceNo || !pIEType)
    {
        return -1;
    }

    if(szXML)
    {
        handleResultAll(pAreaID, pChannelNO, pIEType, pSquenceNo, pDataabse, szXML);
    }


    char check_result[32] = {0};

    TiXmlElement *item_result = root->FirstChildElement("CHECK_RESULT");
    if(item_result)
    {
        char* pText = (char*) item_result->GetText();
        if(pText)
        {
            strcpy(check_result, pText);
        }
        else
        {
            strcpy(check_result, "");
        }
    }

    char op_hint[1024] = {0};
    TiXmlElement *item_hint = root->FirstChildElement("OP_HINT");
    if(item_hint)
    {
        char* pText = (char*) item_hint->GetText();
        if(pText)
        {
            strcpy(op_hint, pText);
        }
        else
        {
            strcpy(op_hint, "");
        }
    }


    char form_id[64] = {0};
    TiXmlElement *item_formid = root->FirstChildElement("FORM_ID");
    if(item_formid)
    {
        char* pText = (char*) item_formid->GetText();
        if(pText)
        {
            strcpy(form_id, pText);
        }
        else
        {
            strcpy(form_id, "");
        }
    }

    char seal_id[128] = {0};
    char seal_key[64] = {0};
    char open_times[32] = {0};

    TiXmlElement *item_seal_tag = root->FirstChildElement("SEAL");
    if(item_seal_tag)
    {
        TiXmlElement* item_eseal_id = item_seal_tag->FirstChildElement("ESEAL_ID");
        if(item_eseal_id)
        {
            char* pText = (char*) item_eseal_id->GetText();
            if(pText)
            {
                strcpy(seal_id, pText);
            }
            else
            {
                strcpy(seal_id, "");
            }

        }

        TiXmlElement* item_key = item_seal_tag->FirstChildElement("SEAL_KEY");
        if(item_key)
        {
            char* pText = (char*) item_key->GetText();
            if(pText)
            {
                strcpy(seal_key, pText);
            }
            else
            {
                strcpy(seal_key, "");
            }

        }


        TiXmlElement* item_times = item_seal_tag->FirstChildElement("OPEN_TIMES");
        if(item_times)
        {
            char* pText = (char*) item_times->GetText();
            if(pText)
            {
                strcpy(open_times, pText);
            }
            else
            {
                strcpy(open_times, "");
            }

        }
    }


    T_PassResultInfo resultInfo;
    memset(&resultInfo, 0, sizeof (T_PassResultInfo));

    strcpy(resultInfo.szAeraID, pAreaID);
    strcpy(resultInfo.szChlNo, pChannelNO);
    strcpy(resultInfo.szSeqNo, pSquenceNo);
    strcpy(resultInfo.szIEType, pIEType);

    strcpy(resultInfo.szCheckResult, check_result);
    strcpy(resultInfo.szOPHint, op_hint);
    strcpy(resultInfo.szFormID, form_id);
    strcpy(resultInfo.szSealID, seal_id);
    strcpy(resultInfo.szSealKey, seal_key);
    strcpy(resultInfo.szOpenTimes, open_times);

    return handlePassResult(resultInfo, pDataabse);

}

int CBasicDataAccess::handleICTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item)
{
    if(!item)
    {
        return -1;
    }

    char ic_no[32] = {0};
    TiXmlElement* item_ic_no = item->FirstChildElement("DR_IC_NO");
    if(item_ic_no)
    {
        char* pText = (char*) item_ic_no->GetText();
        if(pText)
        {
            strcpy(ic_no, pText);
        }
        else
        {
            strcpy(ic_no, "");
        }

    }


    char ic_driver[128] = {0};
    TiXmlElement* item_driver_no = item->FirstChildElement("IC_DR_CUSTOMS_NO");
    if(item_driver_no)
    {
        char* pText = (char*) item_driver_no->GetText();
        if(pText)
        {
            strcpy(ic_driver, pText);
        }
        else
        {
            strcpy(ic_driver, "");
        }

    }


    char ic_company[128] = {0};
    TiXmlElement* item_company_no = item->FirstChildElement("IC_CO_CUSTOMS_NO");
    if(item_company_no)
    {
        char* pText = (char*) item_company_no->GetText();
        if(pText)
        {
            strcpy(ic_company, pText);
        }
        else
        {
            strcpy(ic_company, "");
        }

    }


    char ic_bill[64] = {0};
    TiXmlElement* item_bill_no = item->FirstChildElement("IC_BILL_NO");
    if(item_bill_no)
    {
        char* pText = (char*) item_bill_no->GetText();
        if(pText)
        {
            strcpy(ic_bill, pText);
        }
        else
        {
            strcpy(ic_bill, "");
        }

    }


    char ic_grossweight[64] = {0};
    TiXmlElement* item_gross_weight = item->FirstChildElement("IC_GROSS_WT");
    if(item_gross_weight)
    {
        char* pText = (char*) item_gross_weight->GetText();
        if(pText)
        {
            strcpy(ic_grossweight, pText);
        }
        else
        {
            strcpy(ic_grossweight, "");
        }

    }


    char ic_ve_no[64] = {0};
    TiXmlElement* item_ve_customs_no = item->FirstChildElement("IC_VE_CUSTOMS_NO");
    if(item_ve_customs_no)
    {
        char* pText = (char*) item_ve_customs_no->GetText();
        if(pText)
        {
            strcpy(ic_ve_no, pText);
        }
        else
        {
            strcpy(ic_ve_no, "");
        }

    }


    char ic_ve_name[64] = {0};
    TiXmlElement* item_ve_name = item->FirstChildElement("IC_VE_NAME");
    if(item_ve_name)
    {
        char* pText = (char*) item_ve_name->GetText();
        if(pText)
        {
            strcpy(ic_ve_name, pText);
        }
        else
        {
            strcpy(ic_ve_name, "");
        }

    }


    char ic_conta_id[64] = {0};
    TiXmlElement* item_conta_id = item->FirstChildElement("IC_CONTA_ID");
    if(item_conta_id)
    {
        char* pText = (char*) item_conta_id->GetText();
        if(pText)
        {
            strcpy(ic_conta_id, pText);
        }
        else
        {
            strcpy(ic_conta_id, "");
        }

    }


    char ic_eseal_id[64] = {0};
    TiXmlElement* item_eseal_id = item->FirstChildElement("IC_ESEAL_ID");
    if(item_eseal_id)
    {
        char* pText = (char*) item_eseal_id->GetText();
        if(pText)
        {
            strcpy(ic_eseal_id, pText);
        }
        else
        {
            strcpy(ic_eseal_id, "");
        }

    }


    char ic_buss_type[64] = {0};
    TiXmlElement* item_buss_type = item->FirstChildElement("IC_BUSS_TYPE");
    if(item_eseal_id)
    {
        char* pText = (char*) item_buss_type->GetText();
        if(pText)
        {
            strcpy(ic_buss_type, pText);
        }
        else
        {
            strcpy(ic_buss_type, "");
        }

    }


    char ic_ex_data[512] = {0};
    TiXmlElement* item_ex_data = item->FirstChildElement("IC_EX_DATA");
    if(item_ex_data)
    {
        char* pText = (char*) item_ex_data->GetText();
        if(pText)
        {
            strcpy(ic_ex_data, pText);
        }
        else
        {
            strcpy(ic_ex_data, "");
        }

    }


    char cpYearMonDay[256] = {0};
    struct tm* tmNow;
    time_t tmTime = time(NULL);
    tmNow = localtime(&tmTime);
    sprintf(cpYearMonDay, "%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);


    char chQuerySQL[1024] = {0};
    sprintf(chQuerySQL, "INSERT INTO T_GATHERINFO_IC VALUES('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')"
            , seq_no
            , area_id
            , chnl_no
            , cpYearMonDay
            , ic_no
            , ic_driver
            , ic_company
            , ic_bill
            , ic_grossweight
            , ic_ve_no
            , ic_ve_name
            , ic_conta_id
            , ic_eseal_id
            , ic_buss_type
            , ic_ex_data);



    int nRet = pDatabase->execSQL(chQuerySQL);

    if(nRet != 1)
    {
        char chQuerySQL[1024] = {0};
        sprintf(chQuerySQL, "UPDATE T_GATHERINFO_IC SET DR_IC_NO='%s' ,IC_DR_CUSTOMS_NO ='%s',IC_CO_CUSTOMS_NO ='%s' ,IC_BILL_NO = '%s',\
                               IC_GROSS_WEIGHT='%s' ,IC_VE_CUSTOMS_NO='%s',IC_VE_NAME='%s',IC_CONTA_ID='%s' IC_ESEAL_ID='%s',IC_BUSS_TYPE='%s'\
                               ,IC_EX_DATA ='%s' WHERE SEQUENCE_NO='%s' "
                , ic_no
                , ic_driver
                , ic_company
                , ic_bill
                , ic_grossweight
                , ic_ve_no
                , ic_ve_name
                , ic_conta_id
                , ic_eseal_id
                , ic_buss_type
                , ic_ex_data
                , seq_no);


        pDatabase->execSQL(chQuerySQL);

    }

    return nRet;




}

int CBasicDataAccess::handleWeightTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item)
{
    if(!item)
    {
        return -1;
    }

    char weight[32] = {0};
    TiXmlElement* item_weight = item->FirstChildElement("GROSS_WT");
    if(item_weight)
    {
        char* pText = (char*) item_weight->GetText();
        if(pText)
        {
            strcpy(weight, pText);
        }
        else
        {
            strcpy(weight, "");
        }

    }



    char cpYearMonDay[256] = {0};
    struct tm* tmNow;
    time_t tmTime = time(NULL);
    tmNow = localtime(&tmTime);
    sprintf(cpYearMonDay, "%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);


    char chQuerySQL[1024] = {0};
    sprintf(chQuerySQL, "INSERT INTO T_GATHERINFO_WEIGHT VALUES('%s','%s','%s','%s','%s')"
            , seq_no
            , area_id
            , chnl_no
            , cpYearMonDay
            , weight);


    return pDatabase->execSQL(chQuerySQL);
}

int CBasicDataAccess::handleCarTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item)
{
    if(!item)
    {
        return -1;
    }

    char ve_name[128] = {0};
    TiXmlElement* item_ve_name = item->FirstChildElement("VE_NAME");
    if(item_ve_name)
    {
        char* pText = (char*) item_ve_name->GetText();
        if(pText)
        {
            strcpy(ve_name, pText);
        }
        else
        {
            strcpy(ve_name, "");
        }

    }


    char car_ec_no[64] = {0};
    TiXmlElement* item_ec_no = item->FirstChildElement("CAR_EC_NO");
    if(item_ec_no)
    {
        char* pText = (char*) item_ec_no->GetText();
        if(pText)
        {
            strcpy(car_ec_no, pText);
        }
        else
        {
            strcpy(car_ec_no, "");
        }

    }


    char car_ec_no2[64] = {0};
    TiXmlElement* item_ec_no2 = item->FirstChildElement("CAR_EC_NO2");
    if(item_ec_no2)
    {
        char* pText = (char*) item_ec_no2->GetText();
        if(pText)
        {
            strcpy(car_ec_no2, pText);
        }
        else
        {
            strcpy(car_ec_no2, "");
        }

    }



    char ve_customs_no[128] = {0};
    TiXmlElement* item_customs_no = item->FirstChildElement("VE_CUSTOMS_NO");
    if(item_customs_no)
    {
        char* pText = (char*) item_customs_no->GetText();
        if(pText)
        {
            strcpy(ve_customs_no, pText);
        }
        else
        {
            strcpy(ve_customs_no, "");
        }

    }


    char ve_weight[64] = {0};
    TiXmlElement* item_ve_weight = item->FirstChildElement("VE_WEIGHT");
    if(item_ve_weight)
    {
        char* pText = (char*) item_ve_weight->GetText();
        if(pText)
        {
            strcpy(ve_weight, pText);
        }
        else
        {
            strcpy(ve_weight, "");
        }

    }



    char cpYearMonDay[256] = {0};
    struct tm* tmNow;
    time_t tmTime = time(NULL);
    tmNow = localtime(&tmTime);
    sprintf(cpYearMonDay, "%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);


    char chQuerySQL[1024] = {0};
    sprintf(chQuerySQL, "INSERT INTO T_GATHERINFO_CAR VALUES('%s','%s','%s','%s','%s','%s','%s','%s','%s')"
            , seq_no
            , area_id
            , chnl_no
            , cpYearMonDay
            , ve_name
            , car_ec_no
            , car_ec_no2
            , ve_customs_no
            , ve_weight);


    int nRet = pDatabase->execSQL(chQuerySQL);

    if(nRet != 1)
    {
        char chQuerySQL[1024] = {0};
        sprintf(chQuerySQL, "UPDATE T_GATHERINFO_CAR SET VE_NAME='%s' ,CAR_EC_NO ='%s',CAR_EC_NO2 ='%s' ,VE_CUSTOMS_NO = '%s',VE_WEIGHT='%s' WHERE SEQUENCE_NO='%s' "
                , ve_name
                , car_ec_no
                , car_ec_no2
                , ve_customs_no
                , ve_weight
                , seq_no);


        pDatabase->execSQL(chQuerySQL);

    }

    return nRet;

}

int CBasicDataAccess::handleTrailerTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item)
{
    if(!item)
    {
        return -1;
    }

    char tr_ec_no[128] = {0};
    TiXmlElement* item_tr_ec_no = item->FirstChildElement("TR_EC_NO");
    if(item_tr_ec_no)
    {
        char* pText = (char*) item_tr_ec_no->GetText();
        if(pText)
        {
            strcpy(tr_ec_no, pText);
        }
        else
        {
            strcpy(tr_ec_no, "");
        }

    }


    char tr_name[64] = {0};
    TiXmlElement* item_tr_name = item->FirstChildElement("TR_NAME");
    if(item_tr_name)
    {
        char* pText = (char*) item_tr_name->GetText();
        if(pText)
        {
            strcpy(tr_name, pText);
        }
        else
        {
            strcpy(tr_name, "");
        }

    }


    char tr_wt[64] = {0};
    TiXmlElement* item_tr_wt = item->FirstChildElement("TR_WT");
    if(item_tr_wt)
    {
        char* pText = (char*) item_tr_wt->GetText();
        if(pText)
        {
            strcpy(tr_wt, pText);
        }
        else
        {
            strcpy(tr_wt, "");
        }

    }




    char cpYearMonDay[256] = {0};
    struct tm* tmNow;
    time_t tmTime = time(NULL);
    tmNow = localtime(&tmTime);
    sprintf(cpYearMonDay, "%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);


    char chQuerySQL[1024] = {0};
    sprintf(chQuerySQL, "INSERT INTO T_GATHERINFO_TRAILER VALUES('%s','%s','%s','%s','%s','%s','%s')"
            , seq_no
            , area_id
            , chnl_no
            , cpYearMonDay
            , tr_ec_no
            , tr_name
            , tr_wt);



    int nRet = pDatabase->execSQL(chQuerySQL);

    if(nRet != 1)
    {
        char chQuerySQL[1024] = {0};
        sprintf(chQuerySQL, "UPDATE T_GATHERINFO_TRAILER SET TR_EC_NO='%s' ,TR_NAME ='%s',TR_WEIGHT ='%s' WHERE SEQUENCE_NO='%s' "
                , tr_ec_no
                , tr_name
                , tr_wt
                , seq_no);


        pDatabase->execSQL(chQuerySQL);

    }




    return nRet;





}

int CBasicDataAccess::handleContaTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item)
{
    if(!item)
    {
        return -1;
    }

    char conta_num[128] = {0};
    int n_conta_num = 0;
    TiXmlElement* item_conta_num = item->FirstChildElement("CONTA_NUM");
    if(item_conta_num)
    {
        char* pText = (char*) item_conta_num->GetText();
        if(pText)
        {
            strcpy(conta_num, pText);
            n_conta_num = atoi(conta_num);
        }
        else
        {
            strcpy(conta_num, "");
        }

    }


    char conta_reco[64] = {0};
    int n_conta_reco = 0;
    TiXmlElement* item_conta_reco = item->FirstChildElement("CONTA_RECO");
    if(item_conta_reco)
    {
        char* pText = (char*) item_conta_reco->GetText();
        if(pText)
        {
            strcpy(conta_reco, pText);
            n_conta_reco = atoi(conta_reco);
        }
        else
        {
            strcpy(conta_reco, "");
        }

    }


    char conta_id_f[64] = {0};
    TiXmlElement* item_conta_id_f = item->FirstChildElement("CONTA_ID_F");
    if(item_conta_id_f)
    {
        char* pText = (char*) item_conta_id_f->GetText();
        if(pText)
        {
            strcpy(conta_id_f, pText);
        }
        else
        {
            strcpy(conta_id_f, "");
        }

    }


    char conta_id_b[64] = {0};
    TiXmlElement* item_conta_id_b = item->FirstChildElement("CONTA_ID_B");
    if(item_conta_id_b)
    {
        char* pText = (char*) item_conta_id_b->GetText();
        if(pText)
        {
            strcpy(conta_id_b, pText);
        }
        else
        {
            strcpy(conta_id_b, "");
        }

    }


    char conta_model_f[64] = {0};
    TiXmlElement* item_conta_model_f = item->FirstChildElement("CONTA_MODEL_F");
    if(item_conta_model_f)
    {
        char* pText = (char*) item_conta_model_f->GetText();
        if(pText)
        {
            strcpy(conta_model_f, pText);
        }
        else
        {
            strcpy(conta_model_f, "");
        }

    }



    char conta_model_b[64] = {0};
    TiXmlElement* item_conta_model_b = item->FirstChildElement("CONTA_MODEL_B");
    if(item_conta_model_b)
    {
        char* pText = (char*) item_conta_model_b->GetText();
        if(pText)
        {
            strcpy(conta_model_b, pText);
        }
        else
        {
            strcpy(conta_model_b, "");
        }

    }


    char cpYearMonDay[256] = {0};
    struct tm* tmNow;
    time_t tmTime = time(NULL);
    tmNow = localtime(&tmTime);
    sprintf(cpYearMonDay, "%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);


    char chQuerySQL[1024] = {0};
    sprintf(chQuerySQL, "INSERT INTO T_GATHERINFO_CONTA VALUES('%s','%s','%s','%s',%d,%d,'%s','%s','%s','%s')"
            , seq_no
            , area_id
            , chnl_no
            , cpYearMonDay
            , n_conta_num
            , n_conta_reco
            , conta_id_f
            , conta_id_b
            , conta_model_f
            , conta_model_b);



    int nRet = pDatabase->execSQL(chQuerySQL);

    if(nRet != 1)
    {
        char chQuerySQL[1024] = {0};
        sprintf(chQuerySQL, "UPDATE T_GATHERINFO_CONTA SET CONTA_NUM= %d,CONTA_RECO =%d ,CONTA_ID_F  = '%s' ,CONTA_ID_B ='%s',\
                               CONTA_MODEL_F ='%s' ,CONTA_MODEL_B = '%s' WHERE SEQUENCE_NO='%s' "
                , n_conta_num
                , n_conta_reco
                , conta_id_f
                , conta_id_b
                , conta_model_f
                , conta_model_b
                , seq_no);



        pDatabase->execSQL(chQuerySQL);

    }




    return nRet;


}

int CBasicDataAccess::handleSealTag(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, TiXmlElement *item)
{
    if(!item)
    {
        return -1;
    }

    char seal_id[128] = {0};
    TiXmlElement* item_eseal_id = item->FirstChildElement("ESEAL_ID");
    if(item_eseal_id)
    {
        char* pText = (char*) item_eseal_id->GetText();
        if(pText)
        {
            strcpy(seal_id, pText);
        }
        else
        {
            strcpy(seal_id, "");
        }

    }


    char ic_no[64] = {0};
    TiXmlElement* item_ic_no = item->FirstChildElement("ESEAL_IC_NO");
    if(item_ic_no)
    {
        char* pText = (char*) item_ic_no->GetText();
        if(pText)
        {
            strcpy(ic_no, pText);
        }
        else
        {
            strcpy(ic_no, "");
        }

    }



    char cpYearMonDay[256] = {0};
    struct tm* tmNow;
    time_t tmTime = time(NULL);
    tmNow = localtime(&tmTime);
    sprintf(cpYearMonDay, "%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);


    char chQuerySQL[1024] = {0};
    sprintf(chQuerySQL, "INSERT INTO T_GATHERINFO_SEAL VALUES('%s','%s','%s','%s','%s','%s')"
            , seq_no
            , area_id
            , chnl_no
            , cpYearMonDay
            , seal_id
            , ic_no);




    int nRet = pDatabase->execSQL(chQuerySQL);

    if(nRet != 1)
    {
        char chQuerySQL[1024] = {0};
        sprintf(chQuerySQL, "UPDATE T_GATHERINFO_SEAL SET ESEAL_ID = '%s' ,ESEAL_IC_NO='%s'  WHERE SEQUENCE_NO='%s' "
                , seal_id
                , ic_no
                , seq_no);



        pDatabase->execSQL(chQuerySQL);

    }




    return nRet;

}

int CBasicDataAccess::handleAll(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, char* szXML)
{
    char cpYearMonDay[256] = {0};
    struct tm* tmNow;
    time_t tmTime = time(NULL);
    tmNow = localtime(&tmTime);
    sprintf(cpYearMonDay, "%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);


    char chQuerySQL[1024 * 10] = {0};
    sprintf(chQuerySQL, "INSERT INTO T_GATHERINFO VALUES('%s','%s','%s','%s','%s')"
            , seq_no
            , area_id
            , chnl_no
            , cpYearMonDay
            , szXML);


    int nRet = pDatabase->execSQL(chQuerySQL);

    if(nRet != 1)
    {
        char chQuerySQL[1024 * 10] = {0};
        sprintf(chQuerySQL, "UPDATE T_GATHERINFO SET XML_INFO = '%s' WHERE SEQUENCE_NO='%s' "
                , szXML
                , seq_no);



        pDatabase->execSQL(chQuerySQL);

    }

    return nRet;

}

int CBasicDataAccess::handleResultAll(char* area_id, char* chnl_no, char* ie_type, char* seq_no, CppMySQL3DB* pDatabase, char* szXML)
{
    char cpYearMonDay[256] = {0};
    struct tm* tmNow;
    time_t tmTime = time(NULL);
    tmNow = localtime(&tmTime);
    sprintf(cpYearMonDay, "%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);


    char chQuerySQL[1024 * 10] = {0};
    sprintf(chQuerySQL, "INSERT INTO T_GATHERINFO_RESULTXML VALUES('%s','%s','%s','%s','%s')"
            , seq_no
            , area_id
            , chnl_no
            , cpYearMonDay
            , szXML);


    int nRet = pDatabase->execSQL(chQuerySQL);

    if(nRet != 1)
    {
        char chQuerySQL[1024 * 10] = {0};
        sprintf(chQuerySQL, "UPDATE T_GATHERINFO_RESULTXML SET XML_INFO = '%s' WHERE SEQUENCE_NO='%s' "
                , szXML
                , seq_no);



        pDatabase->execSQL(chQuerySQL);

    }

    return nRet;

}

int CBasicDataAccess::handlePassResult(T_PassResultInfo& resultInfo, CppMySQL3DB* pDatabase)
{
    char cpYearMonDay[256] = {0};
    struct tm* tmNow;
    time_t tmTime = time(NULL);
    tmNow = localtime(&tmTime);
    sprintf(cpYearMonDay, "%04d-%02d-%02d %02d:%02d:%02d", tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday, tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec);


    char chQuerySQL[1024 * 4] = {0};
    sprintf(chQuerySQL, "INSERT INTO T_GATHERINFO_RESULT VALUES('%s','%s','%s','%s','%s','%s','%s','%s','%s','%s')"
            , resultInfo.szSeqNo
            , resultInfo.szAeraID
            , resultInfo.szChlNo
            , cpYearMonDay
            , resultInfo.szCheckResult
            , resultInfo.szSealID
            , resultInfo.szSealKey
            , resultInfo.szOpenTimes
            , resultInfo.szFormID
            , resultInfo.szOPHint);


    return pDatabase->execSQL(chQuerySQL);
}

extern "C" CDataAccess* create()
{
    return new CBasicDataAccess;
}

extern "C" void destroy(CDataAccess* p)
{
    delete p;
}
