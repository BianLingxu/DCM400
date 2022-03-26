#include "I2CSite.h"
#include "Sts8100.h"
#include "ATSite_No.h"
using namespace std;
CI2CSite::CI2CSite()
	: m_pbySiteStatus(nullptr)
{
	m_usSiteCount = 0;
}

CI2CSite::~CI2CSite()
{
	Reset();
}

void CI2CSite::SetSiteCount(USHORT usSiteCount)
{
	if (m_usSiteCount != usSiteCount && nullptr != m_pbySiteStatus)
	{
		delete[] m_pbySiteStatus;
		m_pbySiteStatus = nullptr;
	}
	m_usSiteCount = usSiteCount;
	if (nullptr == m_pbySiteStatus)
	{
		m_pbySiteStatus = new BYTE[m_usSiteCount];
		memset(m_pbySiteStatus, 0, m_usSiteCount * sizeof(BYTE));
	}
}

int CI2CSite::SetSiteChannel(USHORT usSiteNo, const CHANNEL_INFO& SCL, const CHANNEL_INFO& SDA)
{
	if (m_usSiteCount <= usSiteNo)
	{
		return -1;
	}
	auto iterSite = m_mapSite.find(usSiteNo);
	if (m_mapSite.end() == iterSite)
	{
		I2CSITE I2CSite;
		I2CSite.m_SCL = SCL;
		I2CSite.m_SDA = SDA;
		m_mapSite.insert(make_pair(usSiteNo, I2CSite));
		SiteBoardRelation();
		m_mapStopStatus.insert(make_pair(usSiteNo, false));
		return 0;
	}
	I2CSITE* pSite = nullptr;
	pSite = &iterSite->second;
	if (pSite->m_SCL.m_bySlotNo != SCL.m_bySlotNo || pSite->m_SCL.m_usChannel != SCL.m_usChannel)
	{
		return -2;
	}
	else if (pSite->m_SDA.m_bySlotNo != SDA.m_bySlotNo || pSite->m_SDA.m_usChannel != SCL.m_usChannel)
	{
		return -3;
	}


	return 0;
}

int CI2CSite::GetSiteChannel(USHORT usSiteNo, CHANNEL_INFO& SCL, CHANNEL_INFO& SDA) const
{
	if (m_usSiteCount <= usSiteNo)
	{
		return -1;
	}
	auto iterSite = m_mapSite.find(usSiteNo);
	if (m_mapSite.end() == iterSite)
	{
		return -2;
	}
	SCL = iterSite->second.m_SCL;
	SDA = iterSite->second.m_SDA;
	return 0;
}

void CI2CSite::GetDataChannel(std::map<USHORT, CHANNEL_INFO>& mapChannel, BOOL bOnlyValidSite) const
{
	mapChannel.clear();
	GetSiteStatus(m_pbySiteStatus, m_usSiteCount);
	for (auto& Site : m_mapSite)
	{
		if (!bOnlyValidSite || 0 != m_pbySiteStatus[Site.first])
		{
			mapChannel.insert(make_pair(Site.first, Site.second.m_SDA));
		}
	}
}

int CI2CSite::GetDataChannel(USHORT usSiteNo, CHANNEL_INFO& Channel, BOOL bOnlyValidSite) const
{
	auto iterSite = m_mapSite.find(usSiteNo);
	if (m_mapSite.end() == iterSite)
	{
		return -1;
	}
	GetSiteStatus(m_pbySiteStatus, m_usSiteCount);
	if (bOnlyValidSite && 0 == m_pbySiteStatus[usSiteNo])
	{
		return -2;
	}
	Channel = iterSite->second.m_SDA;
	return 0;
}

void CI2CSite::GetUseBoard(std::vector<BYTE>& vecUseBoard, BOOL bOnlyValidSite) const
{
	vecUseBoard.clear();

	vector<USHORT> vecInvalidSite;
	if (bOnlyValidSite)
	{
		GetSiteStatus(m_pbySiteStatus, m_usSiteCount);
		for (auto& Site : m_mapSite)
		{
			if (0 == m_pbySiteStatus[Site.first])
			{
				vecInvalidSite.push_back(Site.first);
			}
		}
	}

	for (auto& SiteBoardRelation : m_mapSiteBoardRelation)
	{
		if (bOnlyValidSite && 0 == SiteBoardRelation.second->GetExcludeSiteCount(vecInvalidSite))
		{
			continue;
		}
		vecUseBoard.push_back(SiteBoardRelation.first);
	}
}

