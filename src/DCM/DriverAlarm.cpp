#include "DriverAlarm.h"
#include "AlarmID.h"
#include "sts8100.h"
using namespace std;
// alarm µÈ¼¶
#define NORMAL_MESSAGE            0
#define WARNING_MESSAGE            1
#define FAILURE                2
#define FATAL_ERROR              3

class COutputAuthorization
{
public:
	static COutputAuthorization* Instance();
	void Apply();
	void Release();
private:
	COutputAuthorization();
private:
	CRITICAL_SECTION m_criSafety;
};

COutputAuthorization* COutputAuthorization::Instance()
{
	static COutputAuthorization OutputAuthorization;
	return &OutputAuthorization;
}

void COutputAuthorization::Apply()
{
	EnterCriticalSection(&m_criSafety);
}

void COutputAuthorization::Release()
{
	LeaveCriticalSection(&m_criSafety);
}

COutputAuthorization::COutputAuthorization()
{
	InitializeCriticalSection(&m_criSafety);
}

CDriverAlarm::CDriverAlarm()
	: m_bClearInfo(TRUE)
{
	m_bPinName = TRUE;
	m_uSiteNo = -1;
	m_AlarmID = ALARM_ID::ALARM_NOALARM;
	m_bSiteInvalidAlarm = FALSE;
	memset(m_lpszAlarmMsg, 0, sizeof(m_lpszAlarmMsg));
	m_AlarmInfo.ParamLevel = AlarmError;
	m_AlarmType = ALARM_TYPE::PARAMETER_OCCURALARM;
}

void CDriverAlarm::SetAlarmLevel(ALARM_LEVEL alarmLevel)
{
	m_AlarmInfo.ParamLevel = (AlarmLevel)alarmLevel;
}

void CDriverAlarm::SetAlarmType(ALARM_TYPE AlaramType)
{
	m_AlarmType = AlaramType;
}

int CDriverAlarm::SetDriverPackName(const char* lpszName)
{
	if (nullptr == lpszName)
	{
		return -1;
	}
	if (!m_bClearInfo && 0 == strcmp(m_AlarmInfo.DriverPackFun, lpszName) )
	{
		return 0;
	}
	m_bClearInfo = FALSE;
	m_AlarmInfo.DriverPackFun = (char*)lpszName;
	return 0;
}

CDriverAlarm::~CDriverAlarm()
{
	for (auto& ShieldPin : m_mapShieldPin)
	{
		if (nullptr != ShieldPin.second)
		{
			delete ShieldPin.second;
			ShieldPin.second = nullptr;
		}
	}
	m_mapShieldPin.clear();

	for (auto& ShieldFunc : m_mapShiedFunction)
	{
		if (nullptr != ShieldFunc.second)
		{
			delete ShieldFunc.second;
			ShieldFunc.second = nullptr;
		}
	}
	m_mapShiedFunction.clear();
}

inline void CDriverAlarm::SetAlarmID(ALARM_ID ID)
{
	m_AlarmID = ID;
}

BOOL CDriverAlarm::GetPin(std::string& strPin, USHORT& usSiteNo)
{
	strPin = m_strPinString;
	usSiteNo = m_uSiteNo;
	return m_bPinName;
}

void CDriverAlarm::SetSite(USHORT uSiteNo)
{
	m_uSiteNo = uSiteNo;
}

void CDriverAlarm::SetPinString(const char* lpszPinString, BOOL bPinName)
{
	m_bPinName = bPinName;
	m_strPinString.clear();
	if (nullptr != lpszPinString)
	{
		m_strPinString = lpszPinString;
	}
	m_AlarmInfo.Resource = (char*)m_strPinString.c_str();
}

void CDriverAlarm::VectorNotLoadedAlarm()
{
	SetAlarmID(ALARM_ID::ALARM_VECTOR_NOT_LOAD);
	SetAlarmMsg("The vector is not loaded.");

	m_AlarmInfo.ParamLevel = AlarmError;
	m_AlarmType = ALARM_TYPE::PARAMETER_OCCURALARM;
}


