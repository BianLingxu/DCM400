#pragma once
/*!
* @file      TestDCMSetEdgeParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/18
* @version   v 1.0.0.0
* @brief     测试SetEdge参数有效性
* @comment
*/
#include "..\DCMTestMain.h"
void DataFormatToString(DataFormat dataFormat, char* cMode, int nArrayLength)
{
	switch (dataFormat)
	{
	case DCM_DTFT_NRZ:
		strcpy_s(cMode, nArrayLength, "DTFT_NRZ");
		break;
	case DCM_DTFT_RZ:
		strcpy_s(cMode, nArrayLength, "DTFT_RZ");
		break;
	case DCM_DTFT_RO:
		strcpy_s(cMode, nArrayLength, "DTFT_RO");
		break;
	case DCM_DTFT_SBL:
		strcpy_s(cMode, nArrayLength, "DTFT_SBL");
		break;
	case DCM_DTFT_SBH:
		strcpy_s(cMode, nArrayLength, "DTFT_SBH");
		break;
	case DCM_DTFT_SBC:
		strcpy_s(cMode, nArrayLength, "DTFT_SBC");
		break;
	default:
		break;
	}
}

XT_TEST(ParamValidityTest, TestDCMSetEdgeParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	double dPeriod = 0;
	int nRetVal = 0;
	const int nPinGroupTestCount = 3;
	char* lpszPinGroup[nPinGroupTestCount] = { nullptr, "G_NOPIN", "G_ALLPIN" };
	const int nTimeTestCount = 5;
	int nTimeTestValidIndex = 1;
	double dTimeTestValue[nTimeTestCount] = { -3, 50, 60, 70,100 };
	double dPeriodPerTimeSet[TIMESET_COUNT] = { 0 };

	map<BYTE, USHORT> mapSlot;
	GetBoardInfo(mapSlot, g_lpszVectorFilePath, TRUE);

	if (0 == mapSlot.size())
	{
		///<No board is inserted
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No board inserted!");
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}

	nRetVal = dcm.SetEdge("G_ALLPIN", "TestParamValidity", DCM_DTFT_NRZ, 10, 160, 10, 10);
	XT_EXPECT_EQ(VECTOR_FILE_NOT_LOADED, nRetVal);
	if (VECTOR_FILE_NOT_LOADED != nRetVal)
	{
		///<Not warning when vector is not loaded
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No Warning when vector is not loaded!");
	}
	///<Load vector
	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
	///<Defined pin group G_ALLPIN
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	char lpszTimesetName[32] = { 0 };
	_itoa_s(TIMESET_COUNT, lpszTimesetName, sizeof(lpszTimesetName), 10);

	nRetVal = dcm.SetEdge("G_ALLPIN", lpszTimesetName, DCM_DTFT_NRZ, DCM_IO_NRZ, 10, 50, 10, 50, 10);
	if (TIMESET_ERROR != nRetVal)
	{
		XT_EXPECT_EQ(nRetVal, TIMESET_ERROR);
		errMsg.AddNewError(STRING_ERROR_MSG, "G_ALLPIN", 2);
		errMsg.SetErrorItem("lpszPinGroup", "G_ALLPIN");
		errMsg.SaveErrorMsg("No warning when time set is error, the return value is %d!", nRetVal);
	}
	int nStringType = 0;
	for (int nPinGroupIndex = 0; nPinGroupIndex < nPinGroupTestCount; ++nPinGroupIndex)
	{
		nRetVal = dcm.SetEdge(lpszPinGroup[nPinGroupIndex], "0", DCM_DTFT_NRZ, 10, 50, 10, 10);
		nStringType = dcm_GetStringType(lpszPinGroup[nPinGroupIndex]);
		if (0 > nStringType || 2 == nStringType)
		{
			switch (nStringType)
			{
			case -2:
			case 2:
			case -3:
				XT_EXPECT_EQ(nRetVal, PIN_GROUP_ERROR);
				if (PIN_GROUP_ERROR != nRetVal)
				{
					errMsg.AddNewError(STRING_ERROR_MSG, lpszPinGroup[nPinGroupIndex], 2);
					errMsg.SetErrorItem("lpszPinGroup", lpszPinGroup[nPinGroupIndex]);
					errMsg.SaveErrorMsg("The pin group(%s) is error, but the return value(%d) is not equal to %d!",
						lpszPinGroup[nPinGroupIndex], nRetVal, PIN_GROUP_ERROR);
				}
				break;
			default:
				break;
			}
		}
		else
		{
			XT_EXPECT_EQ(nRetVal, EXECUTE_SUCCESS);
			if (EXECUTE_SUCCESS != nRetVal)
			{
				errMsg.AddNewError(STRING_ERROR_MSG);
				errMsg.SetErrorItem("lpszPinGroup", lpszPinGroup[nPinGroupIndex]);
				errMsg.SaveErrorMsg("The pin group(%s) is right,  but the return value(%d) is not equal to %d!", lpszPinGroup[nPinGroupIndex], nRetVal, 0);
			}
		}
	}

	int byTimeSet = 0;
	int nValidTimesetCount = TIMESET_COUNT - 1;

	char lpszDataFormat[20] = { 0 };
	char cIOFormat[20] = { 0 };
	char cCompareFormat[20] = { 0 };

	BYTE bySlot = mapSlot.begin()->first;
	for (byTimeSet = 0; byTimeSet < nValidTimesetCount; ++byTimeSet)
	{
		_itoa_s(byTimeSet, lpszTimesetName, sizeof(lpszTimesetName), 10);
		dPeriodPerTimeSet[byTimeSet] = dcm_GetTimeSetPeriod(bySlot, 0, byTimeSet);
	}

	//Check the CLK edge setting.
	byTimeSet = 2;
	dTimeTestValue[nTimeTestValidIndex] = dPeriodPerTimeSet[byTimeSet] * 1 / 4;
	dTimeTestValue[nTimeTestValidIndex + 1] = dPeriodPerTimeSet[byTimeSet] * 3 / 4;
	dTimeTestValue[nTimeTestCount - 2] = dPeriodPerTimeSet[byTimeSet];
	dTimeTestValue[nTimeTestCount - 1] = dPeriodPerTimeSet[byTimeSet] + 10;

	for (int nT1R = 0; nT1R < nTimeTestCount; ++nT1R)
	{
		for (int nT1F = 0; nT1F < nTimeTestCount; ++nT1F)
		{
			for (int nIOR = 0; nIOR < nTimeTestCount; ++nIOR)
			{
				for (int nIOF = 0; nIOF < nTimeTestCount; ++nIOF)
				{
					for (int nSTBR = 0; nSTBR < nTimeTestCount; ++nSTBR)
					{
						nRetVal = dcm.SetEdge("G_ALLPIN", "2", DCM_DTFT_NRZ, DCM_IO_NRZ, dTimeTestValue[nT1R], dTimeTestValue[nT1F],
							dTimeTestValue[nIOR], dTimeTestValue[nIOF], dTimeTestValue[nSTBR]);
						DataFormatToString(DCM_DTFT_NRZ, lpszDataFormat, 20);

						if (-EQUAL_ERROR > dTimeTestValue[nT1R] || -EQUAL_ERROR > dTimeTestValue[nT1F] 
							|| dPeriodPerTimeSet[2] + EQUAL_ERROR < dTimeTestValue[nT1R] || dPeriodPerTimeSet[2] + EQUAL_ERROR < dTimeTestValue[nT1F]
							|| -EQUAL_ERROR > dTimeTestValue[nIOR] || -EQUAL_ERROR > dTimeTestValue[nIOF]
							|| dPeriodPerTimeSet[2] + EQUAL_ERROR < dTimeTestValue[nIOR] || dPeriodPerTimeSet[2] + EQUAL_ERROR < dTimeTestValue[nIOF]
							|| -EQUAL_ERROR > dTimeTestValue[nSTBR] || dPeriodPerTimeSet[2] + EQUAL_ERROR < dTimeTestValue[nSTBR])
						{
							XT_EXPECT_EQ(nRetVal, TIME_ERROR);
							if (TIME_ERROR != nRetVal)
							{
								errMsg.AddNewError(STRING_ERROR_MSG, "G_ALLPIN", 2);
								char lpszMsg[256] = { 0 };
								sts_sprintf(lpszMsg, 256, "T1R:%.2f,T1F:%.2f,IOR:%.2f,STBR:%.2f", dTimeTestValue[nT1R], dTimeTestValue[nT1F],
									dTimeTestValue[nIOR], dTimeTestValue[nSTBR]);
								errMsg.SetErrorItem("CLK edge", lpszMsg);
								errMsg.SaveErrorMsg("The edge value is error but no warning, the return value(%d) is not equal to %d!", nRetVal, TIMESET_ERROR);
								continue;
							}
						}
						else
						{
							XT_EXPECT_EQ(nRetVal, 0);
							if (0 != nRetVal)
							{
								errMsg.AddNewError(STRING_ERROR_MSG, "G_ALLPIN", 2);
								errMsg.SaveErrorMsg("Unknown error, the return value is %d!", nRetVal);
							}
						}
					}
				}
			}
		}
	}

	errMsg.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
}