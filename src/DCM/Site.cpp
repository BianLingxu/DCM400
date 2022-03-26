#include "Site.h"
#include "Sts8100.h"
#include <set>
#include <iterator>
using namespace std;
CSiteChannel::CSiteChannel(USHORT usSiteNo)
{
	m_usSiteNo = usSiteNo;
}

int CSiteChannel::AddChannel(const std::string& strPinName, CHANNEL_INFO& Channel)
{
	auto iterChannel = m_mapChannel.find(strPinName);
	if (m_mapChannel.end() != iterChannel)
	{
		return -1;
	}
	m_mapChannel.insert(make_pair(strPinName, Channel));
	return 0;
}

int CSiteChannel::GetChannel(const std::string& strPinName, CHANNEL_INFO& Channel) const
{
	auto iterChannel = m_mapChannel.find(strPinName);
	if (m_mapChannel.end() == iterChannel)
	{
		return -1;
	}
	Channel = iterChannel->second;
	return 0;
}

void CSiteChannel::GetChannel(std::vector<CHANNEL_INFO>& vecChannel, BOOL bAppend)
{
	if (!bAppend)
	{
		vecChannel.clear();
	}
	for (auto& Channel : m_mapChannel)
	{
		vecChannel.push_back(Channel.second);
	}
}

void CSiteChannel::GetSlot(std::vector<BYTE>& vecSlot) const
{
	vecSlot.clear();
	BYTE bySlot = 0;
	BOOL bFindSlot = FALSE;
	for (auto& Channel : m_mapChannel)
	{
		bFindSlot = FALSE;
		int nSlotCount = vecSlot.size();
		BYTE bySlot = Channel.second.m_bySlotNo;
		for (int nCurSlot : vecSlot)
		{
			if (nCurSlot == bySlot)
			{
				bFindSlot = TRUE;
				break;
			}
		}
		if (!bFindSlot)
		{
			vecSlot.push_back(bySlot);
		}
	}
	return;
}

int CSiteChannel::GetPinName(const CHANNEL_INFO& Channel, std::string& strPinName) const
{
	strPinName.clear();
	for (auto& Pin : m_mapChannel)
	{
		if (Pin.second.m_bySlotNo == Channel.m_bySlotNo && Pin.second.m_usChannel == Channel.m_usChannel)
		{
			strPinName = Pin.first;
			return 0;
		}
	}
	return -1;
}

void CSiteChannel::GetSiteBoard(std::map<BYTE, USHORT>& mapBoard) const
{
	mapBoard.clear();
	auto iterBoard = mapBoard.begin();
	for (auto& Channel : m_mapChannel)
	{
		iterBoard = mapBoard.find(Channel.second.m_bySlotNo);
		if (mapBoard.end() == iterBoard)
		{
			mapBoard.insert(make_pair(Channel.second.m_bySlotNo, 0));
			iterBoard = mapBoard.find(Channel.second.m_bySlotNo);
		}
		if (iterBoard->second < Channel.second.m_usChannel + 1)
		{
			iterBoard->second = Channel.second.m_usChannel + 1;
		}
	}
}