int CI2CSite::GetBoardSite(BYTE bySlotNo, std::map<USHORT, BYTE>& mapSiteInfo)
{
	mapSiteInfo.clear();
	auto iterSiteBoardRelation = m_mapSiteBoardRelation.find(bySlotNo);
	if (m_mapSiteBoardRelation.end() == iterSiteBoardRelation)
	{
		return -1;
	}
	CBoardSite* pBoardSite = nullptr;;
	pBoardSite = iterSiteBoardRelation->second;
	pBoardSite->GetOnBoardSite(mapSiteInfo);
	return 0;
}

int CI2CSite::GetBoardChannel(BYTE bySlotNo, std::vector<USHORT>& vecChannel, BOOL bOnlyValidSite, BYTE byChannelType) const
{
	vecChannel.clear();
	auto iterSiteBoardRelation = m_mapSiteBoardRelation.find(bySlotNo);
	if (m_mapSiteBoardRelation.end() == iterSiteBoardRelation)
	{
		return -1;
	}
	if (bOnlyValidSite)
	{
		GetSiteStatus(m_pbySiteStatus, m_usSiteCount);
	}
	else
	{
		memset(m_pbySiteStatus, 1, m_usSiteCount);
	}
	BOOL bAddSCL = byChannelType & 0x01;
	BOOL bAddSDA = byChannelType >> 1 & 0x01;
	CBoardSite* pBoardSite = nullptr;;
	pBoardSite = iterSiteBoardRelation->second;
	map<USHORT, BYTE> mapBoardSite;
	pBoardSite->GetOnBoardSite(mapBoardSite);
	for (auto& BoardSite : mapBoardSite)
	{
		auto iterSite = m_mapSite.find(BoardSite.first);
		if (bOnlyValidSite && 0 == m_pbySiteStatus[BoardSite.first])
		{
			continue;
		}
		if (m_mapSite.end() != iterSite)
		{
			if (bAddSCL && (BoardSite.second & 0x01))
			{
				vecChannel.push_back(iterSite->second.m_SCL.m_usChannel);
			}
			if (bAddSDA && ((BoardSite.second >> 1) & 0x01))
			{
				vecChannel.push_back(iterSite->second.m_SDA.m_usChannel);
			}
		}
	}
	return 0;
}

void CI2CSite::GetValidSite(std::vector<USHORT>& vecSite) const
{
	vecSite.clear();
	GetSiteStatus(m_pbySiteStatus, m_usSiteCount);
	for (USHORT usSiteNo = 0; usSiteNo < m_usSiteCount;++usSiteNo)
	{
		if (m_usSiteCount <= usSiteNo)
		{
			break;
		}
		if (0 != m_pbySiteStatus[usSiteNo])
		{
			vecSite.push_back(usSiteNo);
		}
	}
}

void CI2CSite::Reset()
{
	m_mapSite.clear();
	for (auto& SiteBoard : m_mapSiteBoardRelation)
	{
		if (nullptr != SiteBoard.second)
		{
			delete SiteBoard.second;
			SiteBoard.second = nullptr;
		}
	}
	m_mapSiteBoardRelation.clear();
	m_mapStopStatus.clear();
	SetSiteCount(0);
}

int CI2CSite::GetSiteCount() const
{
	return m_usSiteCount;
}

BOOL CI2CSite::IsSiteValid(USHORT usSiteNo) const
{
	if (m_usSiteCount <= usSiteNo)
	{
		return FALSE;
	}
	GetSiteStatus(m_pbySiteStatus, m_usSiteCount);
	if (0 != m_pbySiteStatus[usSiteNo])
	{
		return TRUE;
	}
	return FALSE;
}

void CI2CSite::GetChannel(BOOL bSCL, std::map<BYTE, std::vector<USHORT>>& mapChannel, BOOL bOnlyValidSite)
{
	mapChannel.clear();
	auto iterChannel = mapChannel.begin();
	CHANNEL_INFO* pChannel = nullptr;
	for (auto& Site : m_mapSite)
	{
		if (bOnlyValidSite && !IsSiteValid(Site.first))
		{
			continue;
		}
		if (bSCL)
		{
			pChannel = &Site.second.m_SCL;
		}
		else
		{
			pChannel = &Site.second.m_SDA;
		}
		iterChannel = mapChannel.find(pChannel->m_bySlotNo);
		if (mapChannel.end() == iterChannel)
		{
			vector<USHORT> vecChannel;
			mapChannel.insert(make_pair(pChannel->m_bySlotNo, vecChannel));
			iterChannel = mapChannel.find(pChannel->m_bySlotNo);
		}
		iterChannel->second.push_back(pChannel->m_usChannel);
	}
}