void CDriverAlarm::Output(BOOL bClearFunctionName)
{
	if (ALARM_ID::ALARM_NOALARM == m_AlarmID)
	{
		ClearInfo(bClearFunctionName);
		return;
	}
	if (0 == m_strModuleInfo.size())
	{
		SetModuleInfo();
	}

	COutputAuthorization::Instance()->Apply();

	if (AlarmWarning != m_AlarmInfo.ParamLevel)
	{
		StsSetLogCode((char*)m_strModuleInfo.c_str(), m_lpszAlarmMsg, FAILURE);
	}
	else
	{
		StsSetLogCode((char*)m_strModuleInfo.c_str(), m_lpszAlarmMsg, WARNING_MESSAGE);
	}
	OutputAlarm();

	COutputAuthorization::Instance()->Release();

	ClearInfo(bClearFunctionName);
	if (bClearFunctionName)
	{
		m_bClearInfo = TRUE;
	}
}

int CDriverAlarm::ParameternullptrAlarm(const char* lpszParamName, USHORT usSiteNo, const char* lpszPinString, BOOL bPinName)
{
	if (nullptr == lpszParamName)
	{
		return -1;
	}
	SetAlarmID(ALARM_ID::ALARM_PARAM_NULLPTR);
	SetAlarmMsg("The parameter(%s) is nullptr.", lpszParamName);
	SetPinString(lpszPinString, bPinName);
	m_strParamName = lpszParamName;
	m_uSiteNo = usSiteNo;
	m_AlarmInfo.ParamLevel = AlarmError;
	m_AlarmType = ALARM_TYPE::PARAMETER_OCCURALARM;
	return 0;
}

int CDriverAlarm::PinError(const char* lpszPinString, BOOL bPinName)
{
	if (nullptr == lpszPinString)
	{
		return -1;
	}
	if (bPinName)
	{
		SetAlarmID(ALARM_ID::ALARM_PIN_NAME_ERROR);
		SetAlarmMsg("The pin name(%s) is not defined in vector file.", lpszPinString); 
	}
	else
	{
		SetAlarmID(ALARM_ID::ALARM_PIN_GROUP_ERROR);
		SetAlarmMsg("The pin group(%s) is not defined by call function SetPinGroup.", lpszPinString);
	}
	SetModuleInfo();
	m_strPinString = lpszPinString;
	m_bPinName = bPinName;
	m_AlarmInfo.ParamLevel = AlarmError;
	m_AlarmType = ALARM_TYPE::PARAMETER_NOT_DEFINED;
	return 0;
}

int CDriverAlarm::TimesetError(const char* lpszTimeset)
{
	if (nullptr == lpszTimeset)
	{
		return -1;
	}

	SetAlarmID(ALARM_ID::ALARM_TIMESET_ERROR);
	SetAlarmMsg("The timeset(%s) is not defined in vector file.", lpszTimeset);
	
	m_AlarmInfo.ParamLevel = AlarmError;
	m_AlarmType = ALARM_TYPE::PARAMETER_OCCURALARM;
	return 0;
}

void CDriverAlarm::AllocateMemoryError()
{
	SetAlarmID(ALARM_ID::ALARM_ALLOCTE_MEMORY_ERROR);
	SetAlarmMsg("Allocate memory fail.");
	m_AlarmInfo.ParamLevel = AlarmError;
	m_AlarmType = ALARM_TYPE::PARAMETER_OCCURALARM;
}

int CDriverAlarm::FunctionUseError(const char* lpszPreviousFunction)
{
	if (nullptr == lpszPreviousFunction)
	{
		return -1;
	}
	SetAlarmID(ALARM_ID::ALARM_FUNCTION_USE_ERROR);
	SetAlarmMsg("Must use the function(%s) before.", lpszPreviousFunction);
	m_AlarmInfo.ParamLevel = AlarmError;
	m_AlarmType = ALARM_TYPE::PARAMETER_OCCURALARM;
	return 0;
}

void CDriverAlarm::UnknownError()
{
	SetAlarmID(ALARM_ID::ALARM_ALLOCTE_MEMORY_ERROR);
	SetAlarmMsg("Unknow error.");
	m_AlarmInfo.ParamLevel = AlarmError;
	m_AlarmType = ALARM_TYPE::PARAMETER_OCCURALARM;
}

int CDriverAlarm::SiteOverScaleAlarm(const char* lpszPinString, USHORT usSiteNo, USHORT uMaxSiteNo, BOOL bPinName)
{
	if (nullptr == lpszPinString)
	{
		return -1;
	}
	SetAlarmID(ALARM_ID::ALARM_SITE_OVER_RANGE);
	SetPinString(lpszPinString, bPinName);
	m_uSiteNo = -1;
	m_AlarmInfo.ParamLevel = AlarmError;
	SetRangeAlarm("usSiteNo", usSiteNo, 0, uMaxSiteNo, (BYTE)DATA_FORMAT::INT_FORMAT);
	return 0;
}

