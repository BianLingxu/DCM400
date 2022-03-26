#pragma once
/*!
* @file      TestDCMI2CSetPinLevelParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/13
* @version   v 1.0.0.0
* @brief     测试I2CSetPinLevel参数有效性
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMI2CSetPinLevelParamValidity)
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

	nRetVal = dcm.I2CSetPinLevel(3.0, 0, 1.5, 0.8);
	if (-2 != nRetVal)
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

	USHORT uSiteCount = vecSDAChannel.size();

	dcm.I2CSet(500, uSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());

	//Check the pin level.
	for (int nVIHID = 0; nVIHID < nTestLevelCount; ++nVIHID)
	{
		for (int nVILID = 0; nVILID < nTestLevelCount; ++nVILID)
		{
			for (int nVOHID = 0; nVOHID < nTestLevelCount; ++nVOHID)
			{
				for (int nVOLID = 0; nVOLID < nTestLevelCount; ++nVOLID)
				{
					nRetVal = dcm.I2CSetPinLevel(dPinLevel[nVIHID], dPinLevel[nVILID], dPinLevel[nVOHID], dPinLevel[nVOLID]);

					if (-1.5 - EQUAL_ERROR > dPinLevel[nVIHID] || 6 + EQUAL_ERROR < dPinLevel[nVIHID] ||
						-1.5 - EQUAL_ERROR > dPinLevel[nVILID] || 6 + EQUAL_ERROR < dPinLevel[nVILID] ||
						-1.5 - EQUAL_ERROR > dPinLevel[nVOHID] || 6 + EQUAL_ERROR < dPinLevel[nVOHID] ||
						-1.5 - EQUAL_ERROR > dPinLevel[nVOLID] || 6 + EQUAL_ERROR < dPinLevel[nVOLID])
					{
						//The pin level is exceed the limit.
						XT_EXPECT_EQ(nRetVal, -1);
						if (-1 == nRetVal)
						{
							continue;
						}

						errMsg.AddNewError(STRING_ERROR_MSG);
						char cMsg[256] = { 0 };
						sts_sprintf(cMsg, 256, "VIH:%.1f, VIL:%.2f, VOH:%.2f, VOL:%.2f", dPinLevel[nVIHID], dPinLevel[nVILID], dPinLevel[nVOHID], dPinLevel[nVOLID]);
						errMsg.SetErrorItem("PinLevel", cMsg);
						errMsg.SaveErrorMsg("No Warning when pin level is over scale, the return value is %d/%d!", nRetVal, -2);
					}
					else if (0 != nRetVal)
					{
						XT_EXPECT_EQ(nRetVal, 0);

						errMsg.AddNewError(STRING_ERROR_MSG);
						char cMsg[256] = { 0 };
						sts_sprintf(cMsg, 256, "VIH:%.1f, VIL:%.2f, VOH:%.2f, VOL:%.2f", dPinLevel[nVIHID], dPinLevel[nVILID], dPinLevel[nVOHID], dPinLevel[nVOLID]);
						errMsg.SetErrorItem("PinLevel", cMsg);
						errMsg.SaveErrorMsg("Unknown error when pin level is in scale, the return value is %d/%d!", nRetVal, 0);
					}
					else
					{
						delay_ms(1);
						BYTE bySlot = 0;
						CHANNEL_INFO* pChannel = nullptr;
						for (USHORT usSiteNo = 0; usSiteNo < uSiteCount; ++usSiteNo)
						{
							for (BYTE byPinIndex = 0; byPinIndex < 2; ++byPinIndex)
							{
								if (0 == byPinIndex)
								{
									pChannel = &vecSCLChannel[usSiteNo];
								}
								else
								{
									pChannel = &vecSDAChannel[usSiteNo];
								}
								bySlot = pChannel->m_bySlotNo;
								usChannel = pChannel->m_usChannel;
								dcm_GetLevelSettingValue(bySlot, usChannel, DCM_VIH, dSetPinLevel[0]);
								dcm_GetLevelSettingValue(bySlot, usChannel, DCM_VIL, dSetPinLevel[1]);
								dcm_GetLevelSettingValue(bySlot, usChannel, DCM_VOH, dSetPinLevel[2]);
								dcm_GetLevelSettingValue(bySlot, usChannel, DCM_VOL, dSetPinLevel[3]);
								if (fabs(dSetPinLevel[0] - dPinLevel[nVIHID]) > dLevelResolution || fabs(dSetPinLevel[1] - dPinLevel[nVILID]) > dLevelResolution ||
									fabs(dSetPinLevel[2] - dPinLevel[nVOHID]) > dLevelResolution || fabs(dSetPinLevel[3] - dPinLevel[nVOLID]) > dLevelResolution)
								{
									//The Pin level in DCM is not equal to the pin level set.
									XT_EXPECT_TRUE(FALSE);

									errMsg.AddNewError(VALUE_ERROR);
									if (fabs(dSetPinLevel[0] - dPinLevel[nVIHID]) > dLevelResolution)
									{
										errMsg.SetErrorItem("VIH", nullptr, 1, bySlot, usChannel, VALUE_NOT_EQUAL);
										errMsg.SaveErrorMsg(dSetPinLevel[0], dPinLevel[nVIHID], dLevelResolution, VALUE_DOUBLE_FOUR_DECIMAL);
									}
									if (fabs(dSetPinLevel[1] - dPinLevel[nVILID]) > dLevelResolution)
									{
										errMsg.SetErrorItem("VIL", nullptr, 1, bySlot, usChannel, VALUE_NOT_EQUAL);
										errMsg.SaveErrorMsg(dSetPinLevel[1], dPinLevel[nVILID], dLevelResolution, VALUE_DOUBLE_FOUR_DECIMAL);
									}
									if (fabs(dSetPinLevel[2] - dPinLevel[nVOHID]) > dLevelResolution)
									{
										errMsg.SetErrorItem("VOH", nullptr, 1, bySlot, usChannel, VALUE_NOT_EQUAL);
										errMsg.SaveErrorMsg(dSetPinLevel[2], dPinLevel[nVOHID], dLevelResolution, VALUE_DOUBLE_FOUR_DECIMAL);
									}
									if (fabs(dSetPinLevel[3] - dPinLevel[nVOLID]) > dLevelResolution)
									{
										errMsg.SetErrorItem("VOL", nullptr, 1, bySlot, usChannel, VALUE_NOT_EQUAL);
										errMsg.SaveErrorMsg(dSetPinLevel[3], dPinLevel[nVOLID], dLevelResolution, VALUE_DOUBLE_FOUR_DECIMAL);
									}
								}
							}
						}
					}
				}
			}
		}
	}


	errMsg.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
	dcm_I2CDeleteMemory();
}