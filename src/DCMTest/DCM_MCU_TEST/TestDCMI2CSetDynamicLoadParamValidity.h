#pragma once
/**
 * @file TestDCMI2CSetDynamicLoadParamValidity.h
 * @brief Check the parameter validity of I2CSetDynamicLoad
 * @author Guangyun Wang
 * @date 2021/08/02
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMI2CSetDynamicLoadParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	const int nTestLevelCount = 10;
	const double dLevelResolution = 0.001;//The minimum resolution of pin level.
	double dPeriod = 0;
	int nRetVal = 0;
	double dPinLevel[nTestLevelCount] = { -3, -1.6, -1.5, 0.1, 3.2, 3.5, 6, 6.1, 7 };
	double dSetPinLevel[4] = { 0 };


	map<BYTE, USHORT> mapSlot;
	nRetVal = GetBoardInfo(mapSlot, nullptr);
	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No board inserted!");
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}

	nRetVal = dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, FALSE);
	XT_EXPECT_EQ(nRetVal, -1);
	if (-1 != nRetVal)
	{
		//Channel is not set.
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No warning when channel is not set!");

	}

	char lpszChannel[32] = { 0 };
	USHORT usChannel = 0;
	string strSCLChannel;
	string strSDAChannel;

	vector<CHANNEL_INFO> vecSDAChannel;
	vector<CHANNEL_INFO> vecSCLChannel;

	GetI2CChannel(mapSlot, strSCLChannel, strSDAChannel, vecSCLChannel, vecSDAChannel);

	const USHORT usSiteCount = vecSDAChannel.size();

	dcm.I2CSet(500, usSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());

	const int nVTTestCount = 5;
	const double VTTestValue[nVTTestCount] = { -2, -1.5, 5, 6, 6.5 };
	const int nCurrentTestCount = 5;
	const double dCurrentTestValue[nCurrentTestCount] = { -0.007, -0.006, 0, 0.018, 0.020 };
	const double dCurrentResolution = 1.5e-6;//The minumum resolution of IOH/IOL.
	const double dVoltageLevelResolution = 0.0006;//The minimum resolution of voltage level.
	const int nModeLength = 20;

	nRetVal = dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, DCM_CLOSE_CLAMP_CLOSE_LOAD);

	for (int nEnable = 0; nEnable <= 1; ++nEnable)
	{
		for (int nIOHIndex = 0; nIOHIndex < nCurrentTestCount; ++nIOHIndex)
		{
			for (int nIOLIndex = 0; nIOLIndex < nCurrentTestCount; ++nIOLIndex)
			{
				for (int nVTTestIndex = 0; nVTTestIndex < nVTTestCount; ++nVTTestIndex)
				{
					nRetVal = dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, nEnable, dCurrentTestValue[nIOHIndex], dCurrentTestValue[nIOLIndex], VTTestValue[nVTTestIndex]);
					if (EXECUTE_SUCCESS != nRetVal)
					{
						if (-3 == nRetVal)
						{
							if (-1.25 - EQUAL_ERROR > VTTestValue[nVTTestIndex] || 5.75 + EQUAL_ERROR < VTTestValue[nVTTestIndex])
							{
								continue;
							}
							else
							{

								XT_EXPECT_NE(nRetVal, -3);

								errMsg.AddNewError(STRING_ERROR_MSG, "CH0", 1);
								char cMsg[256] = { 0 };
								sts_sprintf(cMsg, 256, "VT:%.1fV", VTTestValue[nVTTestIndex]);
								errMsg.SetErrorItem("PinLevel", cMsg);
								errMsg.SaveErrorMsg("Warning when all pin level is right, the return value is %d!", nRetVal);

							}
						}
						else if (-2 == nRetVal)
						{
							if (-0.006 - EQUAL_ERROR > dCurrentTestValue[nIOHIndex] || 0.018 + EQUAL_ERROR < dCurrentTestValue[nIOHIndex] ||
								-0.006 - EQUAL_ERROR > dCurrentTestValue[nIOLIndex] || 0.018 + EQUAL_ERROR < dCurrentTestValue[nIOLIndex])
							{
								continue;
							}
							else
							{
								XT_EXPECT_NE(nRetVal, -2);

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
						auto* pChannel = &vecSCLChannel;
						for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
						{
							for (int nChannelType = 0; nChannelType < 2;++nChannelType)
							{
								if (0 == nChannelType)
								{
									pChannel = &vecSCLChannel;
								}
								else
								{
									pChannel = &vecSDAChannel;
								}
								for (auto& Channel : *pChannel)
								{
									dcm_GetLevelSettingValue(Channel.m_bySlotNo, Channel.m_usChannel, DCM_IOH, dIOH);
									dcm_GetLevelSettingValue(Channel.m_bySlotNo, Channel.m_usChannel, DCM_IOL, dIOL);
									dcm_GetLevelSettingValue(Channel.m_bySlotNo, Channel.m_usChannel, DCM_VT, dVT);
									dcm_GetDynamicLoadMode(Channel.m_bySlotNo, Channel.m_usChannel, nDynamicMode);

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

	dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, FALSE);
	errMsg.Print(this, g_lpszReportFilePath);
	dcm_I2CDeleteMemory();

	dcm_CloseFile();//Unload the vector file.
}