int CDriverAlarm::SetNoBoardAlarm(const char* lpszPinString, BOOL bPinName, USHORT usSiteNo)
{
	SetAlarmID(ALARM_ID::ALARM_BOARD_NOT_EXISTED);
	SetPinString(lpszPinString, bPinName);
	if (nullptr != lpszPinString)
	{		
		if (bPinName)
		{
			SetAlarmMsg("The DCM board of %s in SITE_%d is not exited.", lpszPinString, usSiteNo + 1);
		}
		else
		{
			SetAlarmMsg("The DCM board used in pin group(%s) is not existed.", lpszPinString);
		}
	}
	else
	{
		SetAlarmMsg("No DCM board existed.");
	}
	m_uSiteNo = usSiteNo;

	m_AlarmInfo.ParamLevel = AlarmError;
	m_AlarmInfo.SiteNum = usSiteNo;
	return 0;
}

int CDriverAlarm::SiteInvalidAlarm(const char* lpszPinName, USHORT usSiteNo, BOOL bPinName)
{
	if (nullptr == lpszPinName)
	{
		return -1;
	}
	SetAlarmID(ALARM_ID::ALARM_SITE_INVALID);
	SetPinString(lpszPinName, bPinName);
	SetAlarmMsg("The site(SITE_%d) is invalid.", usSiteNo + 1);
	m_uSiteNo = usSiteNo;
	m_AlarmInfo.ParamLevel = AlarmWarning;
	m_AlarmType = ALARM_TYPE::PARAMETER_SITE_INVALID;
	return 0;
}

int CDriverAlarm::ShieldPin(const char* lpszPinName, USHORT usSiteNo, CHANNEL_INFO& Channel, BOOL bShield)
{
	if (nullptr == lpszPinName)
	{
		return -1;
	}
	auto iterShieldPin = m_mapShieldPin.find(lpszPinName);
	if (m_mapShieldPin.end() == iterShieldPin)
	{
		if (!bShield)
		{
			return 0;
		}
		CShieldPin* pShieldPin = new CShieldPin(lpszPinName);
		m_mapShieldPin.insert(make_pair(lpszPinName, pShieldPin));
		iterShieldPin = m_mapShieldPin.find(lpszPinName);
	}
	else
	{
		iterShieldPin->second->ShieldSite(usSiteNo, Channel, bShield);
	}
	return 0;
}

int CDriverAlarm::GetShieldStatus(const char* lpszPinName, USHORT usSiteNo)
{
	if (nullptr == lpszPinName)
	{
		return -1;
	}
	auto iterShieldPin = m_mapShieldPin.find(lpszPinName);
	if (m_mapShieldPin.end() == iterShieldPin)
	{
		return 0;
	}	
	return iterShieldPin->second->IsShield(usSiteNo);
}


void CDriverAlarm::SetSiteCount(USHORT uSiteCount)
{
	m_uSiteCount = uSiteCount;
}

void CDriverAlarm::SetInvalidSiteAlarm(BOOL bAlarm)
{
	m_bSiteInvalidAlarm = bAlarm;
}

void CDriverAlarm::SetAlarmMsg(const char* lpszFormat, ...)
{
	va_list args;
	va_start(args, lpszFormat);
	vsprintf_s(m_lpszAlarmMsg, sizeof(m_lpszAlarmMsg), lpszFormat, args);
	va_end(args);
	m_AlarmInfo.Describe = m_lpszAlarmMsg;
}

int CDriverAlarm::SetParamName(const char* lpszParameterName)
{
	if (nullptr == lpszParameterName)
	{
		m_strParamName.clear();
		return -1;
	}
	m_strParamName = lpszParameterName;
	return 0;
}

BOOL CDriverAlarm::IsSetMsg()
{
	if (0 != strlen(m_lpszAlarmMsg))
	{
		return TRUE;
	}
	return FALSE;
}

int CDriverAlarm::ShieldAlarm(const char* lpszPinName, USHORT usSiteNo, CHANNEL_INFO& ChannelInfo, const char* lpszFunction, BOOL bShield, ALARM_ID AlarmID)
{
	if (nullptr == lpszPinName || nullptr == lpszFunction)
	{
		return -1;
	}
	auto iterShiled = m_mapShiedFunction.find(lpszFunction);
	if (m_mapShiedFunction.end() == iterShiled)
	{
		if (!bShield)
		{
			return 0;
		}
		CShieldFunction* pShieldFunction = new CShieldFunction(lpszFunction);
		m_mapShiedFunction.insert(make_pair(lpszFunction, pShieldFunction));
		iterShiled = m_mapShiedFunction.find(lpszFunction);
	}
	iterShiled->second->ShieldID((UINT)AlarmID, bShield);
	iterShiled->second->ShieldPin(lpszFunction, usSiteNo, ChannelInfo, bShield);
	return 0;
}

