#include "pch.h"
#include "SelfCheck.h"

#define	SELFCHECK_FILE_DASH_NUM	80
CSelfCheck::CSelfCheck(BYTE bySlotNo)
	: m_bySlotNo(bySlotNo)
{

}

CSelfCheck::~CSelfCheck()
{
	for (auto& Controller : m_mapController)
	{
		if (nullptr != Controller.second)
		{
			delete Controller.second;
			Controller.second = nullptr;
		}
	}
}

int CSelfCheck::Check(const char* lpszLogFileName, int& nChannelResult)
{
	if (nullptr != lpszLogFileName)
	{
		m_strLogFile = lpszLogFileName;
	}
	nChannelResult = 0;

	for (BYTE byController = 0; byController < DCM400_MAX_CONTROLLERS_PRE_BOARD;++byController)
	{
		CHardwareFunction* pHardware = new CHardwareFunction(m_bySlotNo, nullptr);
		pHardware->SetControllerIndex(byController);
		m_mapController.insert(make_pair(byController, pHardware));
	}
	SaveBoardInfo();
	return TRUE;
}

void CSelfCheck::SaveBoardInfo()
{
	if (0 == m_strLogFile.size())
	{
		return;
	}
	//显示--模块相关信息
	FILE* pFileLog = nullptr;
	fopen_s(&pFileLog, m_strLogFile.c_str(), "a+");
	for (int nIndex = 0; nIndex < SELFCHECK_FILE_DASH_NUM; ++nIndex)
	{
		fprintf_s(pFileLog, "-");
	}
	fprintf_s(pFileLog, "\n");

// 	STS_HARDINFO HardInfo;
// 	m_pDCM->GetHardInfo(m_bySlotNo, &HardInfo, 1);

	///<Board Name
	fprintf_s(pFileLog, "  Module Type : DCM400 (SM8250) Self Check\n");
	///<The slot number the board inserted
	fprintf_s(pFileLog, "  Slot No.    : #%02d\n", m_bySlotNo);
	///<The version of the self checking
	fprintf_s(pFileLog, "  Version     : 1.00\n");
	///<The serial number
	fprintf_s(pFileLog, "  Serial No.  : %s\n", "N/A");
	///<The hardware revision
	fprintf_s(pFileLog, "  Hard Rev    : %s\n", "N/A");
	///<The FPGA revision

	string strLog;
	char lpszLog[128] = { 0 };
	auto iterContrller = m_mapController.begin();
	sprintf_s(lpszLog, sizeof(lpszLog), "  FPGA Rev    : SM8250->0x%04x", iterContrller->second->GetBoardLogicRevision());
	strLog += lpszLog;
	BYTE byControllerIndex = 0;
	for (auto& Controller : m_mapController)
	{
		if (0 == Controller.first)
		{
			continue;
		}
		sprintf_s(lpszLog, sizeof(lpszLog), "   Sub FPGA %d->0x%04x", Controller.first - 1, Controller.second->GetControllerLogicRevision());
		strLog += lpszLog;
	}
	strLog += "\n";
	fprintf_s(pFileLog, strLog.c_str());

	for (int nIndex = 0; nIndex < SELFCHECK_FILE_DASH_NUM; ++nIndex)
	{
		fprintf_s(pFileLog, "-");
	}
	fprintf_s(pFileLog, "\n");
	fclose(pFileLog);
}
