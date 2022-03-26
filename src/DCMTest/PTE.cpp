#include "PTE.h"
#include "UserRes.h"
#include "SM8213.h"
#include "UserRes.h"
#include <fstream>
#include <iostream>
using namespace std;
CPTE::CPTE()
{
	int nBoardCount = dcm_GetBoardInfo(nullptr, 0);
	if (0 < nBoardCount)
	{
		try
		{
			BYTE* pbyBoard = new BYTE[nBoardCount];
			memset(pbyBoard, 0, nBoardCount * sizeof(BYTE));
			dcm_GetBoardInfo(pbyBoard, nBoardCount);
			for (int nBoardIndex = 0; nBoardIndex < nBoardCount; nBoardIndex++)
			{
				m_vecBoard.push_back(pbyBoard[nBoardIndex]);
			}
			if (nullptr != pbyBoard)
			{
				delete[] pbyBoard;
				pbyBoard = nullptr;
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << endl;
		}
	}
	m_strAllPin = "G_ALLPIN";
}

CPTE::~CPTE()
{
}

int CPTE::SetVectorFile(const char* lpszVectorFile)
{
	if (nullptr == lpszVectorFile)
	{
		return -1;
	}
	fstream VectorFile(lpszVectorFile, ios::in);
	if (!VectorFile.is_open())
	{
		return -3;
	}
	VectorFile.close();
	int nRetVal = dcm_loadvectorfile(lpszVectorFile, FALSE);
	if (-1 != nRetVal)
	{
		return -4;
	}
	return 0;
}

int CPTE::ChannelRedistribution()
{
	return 0;
}

int CPTE::ChannelDistribute(BYTE byControllerCount, BOOL bParallel, USHORT usChannelCountPerSite, USHORT usSiteCount)
{
	USHORT usChannelCount = byControllerCount * DCM_CHANNELS_PER_CONTROL;
	if (m_vecBoard.size() * DCM_MAX_CONTROLLERS_PRE_BOARD < byControllerCount)
	{
		///<The controller count are bigger than board existed
		return -1;
	}
	if (0 == usSiteCount)
	{
		return -2;
	}
	if (usChannelCountPerSite * usSiteCount > usChannelCount)
	{
		///<The channel is not enough to distributed
		return -3;
	}
	BOOL bDisorderChannel = FALSE;///<Whether disorder the channel to make channel distribution not in parallel
	int nChannelOverOccupied = DCM_CHANNELS_PER_CONTROL * byControllerCount / usSiteCount - usChannelCountPerSite;
	int nSiteCountPerController = DCM_CHANNELS_PER_CONTROL / (nChannelOverOccupied + usChannelCountPerSite);
	if (nSiteCountPerController * usSiteCount < byControllerCount)
	{
		nSiteCountPerController = byControllerCount / usSiteCount;
	}
	if (bParallel)
	{
		if (1 < usSiteCount && 1 != nSiteCountPerController)
		{
			///<The controller count is too little to support parallel
			return -4;
		}
	}
	else if (1 == nSiteCountPerController && 1 < usSiteCount)
	{
		///<One site in each controller, change the distribution in case of parallel
		bDisorderChannel = TRUE;
	}
	Reset();
	CPinInfo* pPinInfo = nullptr;
	int nBoardIndex = 0;
	int nChannelOffset = 0;
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; usSiteNo++)
	{
		int nBaseChannel = (nChannelOverOccupied + usChannelCountPerSite) * usSiteNo;
		BYTE bySlotNo = m_vecBoard[nBoardIndex];
		for (USHORT usPin = 0; usPin < usChannelCountPerSite; usPin++)
		{
			char lpszPinName[32] = { 0 };
			sprintf_s(lpszPinName, sizeof(lpszPinName), "PIN%d", usPin);

			auto iterPin = m_mapPinInfo.find(lpszPinName);
			if (m_mapPinInfo.end() == iterPin)
			{
				pPinInfo = new CPinInfo(lpszPinName);
				m_mapPinInfo.insert(make_pair(lpszPinName, pPinInfo));
				iterPin = m_mapPinInfo.find(lpszPinName);
			}
			pPinInfo = iterPin->second;
			if ((nBoardIndex + 1) * DCM_MAX_CHANNELS_PER_BOARD <= nBaseChannel)
			{
				++nBoardIndex;
			}
			if (bDisorderChannel && 1 == usSiteNo)
			{
				nChannelOffset = 1;
				if ((nBaseChannel + nChannelOffset) / DCM_CHANNELS_PER_CONTROL != nBaseChannel / DCM_CHANNELS_PER_CONTROL)
				{
					nChannelOffset = -1;
				}
				nBaseChannel += nChannelOffset;
			}
			pPinInfo->AddChannel(m_vecBoard[nBoardIndex], nBaseChannel++ % DCM_MAX_CHANNELS_PER_BOARD);

			if (bDisorderChannel && 1 == usSiteNo)
			{
				nBaseChannel -= nChannelOffset;
			}
		}
		nBaseChannel += nChannelOverOccupied;
	}

	string strPin;
	char lpszPin[32] = { 0 };
	vector<CHANNEL_INFO> vecChannel;
	dcm_EnableAddPin(TRUE, FALSE);
	string strAllPinList;
	USHORT usPinNo = 0;
	for (auto& PinInfo : m_mapPinInfo)
	{
		strPin.clear();
		PinInfo.second->GetChannel(vecChannel);
		for (auto& Channel : vecChannel)
		{
			sprintf_s(lpszPin, sizeof(lpszPin), "S%d_%d,", Channel.m_bySlotNo, Channel.m_usChannel);
			strPin += lpszPin;
		}
		dcm_AddPin(PinInfo.first.c_str(), usPinNo++, strPin.c_str());
		strAllPinList += PinInfo.first + ",";
	}
	dcm_SetPinGroup(m_strAllPin.c_str(), strAllPinList.c_str());
	
	return 0;
}

