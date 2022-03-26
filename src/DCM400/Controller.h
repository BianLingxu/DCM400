#pragma once
/**
 * @file Controller.h
 * @brief Include the class of CController
 * @detail The class can realize all function of controller
 * @author Guangyun Wang
 * @date 2022/02/09
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "HardwareFunction.h"
#include "FlashInformation.h"
#include "Pattern.h"
/**
 * @class CController
 * @brief The class for controller function
*/
class CController
{
public:
	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number of board belonged
	 * @param[in] byIndex The controller index
	*/
	CController(BYTE bySlotNo, BYTE byIndex, CDriverAlarm* pAlarm);
	/**
	 * @brief Whether the controller is existed
	 * @return Whether the conotroller is existed
	 * - TRUE The controller is existed
	 * - FALSE The controller is not existed
	 */
	BOOL IsExist();
	/**
	 * @brief Get the point of CHardwareFunction
	 * @return The point of CHardwareFunction
	*/
	CHardwareFunction* GetHardwareFunction();
	/**
	 * @brief Whether the vector validity
	 * @return Execute result
	 * - TRUE The vector is valid
	 * - FALSE The vector is invalid
	*/
	BOOL IsVectorValid();
	/**
	 * @brief Set the channel pattern
	 * @param[in] usChannel The channel number
	 * @param[in] uPatternLine The pattern line
	 * @param[in] cPattern The pattern of the channel
	 * @param[in] PatternCMD The pattern command
	 * @return Execute result
	 * - 0 Set pattern successfully
	 * - -1 The channel is over range
	 * - -2 The pattern number is over range
	 * - -3 Allocate memory fail
	*/
	int AddChannelPattern(USHORT usChannel, UINT uPatternLine, char cPattern, const CPatternCMD& PatternCMD);
	/**
	 * @brief Load vector to memory
	 * @return Execute result
	 * - 0 Load vector successfully
	 * - -1 Allocate memory fail
	 */
	int LoadVector();
	/**
	 * @brief Set the vaector validity
	 * @param[in] bValid Whether the vector validity
	*/
	void SetVectorValid(BOOL bValid);
	/**
	 * @brief Set the period Series
	 * @param[in] bySeries The option index
	 * @param[in] dPeriod The period of current option index
	 * @return Execute result
	 * - 0 Set the period Series successfully
	 * - -1 The series index is over range
	 * - -2 The period is over range
	*/
	int SetPeriodSeries(BYTE bySeries, double dPeriod);
	/**
	 * @brief Set the edge Series
	 * @param[in] bySeries The Series index
	 * @param[in] EdgeType The edge type
	 * @param[in] dEdge The edge value
	 * @return Execute result
	 * - 0 Set the edge Series successfully
	 * - -1 The series index is over range
	 * - -2 The edge point is nullptr
	*/
	int SetEdgeSeries(const std::vector<USHORT>& vecChannel, BYTE bySeries, const double* pdEdge);
	/**
	 * @brief Get the Series edge
	 * @param[in] bySeries The option index
	 * @param[in] EdgeType The edge type
	 * @return The edge of the option index
	 * - >=0 The edge Series
	 * - -1 The option index is over range
	*/
	double GetEdgeSeries(USHORT usChannel, BYTE bySeries, EDGE_TYPE EdgeType);
	/**
	 * @brief Set the format Series
	 * @param[in] bySeries The option index
	 * @param[in] WaveFormat The wave format
	 * @param[in] IOFormat The IO format
	 * @param[in] CompareMode The compare mode
	 * @return Execute result
	 * - 0 Set the format successfully
	 * - -1 The series index is over range
	*/
	int SetFormatSeries(const std::vector<USHORT>& vecChannel, BYTE bySeries, WAVE_FORMAT WaveFormat, IO_FORMAT IOFormat, COMPARE_MODE CompareMode);
	/**
	 * @brief Get the format series
	 * @param[in] bySeries The series index
	 * @param[out] WaveFormat The wave format
	 * @param[out] IOFormat The IO format
	 * @param[out] CompareMode The compare mode
	 * @return Execute result
	 * - 0 Get format successfully
	 * - -1 The series index is over range
	*/
	int GetFormatSeries(USHORT usChannel, BYTE bySeries, WAVE_FORMAT& WaveFormat, IO_FORMAT& IOFormat, COMPARE_MODE& CompareMode);
	/**
	 * @brief Set the time set
	 * @param[in] vecChannel The channel number
	 * @param[in] usTimeSetIndex The time set index
	 * @param[in] byPeriodSeries The period series
	 * @param[in] pbyEdgeSeries The edge series index
	 * @param[in] byFormatSeries The format series
	 * @return Execute result
	 * - 0 Set the time set successfully
	 * - -1 The time set index is over range
	 * - -2 The series is over range
	 * - -3 The point pointed to edge series is nullptr
	 * - -4 The channel is over range
	 * - -5 The edge series is over range
	 * - -6 The format series is over range
	 * - -7 The edge value is over range
	*/
	int SetTimeSet(const vector<USHORT>& vecChannel, USHORT usTimeSetIndex, BYTE byPeriodSeries, const BYTE* pbyEdgeSeries, BYTE byFormatSeries);
	/**
	 * @brief Set the operand
	 * @param[in] uLineNo The line number
	 * @param[in] usOperand The operand
	 * @return Execute result
	 * - 0 Set the operand successfully
	 * - -1 The line number is over range
	 * - -2 The operand is over range
	*/
	int SetOperand(UINT uLineNo, USHORT usOperand);
	/**
	 * @brief Set the line information whose wave data will be written in future
	 * @param[in] uStartLine The start line number
	 * @param[in] uLineCount The line number count
	 * @return Execute result
	 * - 0 Set the channel data information successfully
	 * - -1 The start line is over range
	 * - -2 The line count is over range
	 * - -3 Allocate memory fail
	*/
	int SetLineInfo(UINT uStartLine, UINT uLineCount);
	/**
	 * @brief Set the wave data of channel
	 * @param[in] vecChannel The channel number
	 * @param[in] pbyData The wave data data
	 * @return Execute result
	 * - 0 Set the channel data successfully
	 * - -1 Not set the channel data information
	 * - -2 The channel is over range
	 * - -3 The point of data is nullptr
	*/
	int SetWaveData(const std::vector<USHORT>& vecChannel, const BYTE* pbyData);
	/**
	 * @brief Write channel data to memory
	 * @return Execute result
	 * - 0 Set the wave data successfully
	 * - -1 Not set the channel data information
	*/
	int WriteData();
	/**
	 * @brief Get the capture of channel
	 * @param[out] vecCapture The capture data
	 * @return Execute result
	 * - 0 Get the capture successfully
	 * - -1 The channel number is over range
	 * - -2 Vector not ran
	 * - -3 Vector running
	*/
	int GetCapture(USHORT usChannel, std::vector<UINT>& vecCapture);

