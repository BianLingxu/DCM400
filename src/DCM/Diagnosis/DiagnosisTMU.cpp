#include "DiagnosisTMU.h"
#include "..\HDModule.h"
#include <set>
#include "IHDReportDevice.h"
#include "..\Pattern.h"
#include <iterator>
using namespace std;
/**
 * @class CChannelResult
 * @brief The channel result
*/
class CChannelResult
{
public:
	/**
	 * @brief Save the channel result
	 * @param[in] usChannel The channel number
	 * @param[in] byUnit The unit index
	 * @param[in] dPeriod The period measured
	 * @param[in] dDuty The duty measured
	 * @param[in] dTime The time consume
	 * @return Execute result
	 * - 0 Save channel result successfully
	 * - -1 The channel number is over range
	 * - -2 The unit is over range
	*/
	int SaveChannelResult(USHORT usChannel, BYTE byUnit, double dPeriod, double dDuty, double dTime);
	/**
	 * @brief Get the channel result
	 * @param[in] usChannel The channel number
	 * @param[in] byUnit The unit index
	 * @param[out] dPeriod The period
	 * @param[out] dDuty The duty
	 * @param[out] dTime The time consume
	 * @return Execute result
	 * - 0 Get the channel result successfully
	 * - -1 The channel is over range
	 * - -2 The unit is over range
	 * - -3 The channel result is not found
	*/
	int GetChannelResult(USHORT usChannel, BYTE byUnit, double& dPeriod, double& dDuty, double& dTime);
private:
	/**
	 * @struct CHANNEL_RESULT
	 * @brief The result of the channel
	*/
	struct CHANNEL_RESULT
	{
		double m_dMeasPeriod[TMU_UNIT_COUNT_PER_CONTROLLER];///<The period measured of each unit
		double m_dMeasDuty[TMU_UNIT_COUNT_PER_CONTROLLER];///<The duty measured of each unit
		double m_dTime[TMU_UNIT_COUNT_PER_CONTROLLER];///<Save the test time of each unit
		CHANNEL_RESULT()
		{
			memset(m_dMeasPeriod, 0, sizeof(m_dMeasPeriod));
			memset(m_dMeasDuty, 0, sizeof(m_dMeasDuty));
			memset(m_dTime, 0, sizeof(m_dTime));
		}
	};
	map<USHORT, CHANNEL_RESULT> m_mapResult;///<Save the test result of all channel
};

CDiagnosisTMU::CDiagnosisTMU()
{
}

CDiagnosisTMU::~CDiagnosisTMU()
{
}

int CDiagnosisTMU::QueryInstance(const char* lpszName, void** ppPoint)
{
	return -1;
}

void CDiagnosisTMU::Release()
{
}

const char* CDiagnosisTMU::Name() const
{
	return "TMU Diagnosis";
}

int CDiagnosisTMU::GetChildren(STSVector<IHDDoctorItem*>& vecChildren) const
{
	return 0;
}

bool CDiagnosisTMU::IsUserCheck() const
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

#ifdef _DEBUG
		return true;
#else
		return false;
#endif // _DEBUG
		break;
	default:
		return false;
		break;
	}
}

int CDiagnosisTMU::Doctor(IHDReportDevice* pReportDevice)
{
	m_pReportDevice = pReportDevice;
	int nRetVal = -1;

	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	const char* lpszBaseIndent = IndentFormat();

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<TMUDiagnosis>\n", lpszBaseIndent);
	std::string strNextIndent = IndentFormat() + IndentChar();
	const char* lpszNextIndent = strNextIndent.c_str();
	CheckDiagnosable();
	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;
	for (auto& Undiagnosable : m_mapUndiagnosableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(Undiagnosable.first, byBoardControllerIndex);
		m_pReportDevice->PrintfToUi(IHDReportDevice::Fail);
		m_pReportDevice->PrintfToUi("\t Slot %d, Controller %d\n", bySlotNo, byBoardControllerIndex);

		bySlotNo = HDModule::Instance()->ID2Board(Undiagnosable.second, byBoardControllerIndex);
		m_pReportDevice->PrintfToUi("\t\t Undiagnosable for slot %d controller %d is not existed.", bySlotNo, byBoardControllerIndex);

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Controller value='%d, slot value=%d'>\n", IndentFormat());
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Undiagnosable for slot %d controller %d is not existed./>\n", lpszNextIndent, bySlotNo, byBoardControllerIndex);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Controller>\n", IndentFormat());

	}

	int nFailItem = 0;
	int nItemResult = 0;

	Connect();

	if (0 == m_vecEnableController.size())
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Item>No controller valid to be diagnosed</Item>\n", lpszNextIndent);
	}
	else
	{
		do
		{
			nItemResult = PeriodModeDiagnosis(lpszNextIndent);
			if (0 != nItemResult)
			{
				++nFailItem;
			}
			if (m_pReportDevice->IsStop())
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextItem=DelayMode'/>\n", lpszNextIndent);
				break;
			}

			nItemResult = DelayModeDiagnosis(lpszNextIndent);
			if (0 != nItemResult)
			{
				++nFailItem;
			}
		} while (false);
	}
	Disconnect();

	if (0 == nFailItem)
	{
		nRetVal = 0;
		pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextIndent);
	}
	else
	{
		nRetVal = -1;
		pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszNextIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextIndent, dTimeConsume, lpszTimeUnits);

	pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</TMUDiagnosis>\n", lpszBaseIndent);

	return nRetVal;
}

void CDiagnosisTMU::CheckDiagnosable()
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

