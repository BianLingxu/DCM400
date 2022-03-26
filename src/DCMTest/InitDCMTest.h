/*!
* @file      InitDCMTest.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/09/25
* @version   v 1.0.0.0
* @brief     初始化DCM测试用例
* @comment
*/
#include "DCMTestMain.h"
XT_TEST(InitTest, InitDCMTest)
{
	UCHAR ucSiteStatus[32] = { 0 };
	memset(ucSiteStatus, 1, sizeof(ucSiteStatus));
	AstEnableSite(ucSiteStatus);

	char lpszDirectory[256] = { 0 };
	GetModuleFileName(nullptr, lpszDirectory, sizeof(lpszDirectory));
	int nPos = 0;
	string strDirectory = lpszDirectory;
	nPos = strDirectory.rfind("\\");
	if (0 != nPos)
	{
		strDirectory.erase(nPos + 1);
	}
	strDirectory += "AutomaticTestCase\\";
	CreateDirectory(strDirectory.c_str(), nullptr);

	SYSTEMTIME SysTime;
	GetLocalTime(&SysTime);
	sprintf_s(lpszDirectory, sizeof(lpszDirectory), "DCM_%04d%02d%02d_%02d%02d%02d.txt", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
	strDirectory += lpszDirectory;
	strcpy_s(g_lpszReportFilePath, sizeof(g_lpszReportFilePath), strDirectory.c_str());

	//Enable the alarm
	StsSetAlarmSwitch(1);
	
	char lpszFileName[255] = { 0 };
	// 创建report文件
	sts_sprintf(lpszFileName, STS_ARRAY_SIZE(lpszFileName), "TestDCMAlarm_%d%02d%02d_%02d%02d%02d", SysTime.wYear, SysTime.wMonth, SysTime.wDay, SysTime.wHour, SysTime.wMinute, SysTime.wSecond);
	STSOpenParamAlarm(lpszFileName, 0);
	
	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
	char lpszPinName[20] = { 0 };
	for (int nPinIndex = 0; nPinIndex < DCM_CHANNELS_PER_CONTROL; ++nPinIndex)
	{
		sts_sprintf(lpszPinName, 20, "CH%d", nPinIndex);
		for (int nSiteIndex = 0; nSiteIndex < MAX_SITE; ++nSiteIndex)
		{
			dcm.SetAlarmMask(lpszPinName, nSiteIndex, true);
		}
	}
	dcm_CloseFile();
}