#include "pch.h"
#include "Pin.h"
#include <iterator>

CPin::CPin(const char* lpszName, UINT uID)
{
	if (nullptr != lpszName)
	{
		m_strName = lpszName;
	}
	m_uID = uID;
}

CPin::CPin(const CPin& Pin)
{
	m_strName = Pin.m_strName;
	m_uID = Pin.m_uID;
	m_vecChannel = Pin.m_vecChannel;
}

void CPin::AddChannel(CHANNEL_INFO ChannelInfo)
{
	m_vecChannel.push_back(ChannelInfo);
}

int CPin::GetChannel(USHORT usSiteNo, CHANNEL_INFO& Channel)
{
	if (m_vecChannel.size() <= usSiteNo)
	{
		return -1;
	}
	Channel = m_vecChannel[usSiteNo];
	return 0;
}

int CPin::GetSiteCount()
{
	return m_vecChannel.size();
}

UINT CPin::GetID()
{
	return m_uID;
}

const char* CPin::GetName()
{
	return m_strName.c_str();
}

int CPin::GetSiteNo(const CHANNEL_INFO& Channel)
{
	USHORT usSiteNo = 0;
	for (const auto& Site :m_vecChannel)
	{
		if (Channel.m_bySlotNo == Site.m_bySlotNo && Channel.m_usChannel == Site.m_usChannel)
		{
			return usSiteNo;
		}
		++usSiteNo;
	}
	return -1;
}

void CPin::GetAllChannel(std::vector<CHANNEL_INFO>& vecChannel, BOOL bAppend /*= FALSE*/)
{
	if (bAppend)
	{
		copy(m_vecChannel.begin(), m_vecChannel.end(), back_inserter(vecChannel));
	}
	else
	{
		vecChannel = m_vecChannel;
	}
}
