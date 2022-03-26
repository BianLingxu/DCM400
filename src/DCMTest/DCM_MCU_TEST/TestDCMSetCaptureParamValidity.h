#pragma once
/**
 * @file TestDCMSetCaptureParamValidity.h
 * @brief Check the parameter validity of SetCapture
 * @author Guangyun Wang
 * @date 2021/04/21
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMSetCaptureParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	const int nPinNameTestCount = 3;
	char* lpszPinName[nPinNameTestCount] = { nullptr, "NOPIN", "CH0" };
	const int nTestCount = 4;
	const char* LabelTestValue[nTestCount] = { nullptr, "NO_LABEL", "", "FIRST_ST" };

	DWORD dwCaptureData = 0;
	nRetVal = dcm.GetCaptureData(nullptr, "FIRST_ST", 0, 0, 0, dwCaptureData);
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

	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	dcm.RunVectorWithGroup("G_ALLPIN", "FIRST_ST", "FIRST_SP");

	int nRanStartLineNo = dcm_GetLabelLineNo("FIRST_ST");
	int nRanStopLineNo = dcm_GetLabelLineNo("FIRST_SP");

	USHORT uSiteCount = dcm_GetVectorSiteCount() + 1;
	int nExpectValue = 0;
	int nCorrectValue = 0;
	int nLineCount = 20;
	int nLabelLineNo = 0;
	for (auto& PinName : lpszPinName)
	{
		for (USHORT usSiteNo = 0; usSiteNo < uSiteCount; ++usSiteNo)
		{
			for (auto& Label : LabelTestValue)
			{
				nCorrectValue = 0;
				nRetVal = dcm.SetCapture(PinName, Label, usSiteNo, 0, nLineCount);
				nLabelLineNo = dcm_GetLabelLineNo(Label);
				if (0 > nLabelLineNo)
				{
					nCorrectValue = START_LABEL_ERROR;
				}
				nExpectValue = CheckParamValidity(errMsg, mapSlot, nRetVal, PinName, 1, usSiteNo, nCorrectValue, TRUE, 
					Label, TRUE);


				XT_EXPECT_EQ(nRetVal, nExpectValue);
			}
		}
	}
	nRetVal = dcm.SetCapture("G_ALLPIN", "FIRST_SP", mapSlot.begin()->second, 0, 10);
	XT_EXPECT_EQ(nRetVal, VECTOR_NOT_IN_LAST_RAN);
	if (VECTOR_NOT_IN_LAST_RAN != nRetVal)
	{
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("The vector is not in latest ran, but the return value(%d) is not equal to VECTOR_NOT_IN_LAST_RAN(%d).", nRetVal , VECTOR_NOT_IN_LAST_RAN);
	}
	mapSlot.clear();
	errMsg.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();//Unload the vector file.
}