int CDriverAlarm::GetShieldAlarm(const char* lpszPinName, USHORT usSiteNo, const char* lpszFunction, ALARM_ID AlarmID)
{
	if (nullptr == lpszPinName || nullptr == lpszFunction)
	{
		return -1;
	}
	auto iterFunc = m_mapShiedFunction.find(lpszFunction);
	if (m_mapShiedFunction.end() == iterFunc)
	{
		return 0;
	}
	return iterFunc->second->IsShield(lpszPinName, usSiteNo, (UINT)AlarmID);
}

void CDriverAlarm::GetShieldChannel(const char* lpszFunction, std::vector<CHANNEL_INFO>& vecShieldChannel, ALARM_ID AlarmID)
{
	vecShieldChannel.clear();
	if (nullptr == lpszFunction)
	{
		return;
	}
	auto iterShield = m_mapShiedFunction.find(lpszFunction);
	if (m_mapShiedFunction.end() == iterShield)
	{
		return;
	}
	iterShield->second->GetShieldChannel(vecShieldChannel,(UINT)AlarmID);
}

int CDriverAlarm::IsAlarmOpen()
{
	return StsGetAlarmSwitch();
}

inline void CDriverAlarm::SetModuleInfo()
{
	char lpszModuleInfo[256] = { 0 };
	string strFormat="%d//";
	if (0 != m_strPinString.size())
	{
		strFormat += "%s//%s//";
		sprintf_s(lpszModuleInfo, sizeof(lpszModuleInfo), strFormat.c_str(), m_AlarmID, m_strPinString.c_str(), m_AlarmInfo.DriverPackFun);
	}
	else
	{
		strFormat += "//%s//";
		sprintf_s(lpszModuleInfo, sizeof(lpszModuleInfo), strFormat.c_str(), m_AlarmID, m_AlarmInfo.DriverPackFun);
	}
	m_strModuleInfo = lpszModuleInfo;

	if (0 != m_strParamName.size())
	{
		sprintf_s(lpszModuleInfo, "%s", m_strParamName.c_str());
		m_strModuleInfo += lpszModuleInfo;
	}
}

int CDriverAlarm::OutputAlarm()
{
	int ret = 0;
	int site_loop = 0;
	USHORT uChannel = 0;
	bool bWriteAlarmFalg = true;
	bool bTempSiteValidFlag = true;
	int nTempSiteNum = 0;
	USHORT uTempSiteNo = 0;

	if ((USHORT)-1 == m_uSiteNo)
	{
		USHORT uSiteNum = m_uSiteCount;
		nTempSiteNum = uSiteNum;
		if (0 == uSiteNum)
		{
			nTempSiteNum = DCM_MAX_SITE_COUNT;
		}
	}
	else
	{
		nTempSiteNum = 1;
	}
	m_AlarmInfo.ParamType = (ParamAlarmType)m_AlarmType;
	for (site_loop = 0; site_loop < nTempSiteNum; site_loop++)
	{
		if ((USHORT)-1 == m_uSiteNo)
		{
			uTempSiteNo = site_loop;
		}
		else
		{
			uTempSiteNo = m_uSiteNo;
		}
		if (IsAlarmOpen())
		{
			bTempSiteValidFlag = true;
			bWriteAlarmFalg = true;
			if (!m_bSiteInvalidAlarm)
			{
				if (!IsSiteValid(uTempSiteNo))
				{
					bTempSiteValidFlag = false;
				}
			}
			if (m_bPinName && 0 != m_strPinString.size() && 1 == GetShieldStatus(m_AlarmInfo.Resource, m_uSiteNo))
			{
				bWriteAlarmFalg = false;
				break;
			}

			auto iterFunction = m_mapShiedFunction.find(m_AlarmInfo.DriverPackFun);
			if (m_mapShiedFunction.end() != iterFunction)
			{
				if (iterFunction->second->IsShield(m_strPinString.c_str(), m_uSiteNo, (UINT)m_AlarmID))
				{
					bWriteAlarmFalg = false;
					break;
				}
			}


			if (bWriteAlarmFalg && bTempSiteValidFlag)
			{
				m_AlarmInfo.SiteNum = uTempSiteNo;
				STSSetHwAlarm(m_AlarmInfo);
				ret = 1;
			}
		}
	}
	m_AlarmInfo.ParamLevel = AlarmError;
	return ret;
}