double CPTE::GetPTE(BYTE byControllerCount, BOOL bParallel, USHORT usChannelCountPerSite, USHORT usSiteCount, int nCycleTimes)
{
	LARGE_INTEGER TimeStart, TimeStop, TimeFreq;
	QueryPerformanceFrequency(&TimeFreq);

	auto TimeCal = [&]()
	{
		double dTime = 0;
		for (int nCycleIndex = 0; nCycleIndex < nCycleTimes; nCycleIndex++)
		{
			ResetStatus();
			delay_ms(10);
			QueryPerformanceCounter(&TimeStart);
			FuncExecute();
			QueryPerformanceCounter(&TimeStop);
			dTime += (double)(TimeStop.QuadPart - TimeStart.QuadPart) / TimeFreq.QuadPart;
		}
		return dTime;
	};

	int nRetVal = 0;
// 	///<Calculate the time consume in one site
// 	nRetVal = ChannelDistribute(byControllerCount, bParallel, usChannelCountPerSite, 1);
// 	if (0 != nRetVal)
// 	{
// 		return 1e15 - 1;
// 	}
// 	m_usSiteCount = 1;
// 	if (0 != ChannelReDistribution())
// 	{
// 		///<The channel distribution is not meet requirement
// 		return 1e15 - 1;
// 	}
//
//	double dOneSiteTime = TimeCal();
	
	nRetVal = ChannelDistribute(byControllerCount, bParallel, usChannelCountPerSite, usSiteCount);
	if (0 != nRetVal)
	{
		return 1e15 - 1;
	}

	m_usSiteCount = usSiteCount;
	if (0 != ChannelRedistribution())
	{
		///<The channel distribution is not meet requirement
		return 1e15 - 1;
	}

	DWORD dwSiteStatus = StsGetsSiteStatus();
	StsSetSiteStatus(1);
	m_usSiteCount = 1;
	double dOneSiteTime = TimeCal();
	StsSetSiteStatus(dwSiteStatus);
	m_usSiteCount = usSiteCount;
	double dMultiSiteTime = TimeCal();
	
	double TimeIncreEachSite = (dMultiSiteTime - dOneSiteTime) / (usSiteCount - 1);
	if (0 > TimeIncreEachSite)
	{
		TimeIncreEachSite = TimeIncreEachSite;
	}
	return 100 - TimeIncreEachSite / dOneSiteTime * 100;
}

void CPTE::Reset()
{
	for (auto& PinInfo : m_mapPinInfo)
	{
		if (nullptr != PinInfo.second)
		{
			delete PinInfo.second;
			PinInfo.second = nullptr;
		}
	}
	m_mapPinInfo.clear();
}

CPinInfo::CPinInfo(const char* lpszPinName)
{
	if (nullptr != lpszPinName)
	{
		m_strPinName = lpszPinName;
	}
}

const char* CPinInfo::Name()
{
	return m_strPinName.c_str();
}

void CPinInfo::AddChannel(BYTE bySlotNo, USHORT usChannel)
{
	m_vecChannel.push_back(CHANNEL_INFO(bySlotNo, usChannel));
}

void CPinInfo::GetChannel(vector<CHANNEL_INFO>& vecChannel)
{
	vecChannel.clear();
	vecChannel = m_vecChannel;
}

int CPinInfo::GetSiteNo(const CHANNEL_INFO& Channel)
{
	USHORT usSiteNo = 0;
	for (auto& ChannelInfo : m_vecChannel)
	{
		if (ChannelInfo.m_bySlotNo == Channel.m_bySlotNo && ChannelInfo.m_usChannel == Channel.m_usChannel)
		{
			return usSiteNo;
		}
		++usSiteNo;
	}
	return -1;
}