int CDiagnosisTMU::PeriodModeDiagnosis(const char* lpszBaseIndent)
{
	if (!m_nEnableStatus)
	{
		return 0;
	}

	StartTimer();
	m_pReportDevice->PrintfToUi(" PeriodMode\n");

	double dTimeConsume = 0;
	char lpszTimeUnits[8] = { 0 };

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<PeriodMode>\n", lpszBaseIndent);

	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();

	vector<double> vecPeriod;
	vecPeriod.push_back(16);
	vecPeriod.push_back(20);
	vecPeriod.push_back(50);
	vecPeriod.push_back(100);
	vecPeriod.push_back(200);
	vecPeriod.push_back(500);
	vecPeriod.push_back(1e3);
	vecPeriod.push_back(10e3);
	vecPeriod.push_back(20e3);
	vecPeriod.push_back(50e3);
	vecPeriod.push_back(100e3);
	vecPeriod.push_back(200e3);
	vecPeriod.push_back(500e3);
	vecPeriod.push_back(1e6);
	vecPeriod.push_back(10e6);
	vecPeriod.push_back(20e6);
	vecPeriod.push_back(50e6);
	vecPeriod.push_back(100e6);
	vecPeriod.push_back(200e6);
	vecPeriod.push_back(500e6);
	
	int nFailPeriodCount = 0;
	int nRetVal = 0;


	for (auto dPeriod:vecPeriod)
	{
		if (m_pReportDevice->IsStop())
		{
			double dShowPeriod = GetPeriodUnits(dPeriod, lpszTimeUnits, sizeof(lpszTimeUnits));
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextPeriod=%.1f%s'/>\n", lpszFirstIndent, dShowPeriod, lpszTimeUnits);
			break;
		}
		nRetVal = PeriodDiagnosis(lpszFirstIndent, dPeriod);
		if (0 != nRetVal)
		{
			++nFailPeriodCount;
		}
	}

	ShowUIResult();

	if (0 != nFailPeriodCount)
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='fase'/>\n", lpszFirstIndent);
	}
	else
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</PeriodMode>\n", lpszBaseIndent);
	return nRetVal;
}

int CDiagnosisTMU::DelayModeDiagnosis(const char* lpszBaseIndent)
{
	if (!m_nEnableStatus)
	{
		return 0;
	}

	StartTimer();
	m_pReportDevice->PrintfToUi(" DelayMode\n");

	double dTimeConsume = 0;
	char lpszTimeUnits[8] = { 0 };

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DelayMode>\n", lpszBaseIndent);

	string strFirstIndent = lpszBaseIndent + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();
	int nFailDelayCount = 0;
	int nRetVal = 0;

	vector<double> vecDelay;
	vecDelay.push_back(10);
	vecDelay.push_back(20);
	vecDelay.push_back(50);
	vecDelay.push_back(100);
	vecDelay.push_back(200);
	vecDelay.push_back(500);
	vecDelay.push_back(1e3);
	vecDelay.push_back(10e3);
	vecDelay.push_back(20e3);
	vecDelay.push_back(50e3);
	vecDelay.push_back(100e3);
	vecDelay.push_back(200e3);
	vecDelay.push_back(500e3);
	vecDelay.push_back(1e6);
	vecDelay.push_back(10e6);
	vecDelay.push_back(20e6);
	vecDelay.push_back(50e6);
	vecDelay.push_back(100e6);
	vecDelay.push_back(200e6);
	vecDelay.push_back(500e6);

	for (auto dDelay : vecDelay)
	{
		int nItermResult = DelayDiagnosis(lpszFirstIndent, dDelay);
		if (0 != nItermResult)
		{
			++nFailDelayCount;
		}
	}

	ShowUIResult();

	if (0 != nFailDelayCount)
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='fase'/>\n", lpszFirstIndent);
	}
	else
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DelayMode>\n", lpszBaseIndent);

	return nRetVal;
}

int CDiagnosisTMU::PeriodDiagnosis(const char* lpszBaseIndent, double dPeriod)
{
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[8] = { 0 };

	double dShowPeriod = GetPeriodUnits(dPeriod, lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Period value='%.1f%s'>\n", lpszBaseIndent, dShowPeriod, lpszTimeUnits);

	string strNextIndent = lpszBaseIndent + IndentChar();
	const char* lpszNextIndent = strNextIndent.c_str();

	vector<double> vecDuty;
	vecDuty.push_back(0.25);
	vecDuty.push_back(0.50);
	vecDuty.push_back(0.75);

	BOOL bAllPass = TRUE;

	int nRetVal = 0;
	BOOL bDutyCheck = TRUE;
	for (auto dDuty : vecDuty)
	{
		if (8 - EQUAL_ERROR > dPeriod * dDuty || 8 - EQUAL_ERROR > dPeriod * (1 - dDuty))
		{
			continue;
		}
		if (25 - EQUAL_ERROR > dPeriod)
		{
			bDutyCheck = FALSE;
		}

		if (m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='NextDuty=%.2f'/>\n", lpszNextIndent, dDuty * 100);
			break;
		}
		nRetVal = PeriodChannelDiagnosis(lpszNextIndent, dPeriod, dDuty, bDutyCheck);
		if (0 != nRetVal)
		{
			bAllPass = FALSE;
		}
	}
	if (bAllPass)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextIndent);
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszNextIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextIndent, dTimeConsume, lpszTimeUnits);
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Period>\n", lpszBaseIndent);

	return nRetVal;
}

