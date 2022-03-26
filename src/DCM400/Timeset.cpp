#include "pch.h"
#include "Timeset.h"
using namespace std;

CTimeSetSeries::CTimeSetSeries()
	: m_byFormatSeries(0)
	, m_byPeriodSeries(0)
{
	memset(m_abyEdgeSeries, 0, sizeof(m_abyEdgeSeries));
}

CTimeSetSeries::CTimeSetSeries(const CTimeSetSeries& Series)
{
	memcpy_s(m_abyEdgeSeries, sizeof(m_abyEdgeSeries), Series.m_abyEdgeSeries, sizeof(Series.m_abyEdgeSeries));
	m_byFormatSeries = Series.m_byFormatSeries;
	m_byPeriodSeries = Series.m_byPeriodSeries;
}

CTimeSetSeries& CTimeSetSeries::operator=(const CTimeSetSeries& Series)
{
	if (this == &Series)
	{
		return *this;
	}
	memcpy_s(m_abyEdgeSeries, sizeof(m_abyEdgeSeries), Series.m_abyEdgeSeries, sizeof(Series.m_abyEdgeSeries));
	m_byFormatSeries = Series.m_byFormatSeries;
	m_byPeriodSeries = Series.m_byPeriodSeries;
	return *this;
}

int CTimeSetSeries::SetEdgeSeries(const BYTE* pbyEdgeSeries)
{
	for (BYTE byEdgeIndex = 0; byEdgeIndex < EDGE_COUNT;++byEdgeIndex)
	{
		if (TIME_SERIES_MAX_COUNT <= pbyEdgeSeries[byEdgeIndex])
		{
			return -1;
		}
	}
	memcpy_s(m_abyEdgeSeries, sizeof(m_abyEdgeSeries), pbyEdgeSeries, sizeof(m_abyEdgeSeries));
	return 0;
}

const BYTE* CTimeSetSeries::GetEdgeSeries()
{
	return m_abyEdgeSeries;
}

int CTimeSetSeries::SetFormatSeries(BYTE byFormatSeries)
{
	if (TIME_SERIES_MAX_COUNT <= byFormatSeries)
	{
		return -1;
	}
	m_byFormatSeries = byFormatSeries;
	return 0;
}

int CTimeSetSeries::GetFormatSeries()
{
	return m_byFormatSeries;
}

int CTimeSetSeries::SetPeriodSeries(BYTE byPeriodSeries)
{
	if (TIME_SERIES_MAX_COUNT <= byPeriodSeries)
	{
		return -1;
	}
	m_byPeriodSeries = byPeriodSeries;
	return 0;
}

int CTimeSetSeries::GetPeriodSeries()
{
	return m_byPeriodSeries;
}

bool CTimeSetSeries::operator==(const CTimeSetSeries& TimeSetSeries)
{
	if (0 != memcpy_s(m_abyEdgeSeries, sizeof(m_abyEdgeSeries), TimeSetSeries.m_abyEdgeSeries, sizeof(TimeSetSeries.m_abyEdgeSeries)))
	{
		return false;
	}
	if (m_byFormatSeries != TimeSetSeries.m_byFormatSeries || m_byPeriodSeries != TimeSetSeries.m_byPeriodSeries)
	{
		return false;
	}
	return true;
}

CTimeSet::CTimeSet(BYTE byIndex)
	: m_byIndex(byIndex)
{

}

CTimeSet::~CTimeSet()
{
	for (auto& TimeSetChannel : m_mapChannel)
	{
		if (nullptr != TimeSetChannel.second)
		{
			delete TimeSetChannel.second;
			TimeSetChannel.second = nullptr;
		}
	}
	m_mapChannel.clear();
}

int CTimeSet::Index()
{
	return m_byIndex;
}

