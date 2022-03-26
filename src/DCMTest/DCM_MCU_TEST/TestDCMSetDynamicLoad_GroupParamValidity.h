#pragma once
/**
 * @file TestDCMSetDynamicLoad_GroupParamValidity.h
 * @brief Check the function of SetDynamicLoad
 * @author Guangyun Wang
 * @date 2021/06/08
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMSetDynamicLoad_GroupParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	const int nPinNameTestCount = 5;
	char* lpszPinGroupItem[nPinNameTestCount] = { nullptr, " ", "G_NOPIN", "CH0", "G_ALLPIN"};
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
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	auto iterSlot = mapSlot.begin();
	int nExpectRetVal = 0;
	USHORT uSiteCount = dcm_GetVectorSiteCount() + 1;

	int nPinGroupType = 0;
	for (auto lpszPinGroup : lpszPinGroupItem)
	{
		nRetVal = dcm.SetDynamicLoad(lpszPinGroup, false, 0.01, 0.01, 3.5);
		nPinGroupType = dcm_GetStringType(lpszPinGroup);
		if (0 != nPinGroupType && 1 != nPinGroupType)
		{
			XT_EXPECT_EQ(nRetVal, PIN_GROUP_ERROR);
			if (nRetVal != PIN_GROUP_ERROR)
			{
				errMsg.AddNewError(STRING_ERROR_MSG);
				if (nullptr != lpszPinGroup)
				{
					errMsg.SaveErrorMsg("The pin group is nullptr, but the return value(%d) is not equal to PIN_GROUP_ERROR(%d)", nRetVal, PIN_GROUP_ERROR);
				}
				else
				{
					errMsg.SaveErrorMsg("The pin group(%s) is not existed, but the return value(%d) is not equal to PIN_GROUP_ERROR(%d)", lpszPinGroup, nRetVal, PIN_GROUP_ERROR);
				}
			}
		}
		else
		{
			XT_EXPECT_EQ(nRetVal, 0);
			if (0 != nRetVal)
			{
				errMsg.SaveErrorMsg("The pin group(%s) is existed, but the return value(%d) is not equal to 0", lpszPinGroup, nRetVal);
			}
		}
	}
	nRetVal = dcm.SetDynamicLoad("G_ALLPIN", DCM_CLOSE_CLAMP_CLOSE_LOAD);

	vector<BYTE> vecPinNo = { 0,1 };
	dcm.SetPinGroup("G_TWO", "CH0,CH1");

	for (int nEnable = 0; nEnable <= 1; ++ nEnable)
	{
		for (int nIOHIndex = 0; nIOHIndex < nCurrentTestCount; ++nIOHIndex)
		{
			for (int nIOLIndex = 0; nIOLIndex < nCurrentTestCount; ++nIOLIndex)
			{
				for (int nVTTestIndex = 0; nVTTestIndex < nVTTestCount; ++nVTTestIndex)
				{
					nRetVal = dcm.SetDynamicLoad("G_TWO", nEnable, dCurrentTestValue[nIOHIndex], dCurrentTestValue[nIOLIndex], VTTestValue[nVTTestIndex]);
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
						double dIOH(0), dIOL(0), dVT(0);
						int nDynamicMode = 0;
						USHORT usSiteNo = 0;
						for (auto& Slot : mapSlot)
						{
							usSiteNo = Slot.second;
							for (BYTE byController = 0; byController < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byController, ++usSiteNo)
							{
								for (auto Pin : vecPinNo)
								{
									USHORT usChannel = byController * DCM_CHANNELS_PER_CONTROL + Pin;

									dcm_GetLevelSettingValue(Slot.first, usChannel, DCM_IOH, dIOH);
									dcm_GetLevelSettingValue(Slot.first, usChannel, DCM_IOL, dIOL);
									dcm_GetLevelSettingValue(Slot.first, usChannel, DCM_VT, dVT);
									dcm_GetDynamicLoadMode(Slot.first, usChannel, nDynamicMode);

									if (fabs(dIOH - dCurrentTestValue[nIOHIndex]) > dCurrentResolution || fabs(dIOL - dCurrentTestValue[nIOLIndex]) > dCurrentResolution ||
										fabs(dVT - VTTestValue[nVTTestIndex]) > dVoltageLevelResolution)
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
									else if (nEnable != nDynamicMode)
									{
										XT_EXPECT_TRUE(FALSE);
										char cRegMode[nModeLength] = { 0 };
										errMsg.AddNewError(STRING_ERROR_MSG, "CH0", 1);
										errMsg.SetErrorItem("DynamicMode", cRegMode);
										errMsg.SaveErrorMsg("The dynamic mode(%s) is not equal to seting value(%s), the return value is %d", nRetVal, nDynamicMode ? "Enable" : "Disable", 0 != nEnable ? "Enable" : "Disable");
									}
								}
							}
						}
					}
				}
			}
		}
	}

	nRetVal = dcm.SetDynamicLoad("G_ALLPIN", DCM_CLOSE_CLAMP_CLOSE_LOAD);

	errMsg.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
}