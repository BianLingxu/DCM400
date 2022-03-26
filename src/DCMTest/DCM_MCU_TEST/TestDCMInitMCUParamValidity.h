#pragma once
/*!
* @file      TestDCMInitMCUParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/09/27
* @version   v 1.0.0.0
* @brief     测试StopVector参数有效性
* @comment
*/
#include "..\DCMTestMain.h"

XT_TEST(ParamValidityTest, TestDCMInitMCUParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	const int nPinGroupTestCount = 4;
	char* lpszPinGroup[nPinGroupTestCount] = { nullptr, "G_NOPIN", "G_ALLPIN", "CH0" };

	int nValidBoardCount = dcm_GetValidBoard();

	if (0 == nValidBoardCount)
	{
		///<No board is inserted
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No board inserted!");
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}
	nRetVal = dcm.InitMCU("G_ALLPIN");
	if (VECTOR_FILE_NOT_LOADED != nRetVal)
	{
		///<Not warning when vector is not loaded
		XT_EXPECT_EQ(VECTOR_FILE_NOT_LOADED, nRetVal);
		errMsg.AddNewError(STRING_ERROR_MSG, "G_ALLPIN", 2);
		errMsg.SaveErrorMsg("No Warning when vector is not loaded!");
	}
	//Load vector.
	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
	//Defined pin group G_ALLPIN
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	int nStringType = 0;
	for(int nTestIndex = 0; nTestIndex < nPinGroupTestCount; ++nTestIndex)
	{
		nRetVal = dcm.InitMCU(lpszPinGroup[nTestIndex]);
		nStringType = dcm_GetStringType(lpszPinGroup[nTestIndex]);
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
					errMsg.AddNewError(STRING_ERROR_MSG, lpszPinGroup[nTestIndex], 2);
					errMsg.SetErrorItem("lpszPinGroup", lpszPinGroup[nTestIndex]);
					errMsg.SaveErrorMsg("The pin group(%s) is error, but the return value(%d) is not equal to %d!",
						lpszPinGroup[nTestIndex], nRetVal, PIN_GROUP_ERROR);
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
				errMsg.SetErrorItem("lpszPinGroup", lpszPinGroup[nTestIndex]);
				errMsg.SaveErrorMsg("The pin group(%s) is right, but the return value(%d) is not equal to %d!", lpszPinGroup[nTestIndex], nRetVal, 0);
			}
		}
	}

	errMsg.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();//Unload the vector file.
}