int CDiagnosisTMU::PeriodChannelDiagnosis(const char* lpszBaseIndent, double dPeriod, double dDuty, BOOL bDutyCheck)
{
	StartTimer();

	string strFirstIndent = lpszBaseIndent + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	string strThirdIndent = strSecondIndent + IndentChar();
	string strForthIndent = strThirdIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();
	const char* lpszThirdIndent = strThirdIndent.c_str();
	const char* lpszForthIndent = strForthIndent.c_str();
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Duty value='%.2f'>\n", lpszBaseIndent, dDuty * 100);
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Duty%s/>\n", lpszFirstIndent, bDutyCheck ? "Check" : "Unchecked");
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };	

	map<UINT, CChannelResult*> mapChannelResult;

	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < HDModule::ChannelCountPerControl; ++usChannel)
	{
		vecChannel.push_back(usChannel);
	}

	CHardwareFunction* pHardware = nullptr;

	BOOL bDutyPass = TRUE;
	UINT uPeriodCount = 100;
	int nSampleNum = 10;
	int uHoldOffTime = 0;
	int uHoldOffNum = 1;

	double dCriterion = dPeriod * 1e-3 + 2;

	if (100e6 - EQUAL_ERROR <= dPeriod)
	{
		///<100ms
		uPeriodCount = 5;
		nSampleNum = 1;
		uHoldOffTime = dPeriod / 2;
		uHoldOffNum = 0;
	}
	else if (10e6 - EQUAL_ERROR <= dPeriod)
	{
		///<10ms
		uPeriodCount = 10;
		nSampleNum = 5;
		uHoldOffTime = dPeriod / 2;
		uHoldOffNum = 0;
	}
	else if (1e6 - EQUAL_ERROR <= dPeriod)
	{
		///<1ms
		uPeriodCount = 20;
		nSampleNum = 10;
		uHoldOffTime = dPeriod / 2;
		uHoldOffNum = 1;
	}
	else if (1e3 - EQUAL_ERROR <= dPeriod)
	{
		///<1us
		uPeriodCount = 50;
		nSampleNum = 10;
		uHoldOffTime = 0;
		uHoldOffNum = 1;
	}

	int nTimeBeforeRun = 10;///<ms
	double dTimeout = (nSampleNum * dPeriod * 10) * 1e-6 + nTimeBeforeRun;///<ms
	
	UINT uBindControllerID = Bind(-1);
	pHardware = GetHardware(uBindControllerID);
	pHardware->SetPinLevel(vecChannel, 3, 0, 1.5, 1.5, 0.8);
	ClearBind();

	BYTE bySlotNo = 0;
	BYTE byBoardController = 0;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
	{
		USHORT usOddChannel = usChannel + 2;
		if (DCM_CHANNELS_PER_CONTROL <= usOddChannel)
		{
			usOddChannel = usOddChannel - DCM_CHANNELS_PER_CONTROL;
		}

		int nPatternCount = LoadPeriodPattern(usChannel, usOddChannel, dPeriod, dDuty, uPeriodCount);
		
		uBindControllerID = Bind(-1);
		pHardware = GetHardware(uBindControllerID);
		pHardware->SetRunParameter(0, nPatternCount - 1);
		ClearBind();

		SetTMUParam(TMU_MEAS_MODE::DUTY_PERIOD, TRUE, uHoldOffTime, uHoldOffNum, nSampleNum, dTimeout, usChannel, usOddChannel);
		pHardware->SynRun();
		WaitStop();
		int nErrorCode = 0;

		for (auto uControllerID : m_vecEnableController)
		{
			bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardController);

			BOOL bControllerPass = TRUE;

			pHardware = GetHardware(uControllerID);

			for (BYTE byUnit = 0; byUnit < TMU_UNIT_COUNT_PER_CONTROLLER; ++byUnit)
			{
				StartTimer();

				double dMeasPeriod = pHardware->GetTMUUnitMeasureResult(byUnit, TMU_MEAS_TYPE::FREQ, nErrorCode);
				if (-EQUAL_ERROR < dMeasPeriod)
				{
					dMeasPeriod = 1 / dMeasPeriod * 1e6;
				}
				else
				{
					dMeasPeriod = dMeasPeriod;
				}
				double dMeasDuty = 0;
				if (bDutyCheck)
				{
					dMeasDuty = pHardware->GetTMUUnitMeasureResult(byUnit, TMU_MEAS_TYPE::HIGH_DUTY, nErrorCode);
					if (-EQUAL_ERROR >= dMeasDuty)
					{
						dMeasDuty = dMeasDuty;
					}
				}
				
				auto iterChannelResult = mapChannelResult.find(uControllerID);
				if (mapChannelResult.end() == iterChannelResult || nullptr == iterChannelResult->second)
				{
					CChannelResult* pChannelResult = new CChannelResult();
					mapChannelResult.insert(make_pair(uControllerID, pChannelResult));
					iterChannelResult = mapChannelResult.find(uControllerID);
				}
				USHORT usCurChannel = usChannel;
				if (0 != byBoardController % 2)
				{
					usCurChannel = usOddChannel;
				}
				dTimeConsume = StopTimer();
				iterChannelResult->second->SaveChannelResult(usCurChannel, byUnit, dMeasPeriod, dMeasDuty, dTimeConsume);
			}
		}
	}

	CChannelResult* pChannelResult = nullptr;
	BOOL bAllPass = TRUE;
	for (auto& ChannelResult : mapChannelResult)
	{
		double dControllerTime = 0;
		BYTE byControllerPass = TRUE;
		bySlotNo = HDModule::Instance()->ID2Board(ChannelResult.first, byBoardController);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszFirstIndent, byBoardController, bySlotNo);
		pChannelResult = ChannelResult.second;
		for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
		{
			double dChannelTime = 0;;
			BYTE byChannelPass = TRUE;
			BOOL bSaveChannel = FALSE;

			for (BYTE byUnitIndex = 0; byUnitIndex < TMU_UNIT_COUNT_PER_CONTROLLER;++byUnitIndex)
			{
				BOOL bSaveUnit = FALSE;
				double dMeasPeriod = 0;
				double dMeasDuty = 0;
				pChannelResult->GetChannelResult(usChannel, byUnitIndex, dMeasPeriod, dMeasDuty, dTimeConsume);
				if (-EQUAL_ERROR > dMeasPeriod  || dCriterion < fabs(dMeasPeriod - dPeriod) || (bDutyCheck && (-EQUAL_ERROR > dMeasDuty || 5 < fabs(dMeasDuty - dDuty * 100))))
				{
					bAllPass = FALSE;
					byControllerPass = FALSE;
					byChannelPass = FALSE;
					if (!bSaveChannel)
					{
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Channel value='%d'>\n", lpszSecondIndent, usChannel);
						bSaveChannel = TRUE;
					}
					if (!bSaveUnit)
					{
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Unit value='%d'>\n", lpszThirdIndent, byUnitIndex);
						bSaveUnit = TRUE;
					}
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='Period=%.2f, Duty=%.2f' />\n", lpszForthIndent,
						dMeasPeriod, dMeasDuty);

					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszForthIndent);
				}
				dChannelTime += dTimeConsume;
				dTimeConsume = GetTimeUnits(dTimeConsume, lpszTimeUnits, sizeof(lpszTimeUnits));
				if (bSaveUnit)
				{
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszForthIndent, dTimeConsume, lpszTimeUnits);
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Unit>\n", lpszThirdIndent);
				}
			}
			dControllerTime += dChannelTime;
			dTimeConsume = GetTimeUnits(dChannelTime, lpszTimeUnits, sizeof(lpszTimeUnits));
			if (bSaveChannel)
			{
				if (byChannelPass)
				{
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszThirdIndent);
				}
				else
				{
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszThirdIndent);
				}
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszThirdIndent, dTimeConsume, lpszTimeUnits);

				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Channel>\n", lpszSecondIndent);
			}
		}
		if (byControllerPass)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
		}
		else
		{
			SaveFailController(ChannelResult.first);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
		}
		dTimeConsume = GetTimeUnits(dControllerTime, lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Controller>\n", lpszFirstIndent);

	}
	for (auto& ChannelResult : mapChannelResult)
	{
		if (nullptr != ChannelResult.second)
		{
			delete ChannelResult.second;
			ChannelResult.second = nullptr;
		}
	}

	int nRetVal = 0;
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
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Duty>\n", lpszBaseIndent);
	return nRetVal;
}