void CI2CSite::GetInvalidSiteChannel(BYTE bySlotNo, std::vector<USHORT>& vecChannel) const
{
	GetSiteStatus(m_pbySiteStatus, m_usSiteCount);
	for (auto& Site : m_mapSite)
	{
		if (0 == m_pbySiteStatus[Site.first])
		{
			if (bySlotNo == Site.second.m_SCL.m_bySlotNo)
			{
				vecChannel.push_back(Site.second.m_SCL.m_usChannel);
			}
			if (bySlotNo == Site.second.m_SDA.m_bySlotNo)
			{
				vecChannel.push_back(Site.second.m_SDA.m_usChannel);
			}
		}
	}
}

void CI2CSite::SetSiteStopStatus(BOOL bHighImpedance)
{
	GetSiteStatus(m_pbySiteStatus, m_usSiteCount);
	for (auto& Site : m_mapStopStatus)
	{
		if (0 != m_pbySiteStatus[Site.first])
		{
			Site.second = bHighImpedance;
		}
	}
}

BOOL CI2CSite::GetSiteStopStatus(USHORT usSiteNo) const
{
	auto iterStopStatus = m_mapStopStatus.find(usSiteNo);
	if (m_mapStopStatus.end() == iterStopStatus)
	{
		return FALSE;
	}
	return iterStopStatus->second;
}

void CI2CSite::GetAllChannel(vector<CHANNEL_INFO>& vecChannel) const
{
	vecChannel.clear();
	for (auto& Site : m_mapSite)
	{
		vecChannel.push_back(Site.second.m_SCL);
		vecChannel.push_back(Site.second.m_SDA);
	}
}

void CI2CSite::SiteBoardRelation()
{
	auto iterSiteBoardRelation = m_mapSiteBoardRelation.begin();
	I2CSITE* pSite = nullptr;
	const CHANNEL_INFO* pChannel = nullptr;
	BOOL bSCL = TRUE;
	for (auto& Site : m_mapSite)
	{
		pSite = &Site.second;
		for (BYTE byIndex = 0; byIndex < 2; ++byIndex)
		{
			if (0 == byIndex)
			{
				bSCL = TRUE;
				pChannel = &pSite->m_SCL;
			}
			else
			{
				bSCL = FALSE;
				pChannel = &pSite->m_SDA;
			}
			iterSiteBoardRelation = m_mapSiteBoardRelation.find(pChannel->m_bySlotNo);
			if (m_mapSiteBoardRelation.end() == iterSiteBoardRelation)
			{
				CBoardSite* pBoardSite = new CBoardSite(pChannel->m_bySlotNo);
				m_mapSiteBoardRelation.insert(make_pair(pChannel->m_bySlotNo, pBoardSite));
				iterSiteBoardRelation = m_mapSiteBoardRelation.find(pChannel->m_bySlotNo);
			}
			iterSiteBoardRelation->second->AddChannel(Site.first, bSCL);
		}
	}
}

void CI2CSite::GetSiteStatus(BYTE* pbySiteStatus, USHORT usSiteCount) const
{
	AT_StsGetSitesStatus(pbySiteStatus, usSiteCount);
}

CBoardSite::CBoardSite(BYTE bySlotNo)
{
	m_bySlotNo = bySlotNo;
}

void CBoardSite::AddChannel(USHORT usSiteNo, BOOL bSCL)
{
	auto iterSite = m_mapOnBoardSite.find(usSiteNo);
	if (m_mapOnBoardSite.end() == iterSite)
	{
		m_mapOnBoardSite.insert(make_pair(usSiteNo, 0));
		iterSite = m_mapOnBoardSite.find(usSiteNo);
	}
	BYTE byShif = 1;
	if (bSCL)
	{
		byShif = 0;
	}
	iterSite->second |= 1 << byShif;
}

USHORT CBoardSite::GetExcludeSiteCount(const std::vector<USHORT>& vecExcludeSite)
{
	if (0 == vecExcludeSite.size())
	{
		return m_mapOnBoardSite.size();
	}
	int nSiteCount = m_mapOnBoardSite.size();
	for (auto Site : vecExcludeSite)
	{
		if (m_mapOnBoardSite.end() != m_mapOnBoardSite.find(Site))
		{
			--nSiteCount;
		}
		if (0 >= nSiteCount)
		{
			break;
		}
	}
	return nSiteCount;
}

void CBoardSite::GetOnBoardSite(std::map<USHORT, BYTE>& mapOnBoard)
{
	mapOnBoard = m_mapOnBoardSite;
}