	/**
	 * @brief Get the fail line number of last running
	 * @param[in] usChannel The channel number
	 * @param[in] uGetMaxFailCount The maximum fail count will be gotten
	 * @param[in] vecLineNo The  fail line number of latest running
	 * @param[in] bForceRefresh Whether force refresh fail line number
	 * @return Execute result
	 * - 0 Get the fail line number successfully
	 * - -1 The channel is over range
	 * - -2 Not ran before
	 * - -3 Vector running
	*/
	int GetFailLineNo(USHORT usChannel, UINT uGetMaxFailCount, std::vector<int>& vecLineNo, BOOL bForceRefresh = FALSE);
	/**
	 * @brief Get the calibration information
	 * @param[out] pCalInfo The calibration information of each channel
	 * @param[in] nElementCount The element count of each array
	 * @return Execute result
	 * - 0 Get the calibration information successfully
	 * - -1 The point of array is nullptr
	 * - -2 The element count is less than the channel of controller
	 * - -3 Get calibration information fail
	*/
	int GetCalibrationInfo(STS_CALINFO* pCalInfo, int nElementCount);
private:
	BYTE m_bySlotNo;///<The slot number of board
	BYTE m_byIndex;///<The index of the controller
	CDriverAlarm* m_pAlarm;///<The point pointed to alarm
	CPattern m_Pattern;///<The pattern of current controller
	CHardwareFunction m_Hardware;///<The hardware function class for current class
};

