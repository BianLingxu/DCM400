#pragma once
/*!
* @file      TestDCM100Alarm.h
*
* Copyright (C) ���������ؼ������޹�˾
*
* @author    Guangyun Wang
* @date      2017/10/09
* @version   v 1.0.0.0
* @brief     ����DCMAlarm����
* @comment
*/

XT_TEST(ParamValidityTest, TestDCMAlarm)
{
	ParamAlarm alarmInfo[MAX_SITE];
	char lpszString[30] = { 0 };
	strcpy_s(lpszString, 30, "DCMAlarm");
	char lpszPartID[2] = "0";
	char lpszParamSymbol[20] = "ALARM";
	for (int nSiteIndex = 0; nSiteIndex < MAX_SITE; ++nSiteIndex)
	{
		alarmInfo[nSiteIndex].SiteNum = nSiteIndex;
		alarmInfo[nSiteIndex].DeviceNum = lpszPartID;
		alarmInfo[nSiteIndex].TestFun = lpszString;
		alarmInfo[nSiteIndex].ParamSymbol = lpszParamSymbol;
	}

	short uSite[MAX_SITE] = { 0 };
	STSCheckSaveClearAlarm(alarmInfo, uSite, MAX_SITE);

	STSCloseParamAlarm(0);
}