int CTimeSet::SetChannelSeries(const CHANNEL_INFO& Channel, const CTimeSetSeries& TimesetSeries)
{
	m_vecSameSeries.clear();
	if (DCM400_MAX_CHANNELS_PER_BOARD <= Channel.m_usChannel)
	{
		return -1;
	}
	USHORT usID = GetChannelID(Channel);
	auto iterSeries = m_mapChannel.find(usID);
	if (iterSeries == m_mapChannel.end())
	{
		CTimeSetSeries* pSeries = new CTimeSetSeries(TimesetSeries);
		m_mapChannel.insert(make_pair(usID, pSeries));
		return 0;
	}
	*iterSeries->second = TimesetSeries;
	return 0;
}

int CTimeSet::GetChannelSeries(const CHANNEL_INFO& Channel, CTimeSetSeries& TimesetSeries)
{
	if (DCM400_MAX_CHANNELS_PER_BOARD <= Channel.m_usChannel)
	{
		return -1;
	}
	USHORT usID = GetChannelID(Channel);
	auto iterSeries = m_mapChannel.find(usID);
	if (iterSeries == m_mapChannel.end())
	{
		return -2;
	}
	TimesetSeries = *iterSeries->second;
	return 0;
}

int CTimeSet::GetSameSeriesCount()
{
	m_vecSameSeries.clear();
	for (auto& Channel : m_mapChannel)
	{
		auto AddNewType = [&]()
		{
			vector<USHORT> vecChannel;
			vecChannel.push_back(Channel.first);
			m_vecSameSeries.push_back(vecChannel);
		};
		if (0 == m_vecSameSeries.size())
		{
			AddNewType();
			continue;
		}
		BOOL bFind = FALSE;
		for (auto& Series : m_vecSameSeries)
		{
			auto iterChannel = m_mapChannel.find(Series[0]);
			if (m_mapChannel.end() == iterChannel)
			{
				AddNewType();
				continue;
			}
			if (*Channel.second == *iterChannel->second)
			{
				Series.push_back(Channel.first);
				bFind = TRUE;
				break;
			}
		}
		if (!bFind)
		{
			AddNewType();
		}
	}
	return m_vecSameSeries.size();
}

int CTimeSet::GetSameSeries(int nSameSeriesIndex, std::vector<CHANNEL_INFO>& vecChannel, CTimeSetSeries& TimeSetSeries)
{
	if (m_vecSameSeries.size() <= nSameSeriesIndex)
	{
		return -1;
	}
	vecChannel.clear();
	CHANNEL_INFO Channel;
	USHORT usTargetID = -1;
	for (auto& ID : m_vecSameSeries[nSameSeriesIndex])
	{
		usTargetID = ID;
		GetChannel(ID, Channel);
		vecChannel.push_back(Channel);
	}
	auto iterSeries = m_mapChannel.find(usTargetID);
	if (m_mapChannel.end() == iterSeries)
	{
		return -2;
	}
	TimeSetSeries = *iterSeries->second;
	
	return 0;
}

USHORT CTimeSet::GetChannelID(const CHANNEL_INFO& Channel)
{
	return Channel.m_bySlotNo << 8 | Channel.m_usChannel;
}

void CTimeSet::GetChannel(USHORT usID, CHANNEL_INFO& Channel)
{
	Channel.m_bySlotNo = usID >> 8 & 0xFF;
	Channel.m_usChannel = usID & 0xFF;
	Channel.m_usChannelID = -1;
}

CSeriesValue::CSeriesValue()
	: m_usValidSeriesCount(0)
{
	memset(m_adT1R, 0, sizeof(m_adT1R));
	memset(m_abyFormat, 0, sizeof(m_abyFormat));
	memset(m_adPeriod, 0, sizeof(m_adPeriod));
}

USHORT CSeriesValue::GetSeriesCount()
{
	return m_usValidSeriesCount;
}

