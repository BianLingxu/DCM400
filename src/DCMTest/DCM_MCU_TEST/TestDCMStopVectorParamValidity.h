/*!
* @file      TestDCMStopVectorParamValidity.h
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

XT_TEST(ParamValidityTest, TestDCMStopVectorParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	double dPeriod = 0;
	int nRetVal = 0;
	const int nPinGroupTestCount = 4;
	char* pinGroup[nPinGroupTestCount] = { nullptr, "G_NOPIN", "G_ALLPIN", "CH0" };


	int nValidBoardCount = dcm_GetValidBoard();

	if (0 == nValidBoardCount)
	{
		///<No board is inserted
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No board inserted.");
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}
	nRetVal = dcm.StopVector("G_ALLPIN");
	if (VECTOR_FILE_NOT_LOADED != nRetVal)
	{
		///<Not warning when vector is not loaded
		XT_EXPECT_EQ(VECTOR_FILE_NOT_LOADED, nRetVal);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No Warning when vector is not loaded!");
	}
	//Load vector.
	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
	//Defined pin group G_ALLPIN
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	int nStringType = 0;
	for (int nPinGroupTestIndex = 0; nPinGroupTestIndex < nPinGroupTestCount;++nPinGroupTestIndex)
	{
		nRetVal = dcm.StopVector(pinGroup[nPinGroupTestIndex]);
		nStringType = dcm_GetStringType(pinGroup[nPinGroupTestIndex]);
		if (0 > nStringType || 2 == nStringType)
		{
			if (PIN_GROUP_ERROR != nRetVal)
			{
				///<No warning when pin group is not set
				XT_EXPECT_EQ(nRetVal, PIN_GROUP_ERROR);
				errMsg.AddNewError(STRING_ERROR_MSG, pinGroup[nPinGroupTestIndex], 2);
				errMsg.SetErrorItem("PinGroup", pinGroup[nPinGroupTestIndex]);
				errMsg.SaveErrorMsg("No warning when pin group is not defined, the return value is %d", nRetVal);
			}
		}
		else
		{
			XT_EXPECT_EQ(nRetVal, EXECUTE_SUCCESS);
			if (EXECUTE_SUCCESS != nRetVal)
			{
				errMsg.AddNewError(STRING_ERROR_MSG, pinGroup[nPinGroupTestIndex]);
				errMsg.SaveErrorMsg("Unknown error, the return value is %d!", nRetVal);
			}
		}
	}


	errMsg.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
}