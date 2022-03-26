#pragma once
/*!
* @file      TestDCMSetSiteWaveDataParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2020/01/06
* @version   v 1.0.0.0
* @brief     测试SetSiteWaveData参数有效性
* @comment
*/
#include "..\DCMTestMain.h"

XT_TEST(ParamValidityTest, TestDCMSetSiteWaveDataParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	const int nPinGroupTestCount = 3;
	char* pinGroup[nPinGroupTestCount] = { nullptr, "G_NOPIN", "G_ALLPIN" };
	const int nTestCount = 5;
	const char* lpszTestLabel[nTestCount] = { nullptr, "NO_LABEL", "", "FIRST_ST", "FIRST_SP" };
	const int nTestSiteCount = 4;


	nRetVal = dcm.SetSiteWaveData(0, nullptr);
	if (VECTOR_FILE_NOT_LOADED != nRetVal)
	{
		//Not warning when vector is not loaded.
		XT_EXPECT_EQ(VECTOR_FILE_NOT_LOADED, nRetVal);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No warning when vector is not loaded!");
	}

	map<BYTE, USHORT> mapSlot;
	nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);
	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No board inserted!");
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}

	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("Load vector(%s) fail.", g_lpszVectorFilePath);
		mapSlot.clear();
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}
	//Defined pin group G_ALLPIN
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");


	nRetVal = dcm.SetSiteWaveData(0, nullptr);
	if (FUNCTION_USE_ERROR != nRetVal)
	{
		//Not warning when vector is not loaded.
		XT_EXPECT_EQ(FUNCTION_USE_ERROR, nRetVal);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No warning when not set SetWaveDataParam!");
	}

	dcm.SetWaveDataParam("G_ALLPIN", "FIRST_ST", 0, 8);
	nRetVal = dcm.WriteWaveData();
	if (0 != nRetVal)
	{
		//Not warning when vector is not loaded.
		XT_EXPECT_EQ(PIN_GROUP_ERROR, nRetVal);
		errMsg.SaveErrorMsg("Unknown error, the return value is %d!", nRetVal);
	}

	mapSlot.clear();
	errMsg.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();//Unload the vector file.
}