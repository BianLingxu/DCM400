#pragma once
/*!
* @file      TestDCMI2CGetReadDataParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/17
* @version   v 1.0.0.0
* @brief     测试I2CGetReadData参数有效性
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMI2CGetReadDataParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	const int  nIndexTestCount = 6;
	const int nReadDataLength = 10;
	double dIndexTestValue[nIndexTestCount] = { -1, 0, 2.5, 6, 64, 65 };


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

	nRetVal = dcm.I2CGetReadData(0, 0);
	XT_EXPECT_EQ(nRetVal, -4);
	if (-4 != nRetVal)
	{
		//Channel is not set.
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No warning when channel is not set!");
	}
	string strSCLChannel;
	string strSDAChannel;
	vector<CHANNEL_INFO> vecSDAChannel;
	vector<CHANNEL_INFO> vecSCLChannel;

	GetI2CChannel(mapSlot, strSCLChannel, strSDAChannel, vecSCLChannel, vecSDAChannel);
	USHORT uSiteCount = vecSDAChannel.size();
	//Load vector.
	dcm.I2CSet(500, uSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	dcm.I2CGetReadData(0, 0);
	XT_EXPECT_EQ(nRetVal, -4);
	if (-4 != nRetVal)
	{
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No operation before, the return value(%d) is not equal to %d!", nRetVal, -4);
	}
	dcm.I2CReadData(0, 0, nReadDataLength);

	//Check the pin level.
	USHORT uTestSiteCount = uSiteCount + 1;
	for (int nSiteIndex = 0; nSiteIndex < uTestSiteCount; ++nSiteIndex)
	{
		for (int  nIndexTestIndex = 0;  nIndexTestIndex <  nIndexTestCount; ++ nIndexTestIndex)
		{
			for (int nSiteValid = 0; nSiteValid < 2; ++nSiteValid)
			{
				if (0 == nSiteValid)
				{
					//Set current site invalid.
					DWORD dwSiteStatus = 0xFFFFFFFF;
					dwSiteStatus &= ~(1 << nSiteIndex);
					StsSetSiteStatus(dwSiteStatus);

				}
				else
				{
					UCHAR siteStatus[32] = { 0 };
					memset(siteStatus, 1, MAX_SITE * sizeof(UCHAR));
					StsSetSiteStatus(0xFFFFFFFF);
				}
				nRetVal = dcm.I2CGetReadData(nSiteIndex, dIndexTestValue[nIndexTestIndex]);
				if (uSiteCount <= nSiteIndex)
				{
					XT_EXPECT_EQ(nRetVal, -1);
					if (-1 != nRetVal)
					{
						errMsg.AddNewError(STRING_ERROR_MSG);
						char lpszDigit[16] = { 0 };
						_itoa_s(nSiteIndex, lpszDigit, sizeof(lpszDigit), 10);
						errMsg.SetErrorItem("SiteID", lpszDigit);
						errMsg.SaveErrorMsg("No warning when site id is over scale, the return value is %d!", nRetVal);
					}
				}
				else if (0 == nSiteValid)
				{
					XT_EXPECT_EQ(nRetVal, -2);
					if (-2 != nRetVal)
					{
						errMsg.AddNewError(STRING_ERROR_MSG);
						char lpszDigit[16] = { 0 };
						_itoa_s(nSiteIndex, lpszDigit, sizeof(lpszDigit), 10);
						errMsg.SetErrorItem("SiteID", lpszDigit);
						errMsg.SaveErrorMsg("No warning when site id is invalid, the return value is %d!", nRetVal);
					}
				}
				else if (nReadDataLength <= dIndexTestValue[ nIndexTestIndex] || 0 > dIndexTestValue[ nIndexTestIndex] || 64 <= dIndexTestValue[ nIndexTestIndex])
				{
					XT_EXPECT_EQ(nRetVal, -3);
					if (-3 != nRetVal)
					{
						errMsg.AddNewError(STRING_ERROR_MSG);
						char lpszDigit[10] = { 0 };
						_itoa_s(nSiteIndex, lpszDigit, 10, 10);
						errMsg.SetErrorItem("nDataIndex", lpszDigit);
						errMsg.SaveErrorMsg("No warning when data index out scale, the return value is %d!", nRetVal);
					}
				}
				else
				{
					if (0 > nRetVal)
					{
						//Return value is not right.
						errMsg.AddNewError(STRING_ERROR_MSG);
						errMsg.SaveErrorMsg("Unknown error, the return value is %d!", nRetVal);
					}
				}
			}
		}
	}
	vecSDAChannel.clear();
	vecSCLChannel.clear();
	errMsg.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();//Unload the vector file.
	dcm_I2CDeleteMemory();
}