#pragma once
/*!
* @file      TestDCMI2CSetParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/10
* @version   v 1.0.0.0
* @brief     测试I2CSet参数有效性
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMI2CSetParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	const int nTestSiteCount = 5;
	int nTestSite[nTestSiteCount] = { -1,0,8,32,DCM_MAX_SITE_COUNT };
	int nRetVal = 0;


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

	char lpszChannel[32] = { 0 };

	//Check the board not inserted.
	nRetVal = dcm.I2CSet(1000, 3, DCM_REG8, "S40_1,S40_3,S40_5", "S40_2,S40_4,S40_6");
	XT_EXPECT_EQ(nRetVal, -4);
	if (-4 != nRetVal)
	{
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No warning when board is not inserted!");
	}

	const int nTestChannelCount = 14;
	int nRightChannelCount = 4;
	string strSCLChannel[nTestChannelCount];
	string strSDAChannel[nTestChannelCount];
	int nChannelSiteCount[nTestChannelCount] = { 1,2,16,32,32,32,32,32, 32 };
	int nChannelSeparator = mapSlot.size() * DCM_MAX_CHANNELS_PER_BOARD / 32;
	int nSCLChannelType[nTestChannelCount] = { 0, 0, 0, 0, -3, 0, -4, 0, -5, 0, -6, 0, -8, 0 };//0 is right, -2 is nullptr, -3 is not inserted,-4 is channel invalid, -5 is format error, -6 is duplicated
	int nSDAChannelType[nTestChannelCount] = { 0, 0, 0, 0, 0, -3, 0, -4, 0, -5, 0, -6, 0, -8 };

	USHORT uErrorSite = 3;

	USHORT usChannel = 0;
	auto iterSlot = mapSlot.begin();
	for (int nIndex = 0; nIndex < nRightChannelCount; ++nIndex)
	{
		strSCLChannel[nIndex].clear();
		strSDAChannel[nIndex].clear();
		usChannel = 0;
		USHORT uSiteCount = 0;
		iterSlot = mapSlot.begin();
		BOOL bAddInvalidChannel = FALSE;
		USHORT uCurChannel = 0;
		while (mapSlot.end() != iterSlot)
		{
			usChannel = 0;
			while (DCM_MAX_CHANNELS_PER_BOARD > usChannel)
			{
				if (uSiteCount >= nChannelSiteCount[nIndex])
				{
					break;
				}
				sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d,", iterSlot->first, usChannel);
				strSCLChannel[nIndex] += lpszChannel;
				sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d,", iterSlot->first, usChannel + nChannelSeparator / 2);
				strSDAChannel[nIndex] += lpszChannel;
				++uSiteCount;
				usChannel += nChannelSeparator;
			}
			if (uSiteCount >= nChannelSiteCount[nIndex])
			{
				break;
			}
			++iterSlot;
		}

		strSCLChannel[nIndex].erase(strSCLChannel[nIndex].size() - 1);
		strSDAChannel[nIndex].erase(strSDAChannel[nIndex].size() - 1);
	}
	int nCurChannelType = nRightChannelCount;

	const char* lpszRightSCL = strSCLChannel[nRightChannelCount - 1].c_str();
	const char* lpszRightSDA = strSDAChannel[nRightChannelCount - 1].c_str();

	//SCL nullptr
	strSCLChannel[nCurChannelType].clear();
	strSDAChannel[nCurChannelType++] = lpszRightSDA;
	//SDA nullptr
	strSCLChannel[nCurChannelType] = lpszRightSCL;
	strSDAChannel[nCurChannelType++].clear();


	BYTE byNotInsertSlot = 1;
	while (mapSlot.end() != mapSlot.find(byNotInsertSlot))
	{
		++byNotInsertSlot;
	}

	DWORD dwSiteStatus = STSGetsSiteStatus();

	//SCL or SDA has not inserted board
	USHORT uCurChannel = 0;
	USHORT usSiteIndex = 0;
	BYTE bySlotNo = 0;
	iterSlot = mapSlot.begin();
	while (mapSlot.end() != iterSlot)
	{
		usChannel = 0;
		while (DCM_MAX_CHANNELS_PER_BOARD > usChannel)
		{
			if (32 <= usSiteIndex)
			{
				break;
			}
			if (uErrorSite == usSiteIndex)
			{
				bySlotNo = byNotInsertSlot;
			}
			else
			{
				bySlotNo = iterSlot->first;
			}
			sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d,", bySlotNo, usChannel);
			strSCLChannel[nCurChannelType] += lpszChannel;
			sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d,", iterSlot->first, usChannel + nChannelSeparator / 2);
			strSDAChannel[nCurChannelType] += lpszChannel;


			sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d,", iterSlot->first, usChannel);
			strSCLChannel[nCurChannelType + 1] += lpszChannel;
			sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d,", bySlotNo, usChannel + nChannelSeparator / 2);
			strSDAChannel[nCurChannelType + 1] += lpszChannel;

			++usSiteIndex;
			usChannel += nChannelSeparator;
		}
		++iterSlot;
	}
	strSCLChannel[nCurChannelType].erase(strSCLChannel[nCurChannelType].size() - 1);
	strSDAChannel[nCurChannelType].erase(strSDAChannel[nCurChannelType].size() - 1);

	strSCLChannel[nCurChannelType + 1].erase(strSCLChannel[nCurChannelType + 1].size() - 1);
	strSDAChannel[nCurChannelType + 1].erase(strSDAChannel[nCurChannelType + 1].size() - 1);

	nCurChannelType += 2;

	//SCL or SDA has invalid channel
	usSiteIndex = 0;
	iterSlot = mapSlot.begin();
	while (mapSlot.end() != iterSlot)
	{
		usChannel = 0;
		while (DCM_MAX_CHANNELS_PER_BOARD > usChannel)
		{
			if (32 <= usSiteIndex)
			{
				break;
			}
			if (uErrorSite == usSiteIndex)
			{
				uCurChannel = DCM_MAX_CHANNELS_PER_BOARD + 1;
			}
			else
			{
				uCurChannel = usChannel;
			}
			sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d,", iterSlot->first, uCurChannel);
			strSCLChannel[nCurChannelType] += lpszChannel;
			sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d,", iterSlot->first, usChannel + nChannelSeparator / 2);
			strSDAChannel[nCurChannelType] += lpszChannel;


			sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d,", iterSlot->first, usChannel);
			strSCLChannel[nCurChannelType + 1] += lpszChannel;
			sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d,", iterSlot->first, uCurChannel);
			strSDAChannel[nCurChannelType + 1] += lpszChannel;

			++usSiteIndex;
			usChannel += nChannelSeparator;
		}
		++iterSlot;
	}
	strSCLChannel[nCurChannelType].erase(strSCLChannel[nCurChannelType].size() - 1);
	strSDAChannel[nCurChannelType].erase(strSDAChannel[nCurChannelType].size() - 1);
	strSCLChannel[nCurChannelType + 1].erase(strSCLChannel[nCurChannelType + 1].size() - 1);
	strSDAChannel[nCurChannelType + 1].erase(strSDAChannel[nCurChannelType + 1].size() - 1);
	nCurChannelType += 2;

	//Format error
	usSiteIndex = 0;
	while (mapSlot.end() != iterSlot)
	{
		usChannel = 0;
		while (DCM_MAX_CHANNELS_PER_BOARD > usChannel)
		{
			if (32 <= usSiteIndex)
			{
				break;
			}
			if (uErrorSite == usSiteIndex)
			{
				strcpy_s(lpszChannel, sizeof(lpszChannel), "SS_1");
			}
			else
			{
				sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d,", iterSlot->first, usChannel);
			}
			strSCLChannel[nCurChannelType] += lpszChannel;
			sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d,", iterSlot->first, usChannel + nChannelSeparator / 2);
			strSDAChannel[nCurChannelType] += lpszChannel;


			sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d,", iterSlot->first, usChannel);
			strSCLChannel[nCurChannelType + 1] += lpszChannel;

			if (3 == usSiteIndex)
			{
				strcpy_s(lpszChannel, sizeof(lpszChannel), "SS_1");
			}
			else
			{
				sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d,", iterSlot->first, usChannel + nChannelSeparator / 2);
			}
			sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_%d,", iterSlot->first, usChannel + nChannelSeparator / 2);
			strSDAChannel[nCurChannelType + 1] += lpszChannel;

			++usSiteIndex;
			usChannel += nChannelSeparator;
		}
		++iterSlot;
	}
	nCurChannelType += 2;

	int nMaxSiteCount = 32;

	char* lpszSCLChannel = nullptr;
	char* lpszSDAChannel = nullptr;
	int nCurSite = 0;
	for (int nTestChanneIndex = 0; nTestChanneIndex < nTestChannelCount; ++nTestChanneIndex)
	{
		lpszSCLChannel = (char*)strSCLChannel[nTestChanneIndex].c_str();
		lpszSDAChannel = (char*)strSDAChannel[nTestChanneIndex].c_str();
		if (0 == strlen(lpszSCLChannel))
		{
			lpszSCLChannel = nullptr;
		}
		if (0 == strlen(lpszSDAChannel))
		{
			lpszSDAChannel = nullptr;
		}
		for (int nSiteTestIndex = 0; nSiteTestIndex < nTestSiteCount; ++nSiteTestIndex)
		{
			nCurSite = nTestSite[nSiteTestIndex];
			for (I2C_REG_ADDR_MODE REGMode = DCM_REG8; REGMode <= DCM_REG32 + 1; REGMode = (I2C_REG_ADDR_MODE)(REGMode + 1))
			{
				nRetVal = dcm.I2CSet(MIN_PERIOD, nCurSite, REGMode, lpszSCLChannel, lpszSDAChannel);
				if (0 >= nCurSite || DCM_MAX_SITE_COUNT / 2 < nCurSite)
				{
					XT_EXPECT_EQ(nRetVal, -1);
					if (-1 == nRetVal)
					{
						continue;
					}
					errMsg.AddNewError(STRING_ERROR_MSG);
					char cDigit[10] = { 0 };
					_itoa_s(nTestSite[nSiteTestIndex], cDigit, 10, 10);
					errMsg.SetErrorItem("SiteNum", cDigit);
					errMsg.SaveErrorMsg("Warning when site is error, the return value is %d/%d!", nRetVal, -1);
				}
				else if (DCM_REG32 < REGMode)
				{
					XT_EXPECT_EQ(nRetVal, -2);
					if (-2 == nRetVal)
					{
						continue;
					}
					errMsg.AddNewError(STRING_ERROR_MSG);
					char cDigit[10] = { 0 };
					_itoa_s(nTestSite[nSiteTestIndex], cDigit, 10, 10);
					errMsg.SetErrorItem("Check register mode", cDigit);
					errMsg.SaveErrorMsg("Warning when register mode is error, the return value is %d/%d!", nRetVal, -2);
				}
				else if (nullptr == lpszSDAChannel || nullptr == lpszSCLChannel)
				{
					XT_EXPECT_EQ(nRetVal, -3);
					if (-3 == nRetVal)
					{
						continue;
					}

					errMsg.AddNewError(STRING_ERROR_MSG);
					char cDigit[10] = { 0 };
					_itoa_s(nCurSite, cDigit, 10, 10);
					char cMsg[1024] = { 0 };
					errMsg.SetErrorItem("SCL/SDA", cMsg);
					errMsg.SaveErrorMsg("Warning when the pin is nullptr(lpszSCLChannel(%s), lpszSDAChannel(%s)), the return value is %d/%d!", lpszSCLChannel, lpszSDAChannel, nRetVal, -3);
				}
				else if (-4 == nSCLChannelType[nTestChanneIndex] || -4 == nSDAChannelType[nTestChanneIndex])
				{
					XT_EXPECT_EQ(nRetVal, -4);
					if (-4 != nRetVal)
					{
						errMsg.AddNewError(STRING_ERROR_MSG);
						errMsg.SetErrorItem("Board of valid site not inserted");
						errMsg.SaveErrorMsg("The board of valid site is not inserted (lpszSCLChannel(%s), lpszSDAChannel(%s)), but the return value is %d/%d!", lpszSCLChannel, lpszSDAChannel, nRetVal, -4);
						continue;
					}


					DWORD dwSiteStatus = STSGetsSiteStatus();
					DWORD dwSetSiteStatus = dwSiteStatus;
					dwSetSiteStatus &= ~(1 << uErrorSite);
					StsSetSiteStatus(dwSetSiteStatus);
					nRetVal = dcm.I2CSet(MIN_PERIOD, nChannelSiteCount[nTestChanneIndex], REGMode, lpszSCLChannel, lpszSDAChannel);

					XT_EXPECT_EQ(nRetVal, 0);
					if (0 != nRetVal)
					{
						errMsg.AddNewError(STRING_ERROR_MSG);
						errMsg.SetErrorItem("Board of invalid site not inserted");
						errMsg.SaveErrorMsg("The board of invalid site is not inserted (lpszSCLChannel(%s), lpszSDAChannel(%s)), but the return value is %d/%d!", lpszSCLChannel, lpszSDAChannel, nRetVal, 0);
					}
					StsSetSitesStatus(dwSiteStatus);
					dcm_I2CDeleteMemory();
				}
				else if (-5 == nSCLChannelType[nTestChanneIndex] || -5 == nSDAChannelType[nTestChanneIndex])
				{
					XT_EXPECT_EQ(nRetVal, -5);
					if (-5 == nRetVal)
					{
						continue;
					}

					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SetErrorItem("Check channel validity");
					errMsg.SaveErrorMsg("The channel is invalid(lpszSCLChannel(%s), lpszSDAChannel(%s)) , but the return value is %d/%d!", lpszSCLChannel, lpszSDAChannel, nRetVal, -5);

				}
				else if (-6 == nSCLChannelType[nTestChanneIndex] || -6 == nSDAChannelType[nTestChanneIndex])
				{
					XT_EXPECT_EQ(nRetVal, -6);
					if (-6 == nRetVal)
					{
						continue;
					}
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SetErrorItem("Check parameter format");
					errMsg.SaveErrorMsg("The channel is invalid(lpszSCLChannel(%s), lpszSDAChannel(%s)) , but the return value is %d/%d!", lpszSCLChannel, lpszSDAChannel, nRetVal, -6);
				}
				else if (nCurSite != nChannelSiteCount[nTestChanneIndex])
				{
					XT_EXPECT_EQ(nRetVal, -7);
					if (-7 == nRetVal)
					{
						continue;

					}
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SetErrorItem("Check the SCL/SDA site");
					errMsg.SaveErrorMsg("The channel site(lpszSCLChannel(%s), lpszSDAChannel(%s))is not equal to the site number(%d), but the return value is %d/%d!",
						lpszSCLChannel, lpszSDAChannel, nCurSite, nRetVal, -7);
				}
				else if (-8 == nSCLChannelType[nTestChanneIndex] || -8 == nSDAChannelType[nTestChanneIndex])
				{
					XT_EXPECT_EQ(nRetVal, -8);
					if (-8 == nRetVal)
					{
						continue;
					}
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SetErrorItem("Check the channel conflict of SCL/SDA");
					errMsg.SaveErrorMsg("The channel is conflict(lpszSCLChannel(%s), but the return value is %d/%d!",
						lpszSCLChannel, lpszSDAChannel, nRetVal, -8);
				}
				else
				{
					XT_EXPECT_EQ(nRetVal, 0);
					if (0 == nRetVal)
					{
						continue;
					}
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SetErrorItem("Unknown error");
					errMsg.SaveErrorMsg("Unknown error(lpszSCLChannel(%s), lpszSDAChannel(%s)), but the return value is %d/%d!",
						lpszSCLChannel, lpszSDAChannel, nRetVal, 0);
				}
			}
		}
	}
	errMsg.Print(this, g_lpszReportFilePath);
	dcm_CloseFile();//Unload the vector file.
	dcm_I2CDeleteMemory();
}