void CDriverAlarm::SetRangeAlarm(const char* lpszParamName, double dInputValue, double dMinValue, double dMaxValue, BYTE byDataFormat)
{
	char lpszDataFormat[16] = { 0 };
	sprintf_s(lpszDataFormat, sizeof(lpszDataFormat), "%%.%df", byDataFormat);
	char lpszPrintFormat[256] = { 0 };
	BYTE bLess = FALSE;
	if ((dMaxValue >= (-1 - EQUAL_ERROR)) && (dMaxValue <= (-1 + EQUAL_ERROR)))
	{
		bLess = FALSE;
		sprintf_s(lpszPrintFormat, sizeof(lpszPrintFormat), "The value of %%s(%s) must not less than %s.", lpszDataFormat, lpszDataFormat);
	}
	else if (dMaxValue < dMinValue)
	{
		bLess = TRUE;
		sprintf_s(lpszPrintFormat, sizeof(lpszPrintFormat), "The value of %%s(%s) must less than %s.", lpszDataFormat, lpszDataFormat);
	}
	else
	{
		bLess = FALSE;
		sprintf_s(lpszPrintFormat, sizeof(lpszPrintFormat), "The value of %%s(%s) must between %s and %s.", lpszDataFormat, lpszDataFormat, lpszDataFormat);
	}

	if (bLess)
	{
		sprintf_s(m_lpszAlarmMsg, sizeof(m_lpszAlarmMsg), lpszPrintFormat,
			lpszParamName, dInputValue, dMaxValue);
	}
	else
	{
		sprintf_s(m_lpszAlarmMsg, sizeof(m_lpszAlarmMsg), lpszPrintFormat,
			lpszParamName, dInputValue, dMinValue, dMaxValue);
	}
	m_strParamName = lpszParamName;

	m_AlarmInfo.Describe = m_lpszAlarmMsg;
	m_AlarmType = ALARM_TYPE::PARAMETER_OVERRANGE;
}

inline bool CDriverAlarm::IsSiteValid(USHORT usSiteNo)
{
	bool ret = false;
	BYTE abySiteStatus[DCM_MAX_SITE_COUNT] = { 0 };
	AT_StsGetSitesStatus(abySiteStatus, sizeof(abySiteStatus));
	if (0 != abySiteStatus[usSiteNo])
	{
		ret = true;
	}
	return ret;
}

inline void CDriverAlarm::ClearInfo(BOOL bClear)
{
	m_AlarmID = ALARM_ID::ALARM_NOALARM;
	m_AlarmInfo.ParamLevel = AlarmError;
	m_uSiteNo = -1;
	m_strParamName.clear();
	m_strPinString.clear();
	m_strModuleInfo.clear();
	memset(m_lpszAlarmMsg, 0, sizeof(m_lpszAlarmMsg));
	m_AlarmType = ALARM_TYPE::PARAMETER_OCCURALARM;
	if (bClear)
	{
		m_AlarmInfo.DriverPackFun = (char*)"";
	}
}

CAlarmManage* CAlarmManage::Instance()
{
	static CAlarmManage AlarmManage;
	return &AlarmManage;
}

CDriverAlarm* CAlarmManage::GetAlarm(int nInstanceID)
{
	return Operation(nInstanceID, FALSE);
}

void CAlarmManage::DeleteAlarm(int nInstanceID)
{
	Operation(nInstanceID, TRUE);
}

CAlarmManage::CAlarmManage()
{
	InitializeCriticalSection(&m_criOperation);
}

CDriverAlarm* CAlarmManage::Operation(int nInstanceID, BOOL bDelete)
{
	CDriverAlarm* pDriverAlarm = nullptr;
	EnterCriticalSection(&m_criOperation);
	auto iterAlarm = m_mapAlarm.find(nInstanceID);
	if (m_mapAlarm.end() == iterAlarm)
	{
		if (!bDelete)
		{
			pDriverAlarm = new CDriverAlarm();
			m_mapAlarm.insert(make_pair(nInstanceID, pDriverAlarm));
		}
	}
	else
	{
		pDriverAlarm = iterAlarm->second;
		if (bDelete)
		{
			m_mapAlarm.erase(iterAlarm);
		}
	}
	LeaveCriticalSection(&m_criOperation);
	return pDriverAlarm;
}