int CDiagnosisTMU::LoadPeriodPattern(USHORT usEvenChannel, USHORT usOddChannel, double dPeriod, double dDuty, UINT uPeriodCount)
{
	if (DCM_CHANNELS_PER_CONTROL <= usEvenChannel)
	{
		return -1;
	}
	else if (DCM_CHANNELS_PER_CONTROL <= usOddChannel)
	{
		return -2;
	}
	struct TIMESET_VALUE
	{
		double m_dPeriod;
		WAVE_FORMAT m_WaveFormat;
		double m_pdEdge[EDGE_COUNT];
		TIMESET_VALUE()
		{
			m_dPeriod = 0;
			m_WaveFormat = WAVE_FORMAT::NRZ;
			memset(m_pdEdge, 0, sizeof(m_pdEdge));
		}
	};
	UINT uControllerID = Bind(1);
	CHardwareFunction* pHardware = GetHardware(uControllerID);
	CPattern EvenPattern(*pHardware);
	ClearBind();
	uControllerID = Bind(0);
	pHardware = GetHardware(uControllerID);
	CPattern OddPattern(*pHardware);
	ClearBind();

	UINT uPatternCount = 0;

	char lpszPattern[DCM_CHANNELS_PER_CONTROL + 1] = "XXXXXXXXXXXXXXXX";
	BYTE byTimesetCount = 2;
	map<BYTE, TIMESET_VALUE> mapTimeset;

	if (1e3 + EQUAL_ERROR > dPeriod)
	{
		TIMESET_VALUE TimesetValue;
		TimesetValue.m_WaveFormat = WAVE_FORMAT::RZ;

		TimesetValue.m_dPeriod = dPeriod;
		TimesetValue.m_pdEdge[0] = 4;
		TimesetValue.m_pdEdge[1] = TimesetValue.m_pdEdge[0] + TimesetValue.m_dPeriod * dDuty;
		TimesetValue.m_pdEdge[2] = TimesetValue.m_pdEdge[0];
		TimesetValue.m_pdEdge[3] = TimesetValue.m_pdEdge[1];
		TimesetValue.m_pdEdge[4] = TimesetValue.m_dPeriod / 2;
		TimesetValue.m_pdEdge[5] = TimesetValue.m_dPeriod * 3 / 4;

		mapTimeset.insert(make_pair(0, TimesetValue));

		if (3 >= uPeriodCount)
		{
			uPeriodCount = 4;
		}

		memset(lpszPattern, 'L', sizeof(lpszPattern));
		lpszPattern[DCM_CHANNELS_PER_CONTROL] = 0;
		lpszPattern[usOddChannel] = '1';
		///<Load pattern to even controller
		uControllerID = Bind(TRUE);
		EvenPattern.AddPattern(0, TRUE, lpszPattern, 0, "INC", "", 0, FALSE, FALSE);
		EvenPattern.AddPattern(1, TRUE, lpszPattern, 0, "REPEAT", "", uPeriodCount - 3, FALSE, FALSE);
		EvenPattern.AddPattern(2, TRUE, lpszPattern, 0, "INC", "", 0, FALSE, FALSE);
		lpszPattern[usOddChannel] = 'L';
		EvenPattern.Load();
		ClearBind();

		///<Load pattern to even controller
		lpszPattern[usEvenChannel] = '1';

		uControllerID = Bind(FALSE);
		OddPattern.AddPattern(0, TRUE, lpszPattern, 0, "INC", "", 0, FALSE, FALSE);
		OddPattern.AddPattern(1, TRUE, lpszPattern, 0, "REPEAT", "", uPeriodCount - 3, FALSE, FALSE);
		OddPattern.AddPattern(2, TRUE, lpszPattern, 0, "INC", "", 0, FALSE, FALSE);
		OddPattern.Load();
		ClearBind();

		uPatternCount = 3;
	}
	else
	{
		int nTimesetIndex = 0;
		double dMainPatternPeriod[2] = { 0 };
		double dSecondPatternPeriod[2] = { 0 };
		UINT uMainPatternCount[2] = { 0 };
		UINT uSecondPatternCount[2] = { 0 };
		BYTE byTimesetIndex = 0;

		auto SaveEdge = [&mapTimeset, &byTimesetIndex](double dPeriod)
		{
			TIMESET_VALUE TimesetValue;
			TimesetValue.m_dPeriod = dPeriod;
			TimesetValue.m_WaveFormat = WAVE_FORMAT::NRZ;
			TimesetValue.m_pdEdge[0] = 0;
			TimesetValue.m_pdEdge[1] = dPeriod / 2;
			TimesetValue.m_pdEdge[1] = TimesetValue.m_dPeriod / 2 + TimesetValue.m_pdEdge[0];
			TimesetValue.m_pdEdge[2] = TimesetValue.m_pdEdge[0];
			TimesetValue.m_pdEdge[3] = TimesetValue.m_pdEdge[1];
			TimesetValue.m_pdEdge[4] = TimesetValue.m_dPeriod / 2;
			TimesetValue.m_pdEdge[5] = TimesetValue.m_dPeriod * 3 / 4;
			mapTimeset.insert(make_pair(byTimesetIndex++, TimesetValue));
		};

		for (int nLevelIndex = 0; nLevelIndex < 2; ++nLevelIndex)
		{
			double dPinLevelTime = dPeriod * (0 == nLevelIndex ?dDuty : (1 -dDuty));

			dMainPatternPeriod[nLevelIndex] = dPinLevelTime;
			uMainPatternCount[nLevelIndex] = 1;
			dSecondPatternPeriod[nLevelIndex] = -1;
			if (MAX_PERIOD + EQUAL_ERROR < dMainPatternPeriod[nLevelIndex])
			{
				dMainPatternPeriod[nLevelIndex] = MAX_PERIOD;
				uMainPatternCount[nLevelIndex] = dPinLevelTime / dMainPatternPeriod[nLevelIndex];
				dSecondPatternPeriod[nLevelIndex] = dPinLevelTime - uMainPatternCount[nLevelIndex] * dMainPatternPeriod[nLevelIndex];
				uSecondPatternCount[nLevelIndex] = (dPinLevelTime - uMainPatternCount[nLevelIndex] * dMainPatternPeriod[nLevelIndex]) / dSecondPatternPeriod[nLevelIndex];
				if (MIN_PERIOD > dSecondPatternPeriod[nLevelIndex])
				{
					dSecondPatternPeriod[nLevelIndex] = MIN_PERIOD;
					///<Ignore the period greater
				}
			}

			SaveEdge(dMainPatternPeriod[nLevelIndex]);
			if (0 != uSecondPatternCount[nLevelIndex])
			{
				SaveEdge(dSecondPatternPeriod[nLevelIndex]);
			}
		}

		///<Load pattern
		UINT uCycleCount = uPeriodCount;
		UINT uBasePatternIndex = 0;
		for (int nPeriodIndex = 0; nPeriodIndex < uCycleCount;++nPeriodIndex)
		{
			int nCyclePatternLineIndex = 0;
			for (int nControllerType = 0; nControllerType < 2; ++nControllerType)
			{
				nCyclePatternLineIndex = 0;
				byTimesetIndex = 0;
				USHORT usChannel = usEvenChannel;

				CPattern* pPattern = &OddPattern;
				if (0 != nControllerType)
				{
					usChannel = usOddChannel;
					pPattern = &EvenPattern;
				}

				Bind(nControllerType);

				char cPattern = '1';
				for (int nLevelIndex = 0; nLevelIndex < 2; ++nLevelIndex)
				{
					cPattern = '1';
					if (0 != nLevelIndex)
					{
						cPattern = '0';
					}

					memset(lpszPattern, 'X', sizeof(lpszPattern));
					lpszPattern[usChannel] = cPattern;
					lpszPattern[DCM_CHANNELS_PER_CONTROL] = 0;

					pPattern->AddPattern(uBasePatternIndex + nCyclePatternLineIndex++, TRUE, lpszPattern, byTimesetIndex, "INC", "", 0, FALSE, FALSE);
					if (2 <= uMainPatternCount[nLevelIndex])
					{
						if (2 == uMainPatternCount[nLevelIndex])
						{
							pPattern->AddPattern(uBasePatternIndex + nCyclePatternLineIndex++, TRUE, lpszPattern, byTimesetIndex, "INC", "", 0, FALSE, FALSE);
						}
						else
						{
							pPattern->AddPattern(uBasePatternIndex + nCyclePatternLineIndex++, TRUE, lpszPattern, byTimesetIndex, "REPEAT", "", uMainPatternCount[nLevelIndex] - 2, FALSE, FALSE);
						}
					}
					++byTimesetIndex;

					if (0 != uSecondPatternCount[nLevelIndex])
					{
						pPattern->AddPattern(uBasePatternIndex + nCyclePatternLineIndex++, TRUE, lpszPattern, byTimesetIndex, "INC", "", 0, FALSE, FALSE);
						++byTimesetIndex;
					}
				}
				pPattern->Load();
				ClearBind();
			}
			uBasePatternIndex += nCyclePatternLineIndex;
		}
		uPatternCount = uBasePatternIndex;
	}
	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
	{
		vecChannel.push_back(usChannel);
	}

	uControllerID = Bind(-1);
	pHardware = GetHardware(uControllerID);
	for (auto& Timeset : mapTimeset)
	{
		pHardware->SetPeriod(Timeset.first, Timeset.second.m_dPeriod);
		pHardware->SetEdge(vecChannel, Timeset.first, Timeset.second.m_pdEdge, Timeset.second.m_WaveFormat, IO_FORMAT::NRZ);
	}

	ClearBind();

	char lpszGetPattern[10][17] = { 0 };
	OddPattern.ReadPattern(TRUE, 0, 10, lpszGetPattern);

	return uPatternCount;
}

