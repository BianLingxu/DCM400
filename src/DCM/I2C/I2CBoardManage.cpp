#include "I2CBoardManage.h"
#include "..\RunAuthorization.h"
#ifdef RECORD_TIME
#include "..\Timer.h"
#endif // RECORD_TIME

using namespace std;

CI2CBoardManage::CI2CBoardManage()
	: m_pSite(nullptr)
	, m_bEnableCompareShield(TRUE)
{

}
CI2CBoardManage::~CI2CBoardManage()
{
	Reset();
}


void CI2CBoardManage::SetSite(const CI2CSite& Site)
{
	m_pSite = &Site;
	for (auto& Board : m_mapBoard)
	{
		if (nullptr != Board.second)
		{
			Board.second->SetSite(*m_pSite);
		}
	}
}

CI2CBoard* CI2CBoardManage::GetBoard(BYTE bySlotNo, CDriverAlarm* pAlarm)
{
	if (m_setBoardExisted.end() == m_setBoardExisted.find(bySlotNo))
	{
		return nullptr;
	}
	auto iterBoard = m_mapBoard.find(bySlotNo);
	if (m_mapBoard.end() != iterBoard)
	{
		return iterBoard->second;
	}

	CI2CBoard* pI2CBoard = new CI2CBoard(bySlotNo, pAlarm);
	if (nullptr != m_pSite)
	{
		pI2CBoard->SetSite(*m_pSite);
	}
	if (!pI2CBoard->IsBoardExist(TRUE))
	{
		delete pI2CBoard;
		pI2CBoard = nullptr;
		return nullptr;
	}
	m_mapBoard.insert(make_pair(bySlotNo, pI2CBoard));
	return pI2CBoard;
}

void CI2CBoardManage::Run(UINT uStartLine, UINT uStopLine, BOOL bWithDRAM /*= FALSE*/, UINT uDRAMStartLine /*= 0*/)
{
#ifdef RECORD_TIME
	CTimer::Instance()->Start("CI2CBoardManage::Run");
	CTimer::Instance()->Start("SetRun");
#endif // RECORD_TIME
	int nRetVal = 0;
	vector<CI2CBoard*> vecValidBoard;
	if (0 == m_mapBoard.size())
	{
		return;
	}
	for (auto& Board : m_mapBoard)
	{
		Board.second->EnableCopmareShield(m_bEnableCompareShield);
		nRetVal = Board.second->SetRunParameter(uStartLine, uStopLine, bWithDRAM, uDRAMStartLine);
		if (0 != nRetVal)
		{
			continue;
		}
		vecValidBoard.push_back(Board.second);
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Apply");
#endif // RECORD_TIME
	CRunAuthorization* pRunAuthorization = CRunAuthorization::Instance();
	pRunAuthorization->Apply();

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("EnableRun");
#endif // RECORD_TIME

	for (auto& Board : vecValidBoard)
	{
		Board->EnableRun(TRUE);
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("SynRun");
#endif // RECORD_TIME

	if (0 != vecValidBoard.size())
	{
		vecValidBoard[0]->Run();
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("DisableRun");
#endif // RECORD_TIME

	for (auto& Board : vecValidBoard)
	{
		Board->EnableRun(FALSE);
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("Release");
#endif // RECORD_TIME

	pRunAuthorization->Release();

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Start("WaitStop");
#endif // RECORD_TIME

	for (auto& Board : vecValidBoard)
	{
		Board->WaitStop();
	}


#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
	CTimer::Instance()->Stop();
#endif // RECORD_TIME
}

void CI2CBoardManage::Reset()
{
	if (0 != m_mapBoard.size())
	{
		for (auto& Board : m_mapBoard)
		{
			if (nullptr != Board.second)
			{
				delete Board.second;
				Board.second = nullptr;
			}
		}
		m_mapBoard.clear();
	}
	m_pSite = nullptr;
}

void CI2CBoardManage::SetValidBoard(const std::vector<BYTE>& vecBoard)
{
	m_setBoardExisted.clear();

	for (auto Board : vecBoard)
	{
		m_setBoardExisted.insert(Board);
	}

	if (0 == m_mapBoard.size())
	{
		return;
	}

	auto iterBoard = m_mapBoard.begin();
	while (m_mapBoard.end() != iterBoard)
	{
		if (m_setBoardExisted.end() == m_setBoardExisted.find(iterBoard->first))
		{
			m_mapBoard.erase(iterBoard);
			iterBoard = m_mapBoard.begin();
			continue;
		}
		++iterBoard;
	}
}

void CI2CBoardManage::EnableCopmareShield(BOOL bEnable)
{
	m_bEnableCompareShield = bEnable;
}
