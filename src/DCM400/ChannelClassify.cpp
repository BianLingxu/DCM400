#include "pch.h"
#include "ChannelClassify.h"
#include "DCM400HardwareInfo.h"

using namespace std;
CControllerChannel::CControllerChannel(BYTE byIndex)
{
	m_byIndex = byIndex;
}

CControllerChannel::~CControllerChannel()
{
	m_setChannel.clear();
}

int CControllerChannel::AddChannel(USHORT usChannel)
{
	if (m_setChannel.end() != m_setChannel.find(usChannel))
	{
		return -1;
	}
	m_setChannel.insert(usChannel);
	return 0;
}

void CControllerChannel::GetChannel(std::vector<USHORT>& vecChannel)
{
	vecChannel.clear();
	copy(m_setChannel.begin(), m_setChannel.end(), back_inserter(vecChannel));
}

int CControllerChannel::GetChannelCount()
{
	return m_setChannel.size();
}

void CControllerChannel::Reset()
{
	m_setChannel.clear();
}

CBoardChannelClassify::CBoardChannelClassify()
{
}

CBoardChannelClassify::~CBoardChannelClassify()
{
	for (auto& Channel : m_mapControllerChannel)
	{
		if (nullptr != Channel.second)
		{
			delete Channel.second;
			Channel.second = nullptr;
		}
	}
	m_mapControllerChannel.clear();
}

int CBoardChannelClassify::SetChannel(const std::vector<USHORT>& vecChannel)
{
	auto iterChannel = m_mapControllerChannel.begin();
	while (m_mapControllerChannel.end() != iterChannel)
	{
		if (nullptr == iterChannel->second)
		{
			m_mapControllerChannel.erase(iterChannel);
			iterChannel = m_mapControllerChannel.begin();
			continue;
		}
		iterChannel->second->Reset();
		++iterChannel;
	}
	BOOL bChannelOverRange = FALSE;
	BYTE byCurController = 0;
	USHORT uChannelCount = vecChannel.size();
	for (auto usChannel : vecChannel)
	{
		if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
		{
			bChannelOverRange = TRUE;
			continue;
		}
		byCurController = usChannel / DCM400_CHANNELS_PER_CONTROL;
		iterChannel = m_mapControllerChannel.find(byCurController);
		if (m_mapControllerChannel.end() == iterChannel)
		{
			CControllerChannel* pChannel = new CControllerChannel(byCurController);
			m_mapControllerChannel.insert(make_pair(byCurController, pChannel));
			iterChannel = m_mapControllerChannel.find(byCurController);
		}
		iterChannel->second->AddChannel(usChannel % DCM400_CHANNELS_PER_CONTROL);
	}
	if (bChannelOverRange)
	{
		return -1;
	}
	return 0;
}

int CBoardChannelClassify::GetChannel(BYTE byControllerIndex, std::vector<USHORT>& vecChannel)
{
	vecChannel.clear();
	if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
	{
		return -1;
	}
	auto iterControllerChannel = m_mapControllerChannel.find(byControllerIndex);
	if (m_mapControllerChannel.end() == iterControllerChannel || nullptr == iterControllerChannel->second)
	{
		return 0;
	}
	iterControllerChannel->second->GetChannel(vecChannel);
	return 0;
}

void CBoardChannelClassify::GetController(const std::vector<USHORT>& vecChannel, std::set<BYTE>& setController)
{
	setController.clear();
	for (USHORT usChannel : vecChannel)
	{
		BYTE byController = usChannel / DCM400_CHANNELS_PER_CONTROL;
		setController.insert(byController);
		if (DCM400_MAX_CONTROLLERS_PRE_BOARD == setController.size())
		{
			break;
		}
	}
}

CBoardChannel::CBoardChannel(BYTE bySlotNo)
{
	m_bySlotNo = bySlotNo;
}

CBoardChannel::~CBoardChannel()
{
	m_mapChannel.clear();
}

void CBoardChannel::GetChannel(std::vector<USHORT>& vecChannel, std::vector<USHORT>* pvecChannelID)
{
	vecChannel.clear();
	if (nullptr != pvecChannelID)
	{
		pvecChannelID->clear();
	}
	for (auto& Channel : m_mapChannel)
	{
		vecChannel.push_back(Channel.first);
		if (nullptr != pvecChannelID)
		{
			pvecChannelID->push_back(Channel.second);
		}
	}
}

int CBoardChannel::AddChannel(USHORT usChannel, USHORT usID)
{
	if (m_mapChannel.end() != m_mapChannel.find(usChannel))
	{
		return -1;
	}
	m_mapChannel.insert(make_pair(usChannel, usID));
	return 0;
}

void CBoardChannel::Reset()
{
	m_mapChannel.clear();
}

USHORT CBoardChannel::GetChannelCount()
{
	return m_mapChannel.size();
}

CClassifyBoard::~CClassifyBoard()
{
	for (auto& BoardChannel : m_mapBoardChannel)
	{
		if (nullptr != BoardChannel.second)
		{
			delete BoardChannel.second;
			BoardChannel.second = nullptr;
		}
	}
	m_mapBoardChannel.clear();
}

void CClassifyBoard::SetChannel(const std::vector<CHANNEL_INFO>& vecChannel)
{
	///<Clear all board channel
	for (auto& Board : m_mapBoardChannel)
	{
		if (nullptr != Board.second)
		{
			Board.second->Reset();
		}
	}

	for (auto& Channel : vecChannel)
	{
		auto iterBoard = m_mapBoardChannel.find(Channel.m_bySlotNo);
		if (m_mapBoardChannel.end() == iterBoard)
		{
			CBoardChannel* pBoardChannel = new CBoardChannel(Channel.m_bySlotNo);
			m_mapBoardChannel.insert(make_pair(Channel.m_bySlotNo, pBoardChannel));
			iterBoard = m_mapBoardChannel.find(Channel.m_bySlotNo);
		}
		iterBoard->second->AddChannel(Channel.m_usChannel, Channel.m_usChannelID);
	}
}

void CClassifyBoard::GetBoard(std::set<BYTE>& setBoard) const
{
	setBoard.clear();
	for (auto& Board : m_mapBoardChannel)
	{
		if (0 != Board.second->GetChannelCount())
		{
			setBoard.insert(Board.first);
		}
	}
}

void CClassifyBoard::GetBoardChannel(BYTE bySlotNo, std::vector<USHORT>& vecChannel, std::vector<USHORT>* pvecChannelID) const
{
	vecChannel.clear();
	if (nullptr != pvecChannelID)
	{
		pvecChannelID->clear();
	}
	auto iterBoard = m_mapBoardChannel.find(bySlotNo);
	if (m_mapBoardChannel.end() == iterBoard || nullptr == iterBoard->second)
	{
		return;
	}
	iterBoard->second->GetChannel(vecChannel, pvecChannelID);
}

int CClassifyBoard::GetControllerChannel(USHORT usChannel, BYTE& byController) const
{
	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
	{
		return -1;
	}
	byController = usChannel / DCM400_CHANNELS_PER_CONTROL;
	return usChannel % DCM400_CHANNELS_PER_CONTROL;
}

CChannelGroup::CChannelGroup()
{

}

int CChannelGroup::AddChannel(USHORT usChannel)
{
	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
	{
		return -1;
	}
	m_setChannel.insert(usChannel);
	return 0;
}

void CChannelGroup::GetChannel(std::vector<USHORT>& vecChannel) const
{
	copy(m_setChannel.begin(), m_setChannel.end(), back_inserter(vecChannel));
}