int CDiagnosisTMU::DelayDiagnosis(const char* lpszBaseIndent, double dDelay)
{
	if (!m_nEnableStatus)
	{
		return 0;
	}
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[8] = { 0 };
	double dShowDelay = GetPeriodUnits(dDelay, lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Delay value='%.1f%s'>\n", lpszBaseIndent, dShowDelay, lpszTimeUnits);

	string strFirstIndent = lpszBaseIndent + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	string strThirdIndent = strSecondIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();
	const char* lpszThirdIndent = strThirdIndent.c_str();


	const BYTE byTriggerEdgeCount = 2;

	const char lpszTriggerEdgeType[byTriggerEdgeCount][8] = {"Raise", "Fall"};
	const BOOL bRaiseTriggerEdge[byTriggerEdgeCount] = { TRUE, FALSE };

	int nLineCount = 0;

	double dTimeout = 10;
	if (1e6 - EQUAL_ERROR <= dDelay)
	{
		dTimeout = dDelay * 2 * 1e-6;
	}
	double dCriterion = dDelay * 1e-3 + 2;
	if (1000 > dDelay)
	{
		dCriterion = 3;
	}
	BOOL bDelayPass = TRUE;
	int byTriggerEdgeIndex = 0;
	for (auto TriggerEdge : bRaiseTriggerEdge)
	{
		StartTimer();
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<TriggerEdge value='%s'>\n", lpszFirstIndent, lpszTriggerEdgeType[byTriggerEdgeIndex]);
		
		nLineCount = LoadDelayPattern(dDelay, TriggerEdge);

		SetTMUParam(TMU_MEAS_MODE::SIGNAL_DELAY, TriggerEdge, 0, 0, 0, dTimeout);

		CHardwareFunction* pHardware = GetHardware(m_vecEnableController[0]);
		pHardware->SynRun();
		WaitStop();

		///<Get the delay of each channel
		BYTE bySlotNo = 0;
		BYTE byBoardController = 0;
		BOOL bEdgePass = TRUE;
		int nErrorCode = 0;
		for (auto uControllerID : m_vecEnableController)
		{
			StartTimer();

			bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardController);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszSecondIndent, byBoardController, bySlotNo);

			BOOL bControllerPass = TRUE;

			pHardware = GetHardware(uControllerID);
			double dMeasureDelay = pHardware->GetTMUUnitMeasureResult(1, TMU_MEAS_TYPE::DELAY, nErrorCode);
			if (-EQUAL_ERROR < dMeasureDelay)
			{
				dMeasureDelay *= 1e3;
			}
			if (-EQUAL_ERROR > dMeasureDelay || dCriterion < fabs(dMeasureDelay - dDelay))
			{
				bEdgePass = FALSE;
				bDelayPass = FALSE;
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='%.2f'/>\n", lpszThirdIndent, dMeasureDelay);

				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszThirdIndent);
				SaveFailController(uControllerID);
			}
			else
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszThirdIndent);
			}
			dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszThirdIndent, dTimeConsume, lpszTimeUnits);

			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</controller>\n", lpszSecondIndent);
		}
		if (bEdgePass)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
		}
		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</TriggerEdge>\n", lpszFirstIndent);
		++byTriggerEdgeIndex;
	}
	int nRetVal = 0;
	if (bDelayPass)
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

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</TriggerEdge>\n", lpszBaseIndent);

	return nRetVal;
}

