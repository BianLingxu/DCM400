#include "pch.h"
#include "DCM400HardwareInfo.h"
#include "MainFunction.h"
#include "DCM400.h"
#include "PMU.h"
#include "Relay.h"
#include "STSSP8201.h"
#include "BaseAuthorization.h"
#include "Period.h"
#include <map>
using namespace std;
#pragma data_seg("SM8250")
///<The parameters for shared between multi process
BYTE g_byBoardCount = 0;///<The board count
BYTE g_abyBoardSlot[DCM400_MAX_BOARD_NUM] = { 0 };///<The slot number of each board
DCM400_CAL_DATA g_aCalData[DCM400_MAX_BOARD_NUM][DCM400_MAX_CONTROLLERS_PRE_BOARD][DCM400_MAX_CHANNELS_PER_BOARD] = { 0 };

///<For PMU
unsigned char g_aucMeasIRange[DCM400_MAX_BOARD_NUM][DCM400_MAX_CONTROLLERS_PRE_BOARD][DCM400_CHANNELS_PER_CONTROL] = { 0, 0 };///<The PMU current range
unsigned char g_aucMeasType[DCM400_MAX_BOARD_NUM][DCM400_MAX_CONTROLLERS_PRE_BOARD][DCM400_CHANNELS_PER_CONTROL] = { 0, 0 };///<The PMU measurement type 0 - MV  1 - MI
unsigned char g_aucMeasForceMode[DCM400_MAX_BOARD_NUM][DCM400_MAX_CONTROLLERS_PRE_BOARD][DCM400_CHANNELS_PER_CONTROL] = { 0 };///<The PMU force mode, 0 is FV and 1 is FI
unsigned char g_aucLatestMeasType[DCM400_MAX_BOARD_NUM][DCM400_MAX_CONTROLLERS_PRE_BOARD][DCM400_CHANNELS_PER_CONTROL] = { 0, 0 };///<The PMU latest measurement type 0 - MV  1 - MI

double g_adSamplePeriod[DCM400_MAX_BOARD_NUM][DCM400_MAX_CONTROLLERS_PRE_BOARD][DCM400_CHANNELS_PER_CONTROL] = { 0 };///<The sample period of each channel
UINT g_auSampleTimes[DCM400_MAX_BOARD_NUM][DCM400_MAX_CONTROLLERS_PRE_BOARD][DCM400_CHANNELS_PER_CONTROL] = { 0 };///<The sample times
USHORT g_ausPMUAverageData[DCM400_MAX_BOARD_NUM][DCM400_MAX_CONTROLLERS_PRE_BOARD][DCM400_CHANNELS_PER_CONTROL] = { 0 };///<The PMU average data of each controller

///<Period Series
float g_afPeriod[DCM400_MAX_BOARD_NUM][DCM400_MAX_CONTROLLERS_PRE_BOARD][TIME_SERIES_MAX_COUNT] = { 0 };

///<For relay register value
ULONG g_aulRelayREG[DCM400_MAX_BOARD_NUM][3] = {0};

#pragma data_seg()
#pragma comment(linker, "/SECTION:SM8250,RWS")


CBaseAuthorization<void> g_RunAuthorization;///<The vector running authorization

extern std::map<DCM400*, CMainFunction*> g_mapMain;
extern CMainFunction g_MainFunction;///<The main function
/**
 * @struct BOARD_INFO
 * @brief The board information of the board
*/
struct BOARD_INFO
{
	BYTE m_byBoardIndex;///<The board index of the board
	USHORT m_usChannelCount;///<The channel count of the board
	BOARD_INFO()
	{
		m_byBoardIndex = 0;
		m_usChannelCount = 0;
	}
};
map<BYTE, BOARD_INFO> g_mapValidBoard;

