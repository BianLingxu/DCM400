#pragma once
#include "DCM400HardwareInfo.h"
#include <vector>
#include <map>
/**
 * @class CSeriesValue
 * @brief The series value of each channel
*/
class CSeriesValue
{
public:
	/**
	 * @brief Constructor
	*/
	CSeriesValue();
	/**
	 * @brief Get the series count
	 * @return The series count
	*/
	USHORT GetSeriesCount();
	/**
	 * @brief Set the edge of current series
	 * @param[in] usSeriesIndex The series index
	 * @param[in] pdEdge The edge value of each type, the element count must not less than 6
	 * @return Execute result
	 * - 0 Set edge successfully
	 * - -1 The series index is over range
	 * - -2 The point of edge is nullptr
	*/
	int SetEdge(USHORT usSeriesIndex, const double* pdEdge);
	/**
	 * @brief Get the edge of current series
	 * @param[in] usSeriesIndex The series index
	 * @param[out] pdEdge The edge value, the element count must not less than 6
	 * @return Exeucte result
	 * - 0 Get the edge successfully
	 * - -1 The series index is over range
	 * - -2 The edge point is nullptr
	*/
	int GetEdge(USHORT usSeriesIndex, double* pdEdge);
	/**
	 * @brief Set the format of current series
	 * @param[in] usSeriesIndex The series index
	 * @param[in] WaveFormat The wave format
	 * @param[in] IOFormat The IO format
	 * @param[in] CompareMode The compare mode
	 * @return Execute result
	 * - 0 Set format successfully
	 * - -1 The series index is over range
	*/
	int SetFormat(USHORT usSeriesIndex, const WAVE_FORMAT& WaveFormat, const IO_FORMAT& IOFormat, const COMPARE_MODE& CompareMode);
	/**
	 * @brief Get the format of current series
	 * @param[in] usSeriesIndex The series index
	 * @param[out] WaveFormat The wave format
	 * @param[out] IOFormat The IO format
	 * @param[out] CompareMode The compare mode
	 * @return Execute result
	 * - 0 Get format successfully
	 * - -1 The series index is over range
	*/
	int GetFormat(USHORT usSeriesIndex, WAVE_FORMAT& WaveFormat, IO_FORMAT& IOFormat, COMPARE_MODE& CompareModex);
	/**
	 * @brief Set the period of current series
	 * @param[in] usSeriesIndex The series index
	 * @param[in] dPeriod The period of current series
	 * @return Execute result
	 * - 0 Set period successfully
	 * - -1 The series index is over range
	*/
	int SetPeriod(USHORT usSeriesIndex, double dPeriod);
	/**
	 * @brief Get the period
	 * @param[in] usSeriesIndex The series index
	 * @return The period of current series
	 * - >0 The period
	 * - -1 The series is over range
	*/
	double GetPeriod(USHORT usSeriesIndex);
private:
	USHORT m_usValidSeriesCount;///<The valid series count
	double m_adT1R[TIME_SERIES_MAX_COUNT][EDGE_COUNT];///<The value of each format and each series
	BYTE m_abyFormat[TIME_SERIES_MAX_COUNT];///<The format of each series, inclde wave form/IO format and compare mode
	double m_adPeriod[TIME_SERIES_MAX_COUNT];///<The period of each series
};