void CDiagnosisTMU::ShowUIResult()
{
	BYTE bySlotNo = 0;
	BYTE byBoardController = 0;
	for (auto uControllerID : m_vecEnableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardController);
		if (m_setFailController.end() != m_setFailController.find(uControllerID))
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

UINT CDiagnosisTMU::Bind(int nEvenController)
{
	set<BYTE> setSlot;
	set<BYTE> setController;
	BYTE byTargetSlot = 0;
	UINT uTargetControllerID = 0;
	BYTE byCurSlot = 0;
	BYTE byController = 0;
	BOOL bAllController = FALSE;
	if (-1 == nEvenController)
	{
		bAllController = TRUE;
	}
	BYTE byRemainder = nEvenController ? 0 : 1;

	for (UINT uControllerID : m_vecEnableController)
	{
		byCurSlot = HDModule::Instance()->ID2Board(uControllerID, byController);
		if (bAllController || byRemainder == byController % 2)
		{
			if (setController.end() == setController.find(byController))
			{
				setController.insert(byController);
			}
			if (setSlot.end() == setSlot.find(byCurSlot))
			{
				setSlot.insert(byCurSlot);
			}
			if (0 == byTargetSlot)
			{
				byTargetSlot = byCurSlot;
				uTargetControllerID = uControllerID;
			}
		}
	}
	CBindInfo::Instance()->Bind(setSlot, setController, byTargetSlot);
	return uTargetControllerID;
}

void CDiagnosisTMU::ClearBind()
{
	CBindInfo::Instance()->ClearBind();
}

inline void CDiagnosisTMU::SaveFailController(UINT uControllerID)
{
	if (m_setFailController.end() == m_setFailController.find(uControllerID))
	{
		m_setFailController.insert(uControllerID);
	}
}

int CDiagnosisTMU::LoadDelayPattern(double dDelay, BOOL bRaiseTriggerEdge)
{
	double dPeriod = MIN_PERIOD;
	int nDifferentLineCount = 0;
	double dFallOffset = -1;
	do 
	{
		if (-EQUAL_ERROR < dFallOffset)
		{
			dPeriod *= 2;
		}
		if (dDelay > dPeriod)
		{
			nDifferentLineCount = dDelay / dPeriod;
			dFallOffset = dDelay - dPeriod * nDifferentLineCount;
		}
		else
		{
			nDifferentLineCount = 0;
			dFallOffset = dDelay;
		}
	} while (4 > dPeriod - dFallOffset);


	double dEdge[EDGE_COUNT] = { 0 };
	double dEdge1[EDGE_COUNT] = { 0 };
	dEdge[0] = 0;
	dEdge[1] = dPeriod / 2;
	dEdge[2] = dEdge[0];
	dEdge[3] = dEdge[1];
	dEdge[4] = dPeriod / 2;
	dEdge[5] = dPeriod * 3 / 4;

	dEdge1[0] = dFallOffset;
	dEdge1[1] = dEdge1[0] + 2;
	dEdge1[2] = dEdge1[0];
	dEdge1[3] = dEdge1[1];
	dEdge1[4] = dPeriod / 2;
	dEdge1[5] = dPeriod * 3 / 4;

	UINT uOddControllerID = Bind(FALSE);
	CHardwareFunction* pOddBindHardware = GetHardware(uOddControllerID);
	CPattern OddPattern(*pOddBindHardware);
	ClearBind();
	UINT uEvenControllerID = Bind(TRUE);
	ClearBind();
	CHardwareFunction* pEvenBindHardware = GetHardware(uEvenControllerID);
	CPattern EvenPattern(*pEvenBindHardware);

	CHANNEL_OUTPUT_STATUS ChannelStatus = CHANNEL_OUTPUT_STATUS::HIGH;
	UINT uPatternIndex = 0;

	int nLastLineRepeatCount = 0;
	const int nMaxRepeatTimes = 60000;
	int nRepeatPatternCount = nDifferentLineCount / (nMaxRepeatTimes + 1);
	nLastLineRepeatCount = nDifferentLineCount % (nMaxRepeatTimes + 1);
	BOOL bLastRepeatDiff = FALSE;
	if (0 != nLastLineRepeatCount)
	{
		bLastRepeatDiff = TRUE;
		--nLastLineRepeatCount;
		++nRepeatPatternCount;
	}

	char lpszCMD[12] = "REPEAT";
	if (bRaiseTriggerEdge)
	{
		ChannelStatus = CHANNEL_OUTPUT_STATUS::LOW;
		EvenPattern.AddPattern(uPatternIndex, TRUE, "LL00LLLLLLLLLLLL", 0, "INC", "", 0, FALSE, FALSE);
		OddPattern.AddPattern(uPatternIndex++, TRUE, "00LLLLLLLLLLLLLL", 0, "INC", "", 0, FALSE, FALSE);
		if (0 < nDifferentLineCount)
		{
			int nCurRepeatCount = 60000;
			for (int nRepeatLineIndex = 0; nRepeatLineIndex < nRepeatPatternCount; ++nRepeatLineIndex)
			{
				if (nRepeatLineIndex + 1 == nRepeatPatternCount && bLastRepeatDiff)
				{
					nCurRepeatCount = nLastLineRepeatCount;
					if (0 == nCurRepeatCount)
					{
						strcpy_s(lpszCMD, sizeof(lpszCMD), "INC");
					}
				}
				EvenPattern.AddPattern(uPatternIndex, TRUE, "LL10LLLLLLLLLLLL", 0, lpszCMD, "", nCurRepeatCount, FALSE, FALSE);
				OddPattern.AddPattern(uPatternIndex++, TRUE, "10LLLLLLLLLLLLLL", 0, lpszCMD, "", nCurRepeatCount, FALSE, FALSE);
			}
		}
		EvenPattern.AddPattern(uPatternIndex, TRUE, "LL11LLLLLLLLLLLL", 0, "INC", "", 0, FALSE, FALSE);
		OddPattern.AddPattern(uPatternIndex++, TRUE, "11LLLLLLLLLLLLLL", 0, "INC", "", 0, FALSE, FALSE);

		EvenPattern.AddPattern(uPatternIndex, TRUE, "LL11LLLLLLLLLLLL", 0, "INC", "", 0, FALSE, FALSE);
		OddPattern.AddPattern(uPatternIndex++, TRUE, "11LLLLLLLLLLLLLL", 0, "INC", "", 0, FALSE, FALSE);
	}
	else
	{
		ChannelStatus = CHANNEL_OUTPUT_STATUS::HIGH;
		EvenPattern.AddPattern(uPatternIndex, TRUE, "LL11LLLLLLLLLLLL", 0, "INC", "", 0, FALSE, FALSE);
		OddPattern.AddPattern(uPatternIndex++, TRUE, "11LLLLLLLLLLLLLL", 0, "INC", "", 0, FALSE, FALSE);
		if (0 != nDifferentLineCount)
		{
			int nCurRepeatCount = 60000;
			for (int nRepeatLineIndex = 0; nRepeatLineIndex < nRepeatPatternCount; ++nRepeatLineIndex)
			{
				if (nRepeatLineIndex + 1 == nRepeatPatternCount && bLastRepeatDiff)
				{
					nCurRepeatCount = nLastLineRepeatCount;
					if (0 == nCurRepeatCount)
					{
						strcpy_s(lpszCMD, sizeof(lpszCMD), "INC");
					}
				}
				EvenPattern.AddPattern(uPatternIndex, TRUE, "LL01LLLLLLLLLLLL", 0, lpszCMD, "", nCurRepeatCount, FALSE, FALSE);
				OddPattern.AddPattern(uPatternIndex++, TRUE, "01LLLLLLLLLLLLLL", 0, lpszCMD, "", nCurRepeatCount, FALSE, FALSE);
			}
		}
		EvenPattern.AddPattern(uPatternIndex, TRUE, "LL00LLLLLLLLLLLL", 0, "INC", "", 0, FALSE, FALSE);
		OddPattern.AddPattern(uPatternIndex++, TRUE, "00LLLLLLLLLLLLLL", 0, "INC", "", 0, FALSE, FALSE);
		EvenPattern.AddPattern(uPatternIndex, TRUE, "LL00LLLLLLLLLLLL", 0, "INC", "", 0, FALSE, FALSE);
		OddPattern.AddPattern(uPatternIndex++, TRUE, "00LLLLLLLLLLLLLL", 0, "INC", "", 0, FALSE, FALSE);
	}


	vector<USHORT> vecConnectChannel;
	vecConnectChannel.push_back(0);
	vecConnectChannel.push_back(1);
	vecConnectChannel.push_back(2);
	vecConnectChannel.push_back(3);
	
	///<Even controller
	Bind(TRUE);
	EvenPattern.Load();
	ClearBind();

	///<Odd controller
	Bind(FALSE);
	OddPattern.Load();
	ClearBind();

	UINT uAllBindControllerID = Bind(-1);
	CHardwareFunction* pHardware = GetHardware(uAllBindControllerID);
	vector<USHORT> vecBaseChannel;
	vecBaseChannel.push_back(0);
	vecBaseChannel.push_back(2);
	vector<USHORT> vecCompareChannel;
	vecCompareChannel.push_back(1);
	vecCompareChannel.push_back(3);

	pHardware->SetPeriod(0, dPeriod);
	pHardware->SetPinLevel(vecConnectChannel, 3, 0, 1.5, 1.5, 0.8);
	pHardware->SetEdge(vecBaseChannel, 0, dEdge, WAVE_FORMAT::NRZ, IO_FORMAT::NRZ);
	pHardware->SetEdge(vecCompareChannel, 0, dEdge1, WAVE_FORMAT::NRZ, IO_FORMAT::NRZ);
	pHardware->SetRunParameter(0, uPatternIndex - 1);
	pHardware->SetChannelStatus(vecConnectChannel, ChannelStatus);
	ClearBind();

	return 0;
}

void CDiagnosisTMU::SetTMUParam(TMU_MEAS_MODE MeasMode, BOOL bRaiseTriggerEdge, UINT uHoldOffTime, UINT uHoldOffNum, UINT uSampleNum, double dTimeout)
{
	CHardwareFunction* pHardware = nullptr;
	BYTE bySlotNo = 0;
	BYTE byBoardController = 0;

	for (auto uControllerID : m_vecEnableController)
	{
		vector<USHORT> vecConnectChannel;
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardController);
		pHardware = GetHardware(uControllerID);
		for (BYTE byUnit = 0; byUnit < TMU_UNIT_COUNT_PER_CONTROLLER; ++byUnit)
		{
			USHORT usConnectChannel = byUnit;
			if (0 != byBoardController % 2)
			{
				usConnectChannel += 2;
			}
			vecConnectChannel.push_back(usConnectChannel);
			pHardware->SetTMUUnitChannel(usConnectChannel, byUnit);
		}
		pHardware->SetTMUParam(vecConnectChannel, bRaiseTriggerEdge, uHoldOffTime, uHoldOffNum);
		pHardware->TMUMeasure(vecConnectChannel, MeasMode, uSampleNum, dTimeout);
	}
}