bool operator ==(const BOARD_INFO& Source, const BOARD_INFO& Target)
{
	if (Source.m_byBoardIndex != Target.m_byBoardIndex || Source.m_usChannelCount != Target.m_usChannelCount)
	{
		return false;
	}
	return true;
}
/**
 * @brief Set the memory shared by all process which call this driver
*/
void SetMemoryShared()
{
	for (auto& Board : g_mapValidBoard)
	{
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM400_MAX_CONTROLLERS_PRE_BOARD;++byControllerIndex)
		{
			CPMU::Instance()->SetForceModeMemory(Board.first, byControllerIndex, g_aucMeasForceMode[Board.second.m_byBoardIndex][byControllerIndex], 
				g_aucMeasIRange[Board.first][Board.second.m_byBoardIndex]);
			CPMU::Instance()->SetMeasureModeMemory(Board.first, byControllerIndex, g_aucMeasType[Board.second.m_byBoardIndex][byControllerIndex]);
			CPMU::Instance()->SetLatestMeasureModeMemory(Board.first, byControllerIndex, g_aucLatestMeasType[Board.second.m_byBoardIndex][byControllerIndex]);
			CPMU::Instance()->SetAverageDataMemory(Board.first, byControllerIndex, g_ausPMUAverageData[Board.second.m_byBoardIndex][byControllerIndex]);			
			CPMU::Instance()->SetSampleModeMemory(Board.first, byControllerIndex, g_auSampleTimes[Board.second.m_byBoardIndex][byControllerIndex],
				g_adSamplePeriod[Board.second.m_byBoardIndex][byControllerIndex]);

			CPeriod::Instance()->SetMemory(Board.first, byControllerIndex, g_afPeriod[Board.second.m_byBoardIndex][byControllerIndex]);
		}
		CRelayRigister::Instance()->SetRelayMem(Board.first, g_aulRelayREG[Board.first]);
	}
}

/**
 * @brief Set the configuration of board
 * @param[in] nBoardCount The board count
 * @param[in] pAllBoardInfo The point pointed to the board information
 * @return Execute result
 * - 0 Set the configuration successfully
 * - -1 The board information is nullptr or board count is less than 1
*/
int APIENTRY DCM400_SetConfigInfo(short	nBoardCount, const BOARDINFO* pAllBoardInfo)
{
	if (nullptr == pAllBoardInfo || 0 >= nBoardCount)
	{
		return -1;
	}
	auto mapBoard = g_mapValidBoard;
	g_mapValidBoard.clear();
	g_byBoardCount = nBoardCount;
	BOARD_INFO BoardInfo;
	for (int nBoardIndex = 0; nBoardIndex < nBoardCount;++nBoardIndex)
	{
		g_abyBoardSlot[nBoardIndex] = pAllBoardInfo[nBoardIndex].slot;
		BoardInfo.m_byBoardIndex = nBoardIndex;
		auto iterBoard = mapBoard.find(g_abyBoardSlot[nBoardIndex]);
		if (mapBoard.end() != iterBoard)
		{
			BoardInfo.m_usChannelCount = iterBoard->second.m_usChannelCount;
		}
		else
		{
			BoardInfo.m_usChannelCount = 0;
		}
		g_mapValidBoard.insert(make_pair(g_abyBoardSlot[nBoardIndex], BoardInfo));
	}

	if (g_mapValidBoard != mapBoard)
	{
		SetMemoryShared();
		g_MainFunction.Reset();
		for (auto& Board : g_mapValidBoard)
		{
			g_MainFunction.AddBoard(Board.first, Board.second.m_usChannelCount);
			if (0 == Board.second.m_usChannelCount)
			{
				Board.second.m_usChannelCount = g_MainFunction.GetChannelCount(Board.first);
			}
		}
		for (auto& Instance : g_mapMain)
		{
			Instance.second->Reset();
			Instance.second->CopyBoard(g_MainFunction);
		}
	}
	return 0;
}

int APIENTRY DCM400_GetSlotNo(BYTE byBoardNo)
{
	if (g_byBoardCount <= byBoardNo)
	{
		return -1;
	}
	return g_abyBoardSlot[byBoardNo];
}