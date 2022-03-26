#pragma once
/**
 * @file TestDCMSetValidPinParamValidity.h
 * @brief Check the parameter validity of SetValidPin
 * @author Guangyun Wang
 * @date 2021/08/03
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMSetValidPinParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	map<BYTE, USHORT> mapSlot;
	nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath, TRUE);
	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No board inserted!");
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}

	nRetVal = dcm.SetValidPin(nullptr);
	XT_EXPECT_EQ(nRetVal, VECTOR_FILE_NOT_LOADED);
	if (VECTOR_FILE_NOT_LOADED != nRetVal)
	{
		//Channel is not set.
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("Not load vector before, but the return value(%d) is not euqal to VECTOR_FILE_NOT_LOADED(%d)!", nRetVal, VECTOR_FILE_NOT_LOADED);

		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}
	DCM dcm1;
	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);

	const int nTestCount = 6;
	BOOL bRight[nTestCount] = {FALSE,FALSE,FALSE,TRUE, TRUE, FALSE};
	char* lpszTestpinName[nTestCount] = { nullptr, " ", "NOPIN", "CH0,CH1", "CH0", "CH1"};
	int nIndex = 0;
	for (auto& PinName : lpszTestpinName)
	{
		nRetVal = dcm.SetValidPin(PinName);

		if (bRight[nIndex])
		{
			XT_EXPECT_EQ(nRetVal, EXECUTE_SUCCESS);
			if (EXECUTE_SUCCESS != nRetVal)
			{
				errMsg.AddNewError(STRING_ERROR_MSG);
				errMsg.SaveErrorMsg("The pin name list is right,the return value is %d!", nRetVal);
			}
		}
		else
		{
			XT_EXPECT_EQ(nRetVal, PIN_NAME_ERROR);
			if (PIN_NAME_ERROR != nRetVal)
			{
				errMsg.AddNewError(STRING_ERROR_MSG);
				if (nullptr == PinName)
				{
					errMsg.SaveErrorMsg("The pin name list is nullptr,the return value(%d) is not equal to PIN_NAME_ERROR(%d)!", nRetVal, PIN_NAME_ERROR);
				}
				else
				{
					errMsg.SaveErrorMsg("The pin name list is(%s),the return value(%d) is not equal to PIN_NAME_ERROR(%d)!", PinName, nRetVal, PIN_NAME_ERROR);
				}
			}
		}
		
		++nIndex;
	}
	
	errMsg.Print(this, g_lpszReportFilePath);
	dcm_I2CDeleteMemory();

	dcm_CloseFile();//Unload the vector file.
}