void CDiagnosisTMU::SetTMUParam(TMU_MEAS_MODE MeasMode, BOOL bRaiseTriggerEdge, UINT uHoldOffTime, UINT uHoldOffNum, UINT uSampleNum, double dTimeout, USHORT usEvenUnitChannel, USHORT usOddUnitChannel)
{
	CHardwareFunction* pHardware = nullptr;
	BYTE bySlotNo = 0;
	BYTE byBoardController = 0;
	for (auto uControllerID : m_vecEnableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardController);
		pHardware = GetHardware(uControllerID);
		vector<USHORT> vecConnectChannel;
		USHORT usConnectChannel = usEvenUnitChannel;
		if (0 != byBoardController % 2)
		{
			usConnectChannel = usOddUnitChannel;
		}
		vecConnectChannel.push_back(usConnectChannel);
		for (BYTE byUnit = 0; byUnit < TMU_UNIT_COUNT_PER_CONTROLLER; ++byUnit)
		{			
			pHardware->SetTMUUnitChannel(usConnectChannel, byUnit);
		}

		pHardware->SetTMUParam(vecConnectChannel, bRaiseTriggerEdge, uHoldOffTime, uHoldOffNum);
		pHardware->TMUMeasure(vecConnectChannel, MeasMode, uSampleNum, dTimeout);
	}
}

