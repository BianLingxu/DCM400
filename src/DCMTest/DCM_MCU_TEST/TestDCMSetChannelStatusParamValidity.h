#pragma once
/**
 * @file TestDCMSetChannelStatusParamValidity
 * @brief Check the parameter validity of SetChannelStatus
 * @author Guangyun Wang
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#include "MCUCase.h"
XT_TEST(ParamValidityTest, TestDCMSetChannelStatusParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	map<BYTE, USHORT> mapSlot;
	GetBoardInfo(mapSlot, g_lpszVectorFilePath, TRUE);
	BYTE byBoardCount = dcm_GetBoardInfo(nullptr, 0);
	if (0 == mapSlot.size())
	{
		///<No board is inserted
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No board inserted!");
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}
	nRetVal = dcm.SetChannelStatus("G_ALLPIN", 0, DCM_LOW);
	if (VECTOR_FILE_NOT_LOADED != nRetVal)
	{
		XT_EXPECT_EQ(VECTOR_FILE_NOT_LOADED, nRetVal);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No Warning when vector is not loaded!");
	}

	const int nPinGroupTestCount = 4;
	char* lpszTestPinGroup[nPinGroupTestCount] = { nullptr, "G_NOPIN", "G_ALLPIN", "CH0" };

	USHORT usSiteCount = dcm_GetVectorSiteCount();
	USHORT usTestSiteCount = usSiteCount + 1;
	USHORT usInvalidSiteNo = mapSlot.begin()->second + 1;
	
	DWORD dwSiteStatus = STSGetsSiteStatus();
	DWORD dwCurSiteStatus = dwSiteStatus & ~(1 << usInvalidSiteNo);
	StsSetSiteStatus(dwCurSiteStatus);

	for (auto& lpszPinGroup : lpszTestPinGroup)
	{
		for (USHORT usSiteNo = 0; usSiteNo < usTestSiteCount;++usSiteNo)
		{
			nRetVal = dcm.SetChannelStatus(lpszPinGroup, usSiteNo, DCM_HIGH);
			int nStringType = dcm_GetStringType(lpszPinGroup);
			if (2 < nStringType)
			{
				XT_EXPECT_EQ(nRetVal, PIN_GROUP_ERROR);
				if (PIN_GROUP_ERROR != nRetVal)
				{
					if (nullptr == lpszPinGroup)
					{
						errMsg.SaveErrorMsg("The pin group is nullptr, but the return value(%d) is not Equal to PIN_GROUP_ERROR(%d)", nRetVal, PIN_GROUP_ERROR);
					}
					else
					{
						errMsg.SaveErrorMsg("The pin group(%s) is not defined, but the return value(%d) is not Equal to PIN_GROUP_ERROR(%d)", lpszPinGroup, nRetVal, PIN_GROUP_ERROR);
					}
				}
			}
			else if (usSiteCount > usSiteNo)
			{
				XT_EXPECT_EQ(nRetVal, SITE_ERROR);
				if (SITE_ERROR != nRetVal)
				{
					errMsg.SaveErrorMsg("The site number(%d) is over range, but the return value(%d) is not Equal to SITE_ERROR(%d)", usSiteNo, nRetVal, SITE_ERROR);
				}
			}
			else if (usInvalidSiteNo == usSiteNo)
			{
				XT_EXPECT_EQ(nRetVal, SITE_INVALID);
				if (SITE_INVALID != nRetVal)
				{
					errMsg.SaveErrorMsg("The site (SITE_%d) is invalid, but the return value(%d) is not Equal to SITE_INVALID(%d)", usSiteNo + 1, nRetVal, SITE_INVALID);
				}
			}
			else
			{
				XT_EXPECT_EQ(nRetVal, 0);
				if (0 != nRetVal)
				{
					errMsg.SaveErrorMsg("The return(%d) is not equal to %d", 0);
				}
			}
		}
	}
	StsSetSiteStatus(dwSiteStatus);
	errMsg.Print(this, g_lpszReportFilePath);
	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
}