#include "pch.h"
#include "DCM400.h"
#include "MainFunction.h"
#include "ErrorCode.h"
#include "DriverAlarm.h"
#include "I2C\I2C.h"
#include <map>
using namespace std;

CMainFunction g_MainFunction(nullptr);///<The main function
map<DCM400*, CMainFunction*> g_mapMain;///<The pair for instance of test program and main function, 
///<key is point pointed to the instance of class DCM400 and value is its main function class
map<DCM400*, CI2C*> g_mapI2C;///<The map for I2C operation, key is point pointed to instance and value is point pointed to I2C class

#define GetMain(pFunction) auto iterHard = g_mapMain.find(this); \
if (g_mapMain.end() == iterHard || nullptr == iterHard->second){\
 pFunction = nullptr; }\
 pFunction = iterHard->second;

#define GetI2C(pFunction) auto iterI2C = g_mapI2C.find(this); \
if (g_mapI2C.end() == iterI2C || nullptr == iterI2C->second){\
 pFunction = nullptr; }\
 pFunction = iterI2C->second;

/**
 * @class CMainNew
 * @brief The class for new main function
*/
class CMainNew
{
public:
	/**
	 * @brief Destructor
	*/
	~CMainNew();
	/**
	 * @brief Get the instance of singleton class
	 * @return The point pointed to instance
	*/
	static CMainNew* Instance();
	/**
	 * @brief New class main function
	 * @param[in] pAlarm The point pointed to CDriverAlarm
	 * @return The point pointed to new class
	*/
	CMainFunction* New(CDriverAlarm* pAlarm);
	/**
	 * @brief Delete the main function
	 * @param[in] pMain The point pointed the new main function
	*/
	void Delete(CMainFunction*& pMain);
private:
	/**
	 * @brief Constructor
	*/
	CMainNew();
private:
	BOOL m_bOwned;///<Whether the global main function have its nowner
	CRITICAL_SECTION m_criNew;///<The critical section to ensure thread safety
};

CMainNew::~CMainNew()
{
	DeleteCriticalSection(&m_criNew);
}

CMainNew* CMainNew::Instance()
{
	static CMainNew Main;
	return &Main;
}

CMainFunction* CMainNew::New(CDriverAlarm* pAlarm)
{
	EnterCriticalSection(&m_criNew);
	CMainFunction* pMain = nullptr;
	if (m_bOwned)
	{
		pMain = new CMainFunction(pAlarm);
		pMain->CopyBoard(g_MainFunction);
	}
	else
	{
		pMain = &g_MainFunction;
		m_bOwned = TRUE;
		g_MainFunction.SetDriverAlarm(pAlarm);
	}

	LeaveCriticalSection(&m_criNew);
	return pMain;
}

void CMainNew::Delete(CMainFunction*& pMain)
{
	EnterCriticalSection(&m_criNew);
	if (&g_MainFunction == pMain)
	{
		m_bOwned = FALSE;
	}
	else
	{
		delete pMain;
		pMain = nullptr;
	}
	LeaveCriticalSection(&m_criNew);
}

CMainNew::CMainNew()
{
	m_bOwned = FALSE;
	InitializeCriticalSection(&m_criNew);
}

/**
 * @class DCM400Data
 * @brief The data of DCM400
*/
class DCM400Data
{
public:
	/**
	 * @brief Constructor
	*/
	DCM400Data();
	/**
	 * @brief Get alarm class
	 * @return The point pointed to class CDriverAlarm
	*/
	CDriverAlarm* GetAlarm();
private:
	CDriverAlarm m_Alarm;///<The driver alarm
};

DCM400Data::DCM400Data()
{
}

CDriverAlarm* DCM400Data::GetAlarm()
{
	return &m_Alarm;
}

DCM400::DCM400()
{
	m_pData = new DCM400Data();
	CMainFunction* pMain = nullptr;
	pMain = CMainNew::Instance()->New(m_pData->GetAlarm());
	g_mapMain.insert(make_pair(this, pMain));
}

