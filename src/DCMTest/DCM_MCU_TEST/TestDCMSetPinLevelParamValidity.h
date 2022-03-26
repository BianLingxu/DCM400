#pragma once
/*!
* @file      TestDCMSetPinLevelParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/09/25
* @version   v 1.0.0.0
* @brief     测试SetPinLevel参数有效性
* @comment
*/
#include "..\DCMTestMain.h"

XT_TEST(ParamValidityTest, TestDCMSetPinLevelParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	const int nTestLevelCount = 10;
	const double dLevelResolution = 0.001;//The minimum resolution of pin level.

	double dPeriod = 0;
	int nRetVal = 0;
	const int nPinGroupTestCount = 3;
	char* lpszTestPinGroup[nPinGroupTestCount] = { nullptr, "G_NOPIN", "G_ALLPIN" };
	double dPinLevel[nTestLevelCount] = { -3, -1.6, -1.5, 0.1, 3.2, 3.5, 6, 6.1, 7 };
	double dSetPinLevel[4] = { 0 };


	map<BYTE, USHORT> mapSlot;
	GetBoardInfo(mapSlot, g_lpszVectorFilePath, TRUE);
	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		errMsg.SaveErrorMsg("No board inserted.");
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}

	///<Load vector
	nRetVal = dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		errMsg.SetErrorItem("Load vector");
		char lpszMsg[256] = { 0 };
		sprintf_s(lpszMsg, sizeof(lpszMsg), "Load vector(%s) fail.", g_lpszVectorFilePath);
		errMsg.AddNewError(STRING_ERROR_MSG, lpszMsg);
		mapSlot.clear();
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}
	///<Defined pin group G_ALLPIN
	USHORT puSite[MAX_SITE] = { 0 };
	USHORT uSiteCount = 0;
	for (auto& Slot : mapSlot)
	{
		uSiteCount = dcm_GetSlotSite(Slot.first, puSite, MAX_SITE);
		if (0 < uSiteCount)
		{
			USHORT uFirstSite = puSite[0];
			for (USHORT usSiteIndex = 1; usSiteIndex < uSiteCount; ++usSiteIndex)
			{
				if (MAX_SITE <= usSiteIndex)
				{
					break;
				}
				if (uFirstSite > puSite[usSiteIndex])
				{
					uFirstSite = puSite[usSiteIndex];
				}
			}
			Slot.second = uFirstSite;
		}
	}
	//Defined pin group G_ALLPIN
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	int nStringType = 0;
	for (auto& lpszPinGroup : lpszTestPinGroup)
	{
		nRetVal = dcm.SetPinLevel(lpszPinGroup, 3.0, 0, 1.5, 0.8);
		nStringType = dcm_GetStringType(lpszPinGroup);
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
					errMsg.AddNewError(STRING_ERROR_MSG, lpszPinGroup, 2);
					errMsg.SetErrorItem("lpszPinGroup", lpszPinGroup);
					errMsg.SaveErrorMsg("The pin group(%s) is error, but the return value(%d) is not equal to %d!",
						lpszPinGroup, nRetVal, PIN_GROUP_ERROR);
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
				errMsg.SetErrorItem("lpszPinGroup", lpszPinGroup);
				errMsg.SaveErrorMsg("The pin group(%s) is right, but the return value(%d) is not equal to %d!", lpszPinGroup, nRetVal, 0);
			}
		}
	}


	//Check the pin level.
	for (int nVIHID = 0; nVIHID < nTestLevelCount; ++nVIHID)
	{
		for (int nVILID = 0; nVILID < nTestLevelCount; ++nVILID)
		{
			for (int nVOHID = 0; nVOHID < nTestLevelCount; ++nVOHID)
			{
				for (int nVOLID = 0; nVOLID < nTestLevelCount; ++nVOLID)
				{
					nRetVal = dcm.SetPinLevel("G_ALLPIN", dPinLevel[nVIHID], dPinLevel[nVILID], dPinLevel[nVOHID], dPinLevel[nVOLID]);
					if (0 <= nRetVal)
					{
						delay_ms(20);
						auto iterSlot = mapSlot.begin();
						while (mapSlot.end() != iterSlot)
						{
							BYTE bySlotNo = iterSlot->first;
							for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
							{
								dcm_GetLevelSettingValue(bySlotNo, usChannel, DCM_VIH, dSetPinLevel[0]);
								dcm_GetLevelSettingValue(bySlotNo, usChannel, DCM_VIL, dSetPinLevel[1]);
								dcm_GetLevelSettingValue(bySlotNo, usChannel, DCM_VOH, dSetPinLevel[2]);
								dcm_GetLevelSettingValue(bySlotNo, usChannel, DCM_VOL, dSetPinLevel[3]);

								XT_EXPECT_REAL_EQ(dSetPinLevel[0], dPinLevel[nVIHID], dLevelResolution);
								XT_EXPECT_REAL_EQ(dSetPinLevel[1], dPinLevel[nVILID], dLevelResolution);
								XT_EXPECT_REAL_EQ(dSetPinLevel[2], dPinLevel[nVOHID], dLevelResolution);
								XT_EXPECT_REAL_EQ(dSetPinLevel[3], dPinLevel[nVOLID], dLevelResolution);
								//dcm_GetPinLevel(nBoardId, usChannel, dSetPinLevel[0], dSetPinLevel[1], dSetPinLevel[2], dSetPinLevel[3]);
								if (fabs(dSetPinLevel[0] - dPinLevel[nVIHID]) > dLevelResolution || fabs(dSetPinLevel[1] - dPinLevel[nVILID]) > dLevelResolution ||
									fabs(dSetPinLevel[2] - dPinLevel[nVOHID]) > dLevelResolution || fabs(dSetPinLevel[3] - dPinLevel[nVOLID]) > dLevelResolution)
								{
									///<The Pin level in DCM is not equal to the pin level set
									errMsg.AddNewError(VALUE_ERROR, "G_ALLPIN", 2);
									if (fabs(dSetPinLevel[0] - dPinLevel[nVIHID]) > dLevelResolution)
									{
										errMsg.SetErrorItem("VIH", nullptr, 1, bySlotNo, usChannel, VALUE_NOT_EQUAL);
										errMsg.SaveErrorMsg(dSetPinLevel[0], dPinLevel[nVIHID], dLevelResolution, VALUE_DOUBLE_FOUR_DECIMAL);
									}
									if (fabs(dSetPinLevel[1] - dPinLevel[nVILID]) > dLevelResolution)
									{
										errMsg.SetErrorItem("VIL", nullptr, 1, bySlotNo, usChannel, VALUE_NOT_EQUAL);
										errMsg.SaveErrorMsg(dSetPinLevel[1], dPinLevel[nVILID], dLevelResolution, VALUE_DOUBLE_FOUR_DECIMAL);
									}
									if (fabs(dSetPinLevel[2] - dPinLevel[nVOHID]) > dLevelResolution)
									{
										errMsg.SetErrorItem("VOH", nullptr, 1, bySlotNo, usChannel, VALUE_NOT_EQUAL);
										errMsg.SaveErrorMsg(dSetPinLevel[2], dPinLevel[nVOHID], dLevelResolution, VALUE_DOUBLE_FOUR_DECIMAL);
									}
									if (fabs(dSetPinLevel[3] - dPinLevel[nVOLID]) > dLevelResolution)
									{
										errMsg.SetErrorItem("VOL", nullptr, 1, bySlotNo, usChannel, VALUE_NOT_EQUAL);
										errMsg.SaveErrorMsg(dSetPinLevel[3], dPinLevel[nVOLID], dLevelResolution, VALUE_DOUBLE_FOUR_DECIMAL);
									}
								}
							}
							++iterSlot;
						}
					}
					else
					{
						if (PIN_LEVEL_ERROR == nRetVal)
						{
							if (-1.5 + dLevelResolution > dPinLevel[nVIHID] || 6 - dLevelResolution < dPinLevel[nVIHID] ||
								-1.5 + dLevelResolution > dPinLevel[nVILID] || 6 - dLevelResolution < dPinLevel[nVILID] ||
								-1.5 + dLevelResolution > dPinLevel[nVOHID] || 6 - dLevelResolution < dPinLevel[nVOHID] ||
								-1.5 + dLevelResolution > dPinLevel[nVOLID] || 6 - dLevelResolution < dPinLevel[nVOLID] ||
								dPinLevel[nVIHID] - dLevelResolution < dPinLevel[nVILID] || dPinLevel[nVOHID] - dLevelResolution < dPinLevel[nVOLID])
							{
								continue;
							}
							else
							{
								///<Return value is not right
								XT_EXPECT_TRUE(FALSE);
								errMsg.AddNewError(STRING_ERROR_MSG, "G_ALLPIN", 2);
								char cMsg[256] = { 0 };
								sts_sprintf(cMsg, 256, "VIH:%.1f, VIL:%.2f, VOH:%.2f, VOL:%.2f", dPinLevel[nVIHID], dPinLevel[nVILID], dPinLevel[nVOHID], dPinLevel[nVOLID]);
								errMsg.SetErrorItem("PinLevel", cMsg);
								errMsg.SaveErrorMsg("Warning when pin level is not over scale, the return value is %d!", nRetVal);
							}
						}
						else
						{
							///<Return Value is not right
							XT_EXPECT_TRUE(FALSE);
							errMsg.AddNewError(STRING_ERROR_MSG, "G_ALLPIN", 2);
							errMsg.SaveErrorMsg("Unknown error, the return value is %d!", nRetVal);
						}
					}
				}
			}
		}
	}
	errMsg.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();//Unload the vector file.
}