int CChannelResult::SaveChannelResult(USHORT usChannel, BYTE byUnit, double dPeriod, double dDuty, double dTime)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	if (TMU_UNIT_COUNT_PER_CONTROLLER <= byUnit)
	{
		return -2;
	}
	auto iteChannel = m_mapResult.find(usChannel);
	if (m_mapResult.end() == iteChannel)
	{
		CHANNEL_RESULT ChannelResult;
		m_mapResult.insert(make_pair(usChannel, ChannelResult));
		iteChannel = m_mapResult.find(usChannel);
	}
	iteChannel->second.m_dMeasPeriod[byUnit] = dPeriod;
	iteChannel->second.m_dMeasDuty[byUnit] = dDuty;
	iteChannel->second.m_dTime[byUnit] = dTime;
	return 0;
}

int CChannelResult::GetChannelResult(USHORT usChannel, BYTE byUnit, double& dPeriod, double& dDuty, double& dTime)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	if (TMU_UNIT_COUNT_PER_CONTROLLER <= byUnit)
	{
		return -2;
	}
	auto iterChannel = m_mapResult.find(usChannel);
	if (m_mapResult.end() == iterChannel)
	{
		return -3;
	}
	dPeriod = iterChannel->second.m_dMeasPeriod[byUnit];
	dDuty = iterChannel->second.m_dMeasDuty[byUnit];
	dTime = iterChannel->second.m_dTime[byUnit];
	return 0;
}
