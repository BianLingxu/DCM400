#include "DiagnosisConnectivity.h"
#include "IHDReportDevice.h"
#include "..\HDModule.h"
#include "..\Pattern.h"
#include <iterator>
using namespace std;

CDiagnosisConnectivity::CDiagnosisConnectivity()
{
}

CDiagnosisConnectivity::~CDiagnosisConnectivity()
{}

int CDiagnosisConnectivity::QueryInstance(const char * name, void ** ptr)
{
    return -1;
}

void CDiagnosisConnectivity::Release()
{}

const char * CDiagnosisConnectivity::Name() const
{
    return "Connectivity Diagnosis";
}

int CDiagnosisConnectivity::GetChildren(STSVector<IHDDoctorItem *> & children) const
{
    return 0;
}

int CDiagnosisConnectivity::Doctor(IHDReportDevice* pReportDevice)
{
	StartTimer();
	m_pReportDevice = pReportDevice;
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;
	auto iterController = m_mapUndiagnosableController.begin();
	while (m_mapUndiagnosableController.end() != iterController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(iterController->first, byBoardControllerIndex);
		pReportDevice->PrintfToUi(IHDReportDevice::Fail);
		pReportDevice->PrintfToUi("\t Slot %d, Controller %d\n", bySlotNo, byBoardControllerIndex);

		bySlotNo = HDModule::Instance()->ID2Board(iterController->second, byBoardControllerIndex);
		pReportDevice->PrintfToUi("\t\t Undiagnosable for slot %d controller %d is not existed.", bySlotNo, byBoardControllerIndex);
		++iterController;
	}

	const char* lpszBaseIndent = IndentFormat();
	string strNextIndent = IndentFormat() + IndentChar();
	const char* lpszNextIndent = strNextIndent.c_str();

	pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<ConnectivityTest>\n", lpszBaseIndent);
	int nFailCount = 0;
	for (BYTE byConnectIndex = 0; byConnectIndex < 2; ++byConnectIndex)
	{
		int nRetVal = CheckConnect(lpszNextIndent, 0 == byConnectIndex ? TRUE : FALSE);
		if (0 != nRetVal)
		{
			++nFailCount;
		}
	}
	int nRetVal = -1;
	if (0 == nFailCount)
	{
		nRetVal = 0;
		pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextIndent);
	}
	else
	{
		pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszNextIndent);
	}
	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextIndent, dTimeConsume, lpszTimeUnits);
	pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</ConnectivityTest>\n", lpszBaseIndent);
	return nRetVal;
}

bool CDiagnosisConnectivity::IsUserCheck() const
{
	switch (m_UserRole)
	{
	case CDiagnosisItem::PROCUCTION:
		return true;
		break;
	case CDiagnosisItem::DEVELOPER:
		return true;
		break;
	case CDiagnosisItem::USER:
		return false;
		break;
	default:
		return false;
		break;
	}
	return false;
}

void CDiagnosisConnectivity::CheckDiagnosable()
{
	if (0 == m_vecEnableController.size() && 0 == m_mapUndiagnosableController.size())
	{
		return;
	}

	set<UINT> setController;
	for (auto uControllerID : m_vecEnableController)
	{
		setController.insert(uControllerID);
	}
	m_vecEnableController.clear();

	for (auto& Undiagnosable : m_mapUndiagnosableController)
	{
		setController.insert(Undiagnosable.first);
	}
	m_mapUndiagnosableController.clear();


	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;
	BYTE byRelatedController = 0;
	UINT uRelatedControllerID = 0;
	for (auto uControllerID : setController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);
		if (0 == byBoardControllerIndex % 2)
		{
			byRelatedController = byBoardControllerIndex + 1;
		}
		else
		{
			byRelatedController = byBoardControllerIndex - 1;
		}
		uRelatedControllerID = HDModule::Instance()->GetID(bySlotNo, byRelatedController);
		if (setController.end() == setController.find(uRelatedControllerID))
		{
			m_mapUndiagnosableController.insert(make_pair(uRelatedControllerID, uControllerID));
		}
	}

	for (auto& undiagnosable : m_mapUndiagnosableController)
	{
		auto iterController = setController.find(undiagnosable.first);
		if (setController.end() != iterController)
		{
			setController.erase(iterController);
		}
	}

	copy(setController.begin(), setController.end(), back_inserter(m_vecEnableController));
}

int CDiagnosisConnectivity::CheckConnect(const char* lpszBaseIndent, BOOL bConnect)
{
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	const char lpszConnectStatus[2][12] = { "Connect", "Disconnect" };
	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();

	const char* lpszConnectName = lpszConnectStatus[bConnect ? 0 : 1];
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<%s>\n", lpszBaseIndent, lpszConnectName);


	m_pReportDevice->PrintfToUi(" %s\n", lpszConnectStatus[bConnect ? 0 : 1]);

	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < HDModule::ChannelCountPerControl; ++usChannel)
	{
		vecChannel.push_back(usChannel);
	}

	Bind(m_vecEnableController, m_vecEnableController[0]);
	CHardwareFunction* pHardware = GetHardware(m_vecEnableController[0]);
	pHardware->SetPinLevel(vecChannel, 3, 0, 1.5, 1.5, 0.8);

	///<Set relay status
	vecChannel.clear();
	for (USHORT usChannel = 0; usChannel < HDModule::ChannelCountPerBoard; ++usChannel)
	{
		vecChannel.push_back(usChannel);
	}

	pHardware->SetFunctionRelay(vecChannel, bConnect);
	ClearBind();

	const BYTE byStatusCount = 2;
	CHANNEL_OUTPUT_STATUS ChannelStatus[byStatusCount] = { CHANNEL_OUTPUT_STATUS::LOW, CHANNEL_OUTPUT_STATUS::HIGH };
	int nRetVal = 0;
	BOOL bAllPass = TRUE;
	for (int nStatusIndex = 0; nStatusIndex < byStatusCount; ++nStatusIndex)
	{
		nRetVal = CheckChannelStatus(lpszFirstIndent, bConnect, ChannelStatus[nStatusIndex]);
		if (0 != nRetVal)
		{
			bAllPass = FALSE;
		}

	}
	if (bAllPass)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</%s>\n", lpszBaseIndent, lpszConnectName);

	ShowUIResult();

	return nRetVal;
}