BOOL CSiteChannel::UseBoard(BYTE bySlotNo) const
{
	for (auto& Channel : m_mapChannel)
	{
		if (bySlotNo == Channel.second.m_bySlotNo)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void CSiteChannel::DeletePin(const std::string& strPinName)
{
	auto iterPin = m_mapChannel.find(strPinName);
	if (m_mapChannel.end() != iterPin)
	{
		m_mapChannel.erase(iterPin);
	}
}

CSite::CSite()
	: m_pbySiteStatus(nullptr)
{
}

CSite& CSite::operator=(const CSite& Site)
{
	Reset();
	CSiteChannel* pSiteChannel = nullptr;
	for (auto& CurSite : Site.m_mapSite)
	{
		pSiteChannel = new CSiteChannel(*CurSite.second);
		m_mapSite.insert(make_pair(CurSite.first, new CSiteChannel(*CurSite.second)));
	}
	return *this;
}

CSite::~CSite()
{
	Reset();
}

int CSite::AddChannel(const std::string& strPinName, USHORT usSiteNo, CHANNEL_INFO& Channel)
{
	if (0 == strPinName.size())
	{
		return -1;
	}
	auto iterSite = m_mapSite.find(usSiteNo);
	if (m_mapSite.end() == iterSite)
	{
		CSiteChannel* pSiteData = new CSiteChannel(usSiteNo);
		m_mapSite.insert(make_pair(usSiteNo, pSiteData));
		iterSite = m_mapSite.find(usSiteNo);
	}
	int nRetVal = iterSite->second->AddChannel(strPinName, Channel);
	if (0 != nRetVal)
	{
		nRetVal = -2;
	}
	return nRetVal;
}

int CSite::GetChannel(const std::string& strPinName, USHORT usSiteNo, CHANNEL_INFO& Channel) const
{
	auto iterSite = m_mapSite.find(usSiteNo);
	if (m_mapSite.end() == iterSite)
	{
		return -2;
	}
	int nRetVal = iterSite->second->GetChannel(strPinName, Channel);
	if (0 != nRetVal)
	{
		nRetVal = -1;
	}
	return nRetVal;
}

int CSite::GetChannel(USHORT usSiteNo, std::vector<CHANNEL_INFO>& vecChannel)
{
	auto iterSite = m_mapSite.find(usSiteNo);
	if (m_mapSite.end() == iterSite)
	{
		return -1;
	}
	iterSite->second->GetChannel(vecChannel);
	return 0;
}

USHORT CSite::GetSiteCount()
{
	return m_mapSite.size();
}

void CSite::GetUseBoard(std::vector<BYTE>& vecBoard) const
{
	vecBoard.clear();
	BYTE byBoardCount = 0;
	set<BYTE> setBoard;
	auto iterBoard = setBoard.begin();
	for (auto& Site : m_mapSite)
	{
		Site.second->GetSlot(vecBoard);
		for (BYTE byBoard : vecBoard)
		{
			iterBoard = setBoard.find(byBoard);
			if (setBoard.end() != iterBoard)
			{
				continue;
			}
			setBoard.insert(byBoard);
		}
	}
	vecBoard.clear();
	copy(setBoard.begin(), setBoard.end(), back_inserter(vecBoard));
}

BOOL CSite::IsSiteValid(USHORT usSiteNo)
{
	auto iterSite = m_mapSite.find(usSiteNo);
	if (m_mapSite.end() == iterSite)
	{
		return FALSE;
	}
	if (nullptr == m_pbySiteStatus)
	{
		USHORT usSiteCout = m_mapSite.size();
		m_pbySiteStatus = new BYTE[usSiteCout];
		memset(m_pbySiteStatus, 0, usSiteCout * sizeof(BYTE));
	}
	GetSiteStatus(m_pbySiteStatus, m_mapSite.size());
	if (0 != m_pbySiteStatus[usSiteNo])
	{
		return TRUE;
	}
	return FALSE;
}

void CSite::Reset()
{
	if (0 != m_mapSite.size())
	{
		for (auto& Site : m_mapSite)
		{
			if (nullptr != Site.second)
			{
				delete Site.second;
				Site.second = nullptr;
			}
		}
		m_mapSite.clear();
	}
	if (nullptr != m_pbySiteStatus)
	{
		delete[] m_pbySiteStatus;
		m_pbySiteStatus = nullptr;
	}
}

int CSite::GetPinName(USHORT usSiteNo, const CHANNEL_INFO& Channel, std::string& strPinName)
{
	auto iterSite = m_mapSite.find(usSiteNo);
	if (m_mapSite.end() == iterSite)
	{
		return -1;
	}
	int nRetVal = iterSite->second->GetPinName(Channel, strPinName);
	if (0 != nRetVal)
	{
		nRetVal = -2;
	}
	return nRetVal;
}

void CSite::DeletePin(const std::string& strPin)
{
	for (auto& Site : m_mapSite)
	{
		Site.second->DeletePin(strPin);
	}
}

int CSite::GetSiteBoard(USHORT usSiteNo, std::map<BYTE, USHORT>& mapBoard)
{
	mapBoard.clear();
	auto iterSite = m_mapSite.find(usSiteNo);
	if (m_mapSite.end() == iterSite || nullptr == iterSite->second)
	{
		return -1;
	}
	iterSite->second->GetSiteBoard(mapBoard);
	return 0;
}

void CSite::GetBoardSite(BYTE bySlotNo, std::vector<USHORT>& vecSite) const
{
	vecSite.clear();
	for (auto& Site : m_mapSite)
	{
		if (Site.second->UseBoard(bySlotNo))
		{
			vecSite.push_back(Site.first);
		}
	}
}

void CSite::GetInvalidSiteChannel(std::vector<CHANNEL_INFO>& vecChannel)
{
	vecChannel.clear();
	if (nullptr == m_pbySiteStatus)
	{
		USHORT usSiteCout = m_mapSite.size();
		m_pbySiteStatus = new BYTE[usSiteCout];
		memset(m_pbySiteStatus, 0, usSiteCout * sizeof(BYTE));
	}
	GetSiteStatus(m_pbySiteStatus, m_mapSite.size());
	for (auto& Site : m_mapSite)
	{
		if (0 == m_pbySiteStatus[Site.first])
		{
			Site.second->GetChannel(vecChannel, TRUE);
		}
	}
}

void CSite::GetValidSiteChannel(std::vector<CHANNEL_INFO>& vecChannel)
{
	vecChannel.clear(); 
	if (nullptr == m_pbySiteStatus)
	{
		USHORT usSiteCout = m_mapSite.size();
		m_pbySiteStatus = new BYTE[usSiteCout];
		memset(m_pbySiteStatus, 0, usSiteCout * sizeof(BYTE));
	}
	GetSiteStatus(m_pbySiteStatus, m_mapSite.size());
	for (auto& Site : m_mapSite)
	{
		if (0 == m_pbySiteStatus[Site.first])
		{
			continue;
		}
		Site.second->GetChannel(vecChannel, TRUE);
	}
}

void CSite::GetSiteStatus(BYTE* pbySiteStatus, USHORT usSiteCount) const
{
	AT_StsGetSitesStatus(pbySiteStatus, usSiteCount);
}
