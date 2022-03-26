#pragma once
/*!
* @file      TestDCMSetDynamicLoadParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/10
* @version   v 1.0.0.0
* @brief     测试SetDynamicLoad参数有效性
* @comment
*/
#include "..\DCMTestMain.h"
void inline DynamicModeToString(DynamicMode mode, char* cMode, int nArrayLength)
{
	switch (mode)
	{
	case DCM_CLOSE_CLAMP_CLOSE_LOAD:
		strcpy_s(cMode, nArrayLength, "No_Clamp_No_Load");
		break;
	case DCM_CLOSE_CLAMP_OPEN_LOAD:
		strcpy_s(cMode, nArrayLength, "No_Clamp_Load");
		break;
	case DCM_OPEN_CLAMP_CLOSE_LOAD:
		strcpy_s(cMode, nArrayLength, "Clamp_No_Load");
		break;
	case DCM_OPEN_CLAMP_OPEN_LOAD:
		strcpy_s(cMode, nArrayLength, "Clamp_And_Load");
		break;
	default:
		break;
	}
}

XT_TEST(ParamValidityTest, TestDCMSetDynamicLoadParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	const int nPinNameTestCount = 4;
	char* pinName[nPinNameTestCount] = { nullptr, " ", "CH44", "CH0" };
	const int nVTTestCount = 5;
	const double VTTestValue[nVTTestCount] = { -2, -1.5, 5, 6, 6.5 };
	const int nCurrentTestCount = 5;
	const double dCurrentTestValue[nCurrentTestCount] = { -0.007, -0.006, 0, 0.018, 0.020 };
	
	const double dCurrentResolution = 1.5e-6;//The minumum resolution of IOH/IOL.
	const double dVoltageLevelResolution = 0.0006;//The minimum resolution of voltage level.
	const int nModeLength = 20;
	char cDynamicMode[nModeLength] = { 0 };
	
	nRetVal = dcm.SetDynamicLoad(nullptr, 0);
	if (VECTOR_FILE_NOT_LOADED != nRetVal)
	{
		//Not warning when vector is not loaded.
		XT_EXPECT_EQ(nRetVal, VECTOR_FILE_NOT_LOADED);

		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No Warning when vector is not loaded!");
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
	auto iterSlot = mapSlot.begin();
	int nExpectRetVal = 0;
	USHORT uSiteCount = dcm_GetVectorSiteCount() + 1;

	for (int nPinNameTestIndex = 0; nPinNameTestIndex < nPinNameTestCount; ++nPinNameTestIndex)
	{
		for (int nSiteTestIndex = 0; nSiteTestIndex < uSiteCount; ++nSiteTestIndex)
		{
			nRetVal = dcm.SetDynamicLoad(pinName[nPinNameTestIndex], nSiteTestIndex, DCM_OPEN_CLAMP_OPEN_LOAD);
			nExpectRetVal = CheckParamValidity(errMsg, mapSlot, nRetVal, pinName[nPinNameTestIndex], 0, nSiteTestIndex, 0);
			XT_EXPECT_EQ(nRetVal, nExpectRetVal);
		}
	}
	int nCheckMode = 0;
	for (DynamicMode DynamicLoadMode = DCM_CLOSE_CLAMP_CLOSE_LOAD; DynamicLoadMode < DCM_OPEN_CLAMP_OPEN_LOAD; DynamicLoadMode = (DynamicMode)(DynamicLoadMode + 1))
	{
		switch (DynamicLoadMode)
		{
		case DCM_CLOSE_CLAMP_CLOSE_LOAD:
			nCheckMode = 0;
			break;
		case DCM_CLOSE_CLAMP_OPEN_LOAD:
			nCheckMode = 1;
			break;
		case DCM_OPEN_CLAMP_CLOSE_LOAD:
			nCheckMode = 0;
			break;
		case DCM_OPEN_CLAMP_OPEN_LOAD:
			nCheckMode = 1;
			break;
		default:
			break;
		}
		for (int nIOHIndex = 0; nIOHIndex < nCurrentTestCount; ++nIOHIndex)
		{
			for (int nIOLIndex = 0; nIOLIndex < nCurrentTestCount; ++nIOLIndex)
			{
				for (int nVTTestIndex = 0; nVTTestIndex < nVTTestCount; ++nVTTestIndex)
				{
					nRetVal = dcm.SetDynamicLoad("CH0", mapSlot.begin()->second, DynamicLoadMode, dCurrentTestValue[nIOHIndex],
						dCurrentTestValue[nIOLIndex], VTTestValue[nVTTestIndex]);
					DynamicModeToString(DynamicLoadMode, cDynamicMode, nModeLength);
					if (EXECUTE_SUCCESS != nRetVal)
					{
						if (PIN_LEVEL_ERROR == nRetVal)
						{
							if (-1.25 - EQUAL_ERROR > VTTestValue[nVTTestIndex] || 5.75 + EQUAL_ERROR < VTTestValue[nVTTestIndex])
							{
								continue;
							}
							else
							{

								XT_EXPECT_NE(nRetVal, PIN_LEVEL_ERROR);

								errMsg.AddNewError(STRING_ERROR_MSG, "CH0", 1);
								char cMsg[256] = { 0 };
								sts_sprintf(cMsg, 256, "VT:%.1fV", VTTestValue[nVTTestIndex]);
								errMsg.SetErrorItem("PinLevel", cMsg);
								errMsg.SaveErrorMsg("Warning when all pin level is right, the return value is %d!", nRetVal);

							}
						}
						else if (PIN_CURRENT_ERROR == nRetVal)
						{
							if (-0.006 - EQUAL_ERROR > dCurrentTestValue[nIOHIndex] || 0.018 + EQUAL_ERROR < dCurrentTestValue[nIOHIndex] ||
								-0.006 - EQUAL_ERROR > dCurrentTestValue[nIOLIndex] || 0.018 + EQUAL_ERROR < dCurrentTestValue[nIOLIndex])
							{
								continue;
							}
							else
							{

								XT_EXPECT_NE(nRetVal, PIN_CURRENT_ERROR);

								errMsg.AddNewError(STRING_ERROR_MSG, "CH0", 1);
								char cMsg[256] = { 0 };
								sts_sprintf(cMsg, 256, "IOH:%.3fA, IOL:%.3fA", dCurrentTestValue[nIOHIndex], dCurrentTestValue[nIOLIndex]);
								errMsg.SetErrorItem("Current", cMsg);
								errMsg.SaveErrorMsg("Warning when all current setting value is right, the return value is %d!", nRetVal);
							}
						}
						else
						{
							errMsg.AddNewError(STRING_ERROR_MSG, "CH0", 1);
							errMsg.SaveErrorMsg("Unknown error, the return value is %d!", nRetVal);
						}
					}
					else
					{
						double dIOH(0), dIOL(0), dVT(0), dClampHigh(0), dClampLow(0);
						BYTE bySlotNo = 0;
						int nChannel = 0;
						int nDynamicMode = 0;
						dcm_ConvertPinNameToChannel("CH0", mapSlot.begin()->second, bySlotNo, nChannel);
						dcm_GetLevelSettingValue(bySlotNo, nChannel, DCM_IOH, dIOH);
						dcm_GetLevelSettingValue(bySlotNo, nChannel, DCM_IOL, dIOL);
						dcm_GetLevelSettingValue(bySlotNo, nChannel, DCM_VT, dVT);
						dcm_GetDynamicLoadMode(bySlotNo, nChannel, nDynamicMode);

						if (fabs(dIOH - dCurrentTestValue[nIOHIndex]) > dCurrentResolution || fabs(dIOL - dCurrentTestValue[nIOLIndex]) > dCurrentResolution ||
							fabs(dVT - VTTestValue[nVTTestIndex]) > dVoltageLevelResolution )
						{
							XT_EXPECT_REAL_EQ(dIOH, dCurrentTestValue[nIOHIndex], dCurrentResolution);
							XT_EXPECT_REAL_EQ(dIOL, dCurrentTestValue[nIOLIndex], dCurrentResolution);
							XT_EXPECT_REAL_EQ(dVT, VTTestValue[nVTTestIndex], dVoltageLevelResolution);
							errMsg.AddNewError(VALUE_ERROR, "CH0", 1);
							if (fabs(dIOH - dCurrentTestValue[nIOHIndex]) > dCurrentResolution)
							{
								errMsg.SetErrorItem("IOH", nullptr, 1, 0, VALUE_NOT_EQUAL);
								errMsg.SaveErrorMsg(dIOH, dCurrentTestValue[nIOHIndex], dCurrentResolution, VALUE_DOUBLE_THREE_DECIMAL);
							}
							if (fabs(dIOL - dCurrentTestValue[nIOLIndex]) > dCurrentResolution)
							{
								errMsg.SetErrorItem("IOL", nullptr, 1, 0, VALUE_NOT_EQUAL);
								errMsg.SaveErrorMsg(dIOL, dCurrentTestValue[nIOLIndex], dCurrentResolution, VALUE_DOUBLE_THREE_DECIMAL);
							}
							if (fabs(dVT - VTTestValue[nVTTestIndex]) > dVoltageLevelResolution)
							{
								errMsg.SetErrorItem("VT", nullptr, 1, 0, VALUE_NOT_EQUAL);
								errMsg.SaveErrorMsg(dVT, VTTestValue[nVTTestIndex], dVoltageLevelResolution, VALUE_DOUBLE_THREE_DECIMAL);
							}
						}
						else if (nDynamicMode != nCheckMode)
						{
							XT_EXPECT_TRUE(FALSE);
							char cRegMode[nModeLength] = { 0 };
							DynamicModeToString(DynamicMode(nCheckMode), cRegMode, nModeLength);
							errMsg.AddNewError(STRING_ERROR_MSG, "CH0", 1);
							errMsg.SetErrorItem("DynamicMode", cRegMode);
							errMsg.SaveErrorMsg("The dynamic mode is not equal to seting value, the return value is %d", nRetVal);
						}
					}
				}
			}
		}
	}

	dcm.SetDynamicLoad("SDA_DP", 0, DCM_OPEN_CLAMP_OPEN_LOAD, 0.010, 0.010, 3.5, 7, -1);

	dcm.SetDynamicLoad("CH0", mapSlot.begin()->second);

	errMsg.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
}