int CDiagnosisConnectivity::CheckChannelStatus(const char* lpszBaseIndent, BOOL bConnect, CHANNEL_OUTPUT_STATUS ChannelStatus)
{
	BYTE byCurStatusIndex = 0;
	switch (ChannelStatus)
	{
	case CHANNEL_OUTPUT_STATUS::LOW:
		byCurStatusIndex = 0;
		break;
	case CHANNEL_OUTPUT_STATUS::HIGH:
		byCurStatusIndex = 1;
		break;
	case CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE:
		return -2;
		break;
	default:
		break;
	}
	StartTimer();

	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	string strFirstIndent = lpszBaseIndent + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	ULONG ulExpectData[2][2] = { {0x00000000, 0xFFFFFFFF}, {0x00000000,0x0000000} };
	const char lpszChannelStatus[2][8] = { "LOW",  "HIGH" };
	BYTE byConnectIndex = bConnect ? 0 : 1;
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<%s>\n", lpszBaseIndent, lpszChannelStatus[byCurStatusIndex]);
	USHORT usControllerCount = m_vecEnableController.size();
	BOOL bAllPass = TRUE;

	vector<USHORT> vecChannel;
	for (USHORT usChanel = 0; usChanel < HDModule::ChannelCountPerControl;++usChanel)
	{
		vecChannel.push_back(usChanel);
	}

	const BYTE byItemCount = 2;

	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;

	for (BYTE byItemIndex = 0; byItemIndex < byItemCount; ++byItemIndex)
	{
		///<Set channel status to high impedance
		Bind(m_vecEnableController, m_vecEnableController[0]);
		CHardwareFunction* pHardware = GetHardware(m_vecEnableController[0]);
		pHardware->SetChannelStatus(vecChannel, CHANNEL_OUTPUT_STATUS::HIGH_IMPEDANCE);
		ClearBind();

		vector<UINT> vecDriverController;
		vector<UINT> vecCompareController;
		
		for (auto uControllerID : m_vecEnableController)
		{
			if (byItemIndex == uControllerID % 2)
			{
				vecDriverController.push_back(uControllerID);
			}
			else
			{
				vecCompareController.push_back(uControllerID);
			}

		}

		///<Set channel to target status
		UINT uBindControllerID = vecDriverController[0];
		pHardware = GetHardware(uBindControllerID);
		Bind(vecDriverController, uBindControllerID);
		pHardware->SetChannelStatus(vecChannel, ChannelStatus);
		ClearBind();
	
		///<Ensure the channel status is stable
		pHardware->DelayMs(1);

		for (auto uCurControllerID : vecCompareController)
		{
			StartTimer();

			pHardware = GetHardware(uCurControllerID);

			ULONG ulChannelStatus = pHardware->GetChannelStatus();

			bySlotNo = HDModule::Instance()->ID2Board(uCurControllerID, byBoardControllerIndex);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d', slot value = '%d'>\n", lpszFirstIndent, byBoardControllerIndex, bySlotNo);


			if (ulExpectData[byConnectIndex][byCurStatusIndex] != ulChannelStatus)
			{
				bAllPass = FALSE;
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' value='expect=0x%08X real=0x%08X'/>\n",
					lpszSecondIndent, ulExpectData[byConnectIndex][byCurStatusIndex], ulChannelStatus);
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
				SaveFailController(uCurControllerID);
			}
			else
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
			}


			dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</controller>\n", lpszFirstIndent);
		}
		if (CHANNEL_OUTPUT_STATUS::LOW != ChannelStatus)
		{
			UINT uBindControllerID = vecDriverController[0];
			pHardware = GetHardware(uBindControllerID);
			Bind(vecDriverController, uBindControllerID);
			pHardware->SetChannelStatus(vecChannel, CHANNEL_OUTPUT_STATUS::LOW);
			ClearBind();
		}
	}

	int nRetVal = 0;
	if (bAllPass)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</%s>\n", lpszBaseIndent, lpszChannelStatus[byCurStatusIndex]);

	return nRetVal;
}

void CDiagnosisConnectivity::ShowUIResult()
{
	BYTE bySlotNo = 0;
	BYTE byBoardController = 0;
	for (auto ControllerID : m_vecEnableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(ControllerID, byBoardController);
		if (m_setFailController.end() != m_setFailController.find(ControllerID))
		{
			m_pReportDevice->PrintfToUi(IHDReportDevice::Fail);
		}
		else
		{
			m_pReportDevice->PrintfToUi(IHDReportDevice::Pass);
		}
		m_pReportDevice->PrintfToUi("\t Slot %d, Controller %d\n", bySlotNo, byBoardController);
	}
	m_setFailController.clear();
}

inline void CDiagnosisConnectivity::SaveFailController(UINT uControllerID)
{
	if (m_setFailController.end() == m_setFailController.find(uControllerID))
	{
		m_setFailController.insert(uControllerID);
	}
}
 