DCM400::~DCM400()
{
	auto iterMain = g_mapMain.find(this);
	if (g_mapMain.end() != iterMain)
	{
		CMainNew::Instance()->Delete(iterMain->second);
	}

	if (nullptr != m_pData)
	{
		delete m_pData;
		m_pData = nullptr;
	}
}

using GETPGSFULLPATH = int(_cdecl*)(char* lpszPGSPath, int nBuffSize);
int DCM400::LoadVectorFile(const char* lpszVectorFile, BOOL bReload)
{
	CMainFunction* pHard = nullptr;
	GetMain(pHard);
	if (nullptr == pHard)
	{
		///<Not will happen
		return -1;
	}
	string strFile = lpszVectorFile;
	int nPos = strFile.rfind("\\");
	if (-1 == nPos)
	{
		HMODULE hModule = GetModuleHandle("pgsedit.dll");
		if (nullptr != hModule)
		{
			GETPGSFULLPATH GetPgsFullPath = (GETPGSFULLPATH)GetProcAddress(hModule, "GetPgsFullPath");
			if (nullptr != GetPgsFullPath)
			{
				char lpszFilePath[512] = { 0 };
				GetPgsFullPath(lpszFilePath, sizeof(lpszFilePath));
				string strPGS = lpszFilePath;
				nPos = strPGS.rfind("\\");
				if (0 != nPos)
				{
					strPGS.erase(nPos);
				}
				strFile = strPGS + "\\" + strFile;
			}
		}
	}


	m_pData->GetAlarm()->SetDriverPackName("LoadVectorFile");
	int nRetVal = pHard->LoadVectorFile(strFile.c_str(), bReload);
	if (0 != nRetVal)
	{
		nRetVal = NOT_LOAD_VECTOR;
	}
	m_pData->GetAlarm()->Output();
	return nRetVal;
}

int DCM400::I2CSet(double dPeriod, USHORT usSiteCount, DCM400_I2C_REG_ADDR_MODE REGMode, const char* lpszSCLChannel, const char* lpszSDAChannel)
{
	CI2C* pI2C = nullptr;
	GetI2C(pI2C);
	if (nullptr == pI2C)
	{
		pI2C = new CI2C(m_pData->GetAlarm());
		vector<BYTE> vecBoardExisted;
		CMainFunction* pMain = nullptr;
		GetMain(pMain);
		pMain->GetBoardExisted(vecBoardExisted);
		pI2C->SetExistedBoard(vecBoardExisted);
		g_mapI2C.insert(make_pair(this, pI2C));
	}
	m_pData->GetAlarm()->SetDriverPackName("I2CSet");
	CI2C::REG_MODE Mode = (CI2C::REG_MODE)REGMode;
	int nRetVal = pI2C->Set(dPeriod, usSiteCount, Mode, lpszSCLChannel, lpszSDAChannel);
	if (0 != nRetVal)
	{
		switch (nRetVal)
		{
		case -1:
			///<Site count over range
			nRetVal = I2C_SITE_OVER_RANGE;
			break;
		case -2:
			///<Register byte over range, not will happen
			break;
		case -3:
			///<The channel information is nullptr
			nRetVal = I2C_CHANNEL_INFO_NULLPTR;
			break;
		case -4:
			///<Channel information is blank
			nRetVal = I2C_CHANNEL_INFO_BLANK;
			break;
		case -5:
			///<Channel format wrong
			nRetVal = I2C_CHANNEL_FORMAT_ERROR;
			break;
		case -6:
			///<Channel number is over range
			nRetVal = I2C_CHANNEL_OVER_RANGE;
			break;
		case -7:
			///<Channel not existed
			nRetVal = I2C_CHANNEL_NOT_EXISTED;
			break;
		case -8:
			///<Channel count and site count not equal
			nRetVal = I2C_SITE_CHANNEL_NOT_MATCH;
			break;
		case -9:
			///<The channel of SCL and SDA is conflict
			nRetVal = I2C_CHANNEL_CONFICT;
			break;
		default:
			break;
		}
	}
	m_pData->GetAlarm()->Output();
	return nRetVal;
}
