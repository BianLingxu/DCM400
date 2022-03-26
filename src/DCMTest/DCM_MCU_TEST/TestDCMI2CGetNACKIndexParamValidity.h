/*!
* @file      TestDCMI2CGetNACKIndexParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/17
* @version   v 1.0.0.0
* @brief     测试I2CGetNACKIndex参数有效性
* @comment
*/
#include "..\DCMTestMain.h"

XT_TEST(ParamValidityTest, TestDCMI2CGetNACKIndexParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	double dPeriod = 0;
	int nRetVal = 0;
	const int  nIndexTestCount = 6;
	const int nReadDataLength = 10;
	BYTE bySlaveAddr = 0x00;
	BYTE byREGAddr = 0xA0;


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
	nRetVal = dcm.I2CGetNACKIndex(0);
	XT_EXPECT_EQ(nRetVal, -3);
	if (-3 != nRetVal)
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

	dcm.I2CSet(500, uSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());
	nRetVal = dcm.I2CGetNACKIndex(0);
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
		for (int nSiteValid = 0; nSiteValid < 2; ++nSiteValid)
		{
			if (0 == nSiteValid)
			{
				///<Set current site invalid
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
			nRetVal = dcm.I2CGetNACKIndex(nSiteIndex);
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
			else
			{
				XT_EXPECT_GREATEREQ(nRetVal, 0);
				if (0 > nRetVal)
				{
					///<Return value is not right
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("Unknown error, the return value is %d!", nRetVal);
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