int CSeriesValue::SetEdge(USHORT usSeriesIndex, const double* pdEdge)
{
	if (TIME_SERIES_MAX_COUNT <= usSeriesIndex)
	{
		return -1;
	}
	if (nullptr == pdEdge)
	{
		return -2;
	}
	if (m_usValidSeriesCount <= usSeriesIndex)
	{
		m_usValidSeriesCount = usSeriesIndex + 1;
	}
	for (BYTE byEdgeIndex = 0; byEdgeIndex < EDGE_COUNT;++byEdgeIndex)
	{
		m_adT1R[usSeriesIndex][byEdgeIndex] = pdEdge[byEdgeIndex];
	}
	return 0;
}

int CSeriesValue::GetEdge(USHORT usSeriesIndex, double* pdEdge)
{
	if (TIME_SERIES_MAX_COUNT <= usSeriesIndex)
	{
		return -1;
	}
	if (nullptr == pdEdge)
	{
		return -2;
	}
	for (BYTE byEdgeIndex = 0; byEdgeIndex < EDGE_COUNT; ++byEdgeIndex)
	{
		pdEdge[byEdgeIndex] = m_adT1R[usSeriesIndex][byEdgeIndex];
	}
	return 0;
}
#define WAVE_FORMAT_START_BIT 0
#define WAVE_FORMAT_VALID_BIT 0x0F
#define IO_FORMAT_START_BIT 4
#define IO_FORMAT_VALID_BIT 0x03
#define COMPARE_MODE_START_BIT 6
#define COMPARE_MODE_VALID_BIT 0x03
int CSeriesValue::SetFormat(USHORT usSeriesIndex, const WAVE_FORMAT& WaveFormat, const IO_FORMAT& IOFormat, const COMPARE_MODE& CompareMode)
{
	if (TIME_SERIES_MAX_COUNT <= usSeriesIndex)
	{
		return -1;
	}
	if (m_usValidSeriesCount <= usSeriesIndex)
	{
		m_usValidSeriesCount = usSeriesIndex + 1;
	}
	m_abyFormat[usSeriesIndex] = ((BYTE)CompareMode & COMPARE_MODE_VALID_BIT) << COMPARE_MODE_START_BIT;
	m_abyFormat[usSeriesIndex] |= ((BYTE)IOFormat & IO_FORMAT_VALID_BIT) << IO_FORMAT_START_BIT;
	m_abyFormat[usSeriesIndex] |= ((BYTE)WaveFormat & WAVE_FORMAT_VALID_BIT) << WAVE_FORMAT_START_BIT;
	return 0;
}

int CSeriesValue::GetFormat(USHORT usSeriesIndex, WAVE_FORMAT& WaveFormat, IO_FORMAT& IOFormat, COMPARE_MODE& CompareMode)
{
	if (TIME_SERIES_MAX_COUNT <= usSeriesIndex)
	{
		return -1;
	}
	WaveFormat = (WAVE_FORMAT)(m_abyFormat[usSeriesIndex] >> WAVE_FORMAT_START_BIT & WAVE_FORMAT_VALID_BIT);
	IOFormat = (IO_FORMAT)(m_abyFormat[usSeriesIndex] >> IO_FORMAT_START_BIT & IO_FORMAT_VALID_BIT);
	CompareMode = (COMPARE_MODE)(m_abyFormat[usSeriesIndex] >> COMPARE_MODE_START_BIT & COMPARE_MODE_VALID_BIT);
	return 0;
}

int CSeriesValue::SetPeriod(USHORT usSeriesIndex, double dPeriod)
{
	if (TIME_SERIES_MAX_COUNT <= usSeriesIndex)
	{
		return -1;
	}
	if (m_usValidSeriesCount <= usSeriesIndex)
	{
		m_usValidSeriesCount = usSeriesIndex + 1;
	}
	m_adPeriod[usSeriesIndex] = dPeriod;
	return 0;
}

double CSeriesValue::GetPeriod(USHORT usSeriesIndex)
{
	if (TIME_SERIES_MAX_COUNT <= usSeriesIndex)
	{
		return -1;
	}
	return m_adPeriod[usSeriesIndex];
}