/**
 * @class CTimeSetSeries
 * @brief The timeSet series for the channel
*/
class CTimeSetSeries
{
public:
	/**
	 * @brief Constructor
	*/
	CTimeSetSeries();
	/**
	 * @brief Copy constructor
	 * @param[in] Series The source class
	*/
	CTimeSetSeries(const CTimeSetSeries& Series);
	/**
	 * @brief Overload operate =
	 * @param Series The source class
	 * @return The target class
	*/
	CTimeSetSeries& operator =(const CTimeSetSeries& Series);
	/**
	 * @brief Set the edge series
	 * @param[in] pbyEdgeSeries The point pointed to edge series, the element count must not less than 6
	 * @return Execute result
	 * - 0 Set the edge series successfully
	 * - -1 The point is nullptr
	 * - -2 The series is over range
	*/
	int SetEdgeSeries(const BYTE* pbyEdgeSeries);
	/**
	 * @brief Get the edge series
	 * @return The edge series
	*/
	const BYTE* GetEdgeSeries();
	/**
	 * @brief Set the format series
	 * @param byFormatSeries The format series
	 * @return Execute result
	 * - 0 Set the format series successfully
	 * - -1 The series index is over range
	*/
	int SetFormatSeries(BYTE byFormatSeries);
	/**
	 * @brief Get the format series
	 * @return The format series
	*/
	int GetFormatSeries();
	/**
	 * @brief Set the period series
	 * @param byPeriodSeries The period series
	 * @return Execute result
	 * - 0 Set the period series successfully
	 * - -1 The series is over range
	*/
	int SetPeriodSeries(BYTE byPeriodSeries);
	/**
	 * @brief Get the period series
	 * @return The period series
	*/
	int GetPeriodSeries();
	/**
	 * @brief The resetructor for operator equal
	 * @param[in] TimeSetSeries The target timeset series
	 * @return Whether timeset is euqla
	*/
	bool operator ==(const CTimeSetSeries& TimeSetSeries);
private:
	BYTE m_abyEdgeSeries[EDGE_COUNT];///<The edge series
	BYTE m_byFormatSeries;///<The wave format
	BYTE m_byPeriodSeries;///<The period series
};


/**
 * @class CTimeSet
 * @brief The time set of all channel
*/
class CTimeSet
{
public:
	/**
	 * @brief Constructor
	 * @param[in] byIndex The time set index
	*/
	CTimeSet(BYTE byIndex);
	/**
	 * @brief Destructor
	*/
	~CTimeSet();
	/**
	 * @brief Get the index of time set
	 * @return The time set index
	*/
	int Index();
	/**
	 * @brief Set the timeset series of the channel
	 * @param[in] Channel The channel information
	 * @param[in] TimesetSeries The timset series of the channel
	 * @return Execute result
	 * - 0 Set the series successfully
	 * - -1 The channel is over range
	*/
	int SetChannelSeries(const CHANNEL_INFO& Channel, const CTimeSetSeries& TimesetSeries);
	/**
	 * @brief Get the channel's time set series
	 * @param[in] Channel The channel information
	 * @param[out] TimesetSeries The timset series
	 * @return Execute result
	 * - 0 Get the channel's time set series successfully
	 * - -1 The channel is over range
	 * - -2 The channel's time set series is not existed
	*/
	int GetChannelSeries(const CHANNEL_INFO& Channel, CTimeSetSeries& TimesetSeries);
	/**
	 * @brief Get the same series count
	 * @return The same series couont
	*/
	int GetSameSeriesCount();
	/**
	 * @brief Get the same series
	 * @param nSameSeriesIndex The same serisers index
	 * @param vecChannel The channel with same series
	 * @param TimesetSeries The time set series
	 * @return Execute result
	 * - 0 Get the channel with same series successfully
	 * - -1 The same series index is over range
	*/
	int GetSameSeries(int nSameSeriesIndex, std::vector<CHANNEL_INFO>& vecChannel, CTimeSetSeries& TimesetSeries);
private:
	/**
	 * @brief Get the channel ID
	 * @param[in] Channel The channel information
	 * @return The channel ID
	*/
	inline USHORT GetChannelID(const CHANNEL_INFO& Channel);
	/**
	 * @brief Get the channel information
	 * @param[in] usID The channel ID
	 * @param[out] Channel The channel information
	*/
	inline void GetChannel(USHORT usID, CHANNEL_INFO& Channel);
private:
	BYTE m_byIndex;///<The index of current time set
	std::map<USHORT, CTimeSetSeries*> m_mapChannel;///<The channels' time set series, key is channel ID and value is the point pointed to time set series
	std::vector<std::vector<USHORT>> m_vecSameSeries;///<The channel which has same series
};
