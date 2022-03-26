#pragma once
/**
 * @file PMUData.h
 * @brief The PMU data of all board
 * @author Guangyun Wang
 * @date 2020/08/06
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include <windows.h>
#include "Operation.h"
/**
 * @class CPMUData
 * @brief The PMU measurement data
*/
class CPMU
{
public:
	/**
	 * @brief Get the instance of CPMUData
	 * @return The point of the instance
	*/
	static CPMU* Instance();
	/**
	 *@brief Destructor
	*/
	~CPMU();
	/**
	 * @brief Set the PMU average data memory
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] pusAverageData The memory of average data
	 * @return Execute result
	 * - 0 Set data memory successfully
	 * - -1 Had set the data memory of the controller already
	*/
	int SetAverageDataMemory(BYTE bySlotNo, BYTE byControllerIndex, USHORT* pusAverageData);
	/**
	 * @brief The the sample mode memory
	 * @param[in] bySlotNo The slot memory
	 * @param[in] byControllerIndex The controller index
	 * @param[in] puTimes The sample times memory
	 * @param[in] pdPeriod The sampler period memory
	 * @return Execute result
	 * - 0 Set sample memory successfully
	 * - -1 Had set the sample memory of the controller already
	*/
	int SetSampleModeMemory(BYTE bySlotNo, BYTE byControllerIndex, UINT* puTimes, double* pdPeriod);
	/**
	 * @brief Set the mode of measurement
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] pucMeasureType The measurement type memory
	 * @return  Execute result
	 * - 0 Set sample memory successfully
	 * - -1 Had set the measurement memory of the controller already
	*/
	int SetMeasureModeMemory(BYTE bySlotNo, BYTE byControllerIndex, unsigned char* pucMeasureType);
	/**
	 * @brief Set the mode of latest set
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] pucMeasureType The measurement type memory
	 * @return  Execute result
	 * - 0 Set sample memory successfully
	 * - -1 Had set the measurement memory of the controller already
	*/
	int SetLatestMeasureModeMemory(BYTE bySlotNo, BYTE byControllerIndex, unsigned char* pucMeasureType);
	/**
	 * @brief Set the force mode of measurement
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] pucForceMode The force mode memory
	 * @return  Execute result
	 * - 0 Set foce mode memory successfully
	 * - -1 Had set the force mode memory of the controller already
	*/
	int SetForceModeMemory(BYTE bySlotNo, BYTE byControllerIndex, unsigned char* pucForceMode, unsigned char* pucIRange);
	/**
	 * @brief Save the average data
	 * @param[in] Operation The operation class
	 * @param[in] byGetChip The chip whose average will be gotten and saved
	 * @param[in] uSampleCount The sample count
	 * @param[in] byChipEvenChannel The chip channel type, one bit one chip, 1 is even channel
	 * @return Execute result
	 * - 0 Save average data successfully
	 * - -1 The sample is zero
	 * - -2 Get the average data fail
	 * - -3 Allocate memory fail
	*/
	int SaveAverageData(COperation& Operation, BYTE byGetChip, UINT uSampleCount, BYTE byChipEvenChannel = 0xFF);
	/**
	 * @brief Set the sample setting of channel
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The slot number
	 * @param[in] vecChannel The channel number
	 * @param[in] uSampleCount The sample count
	 * @param[in] dPeriod The sampler period
	*/
	void SetSampleSetting(BYTE bySlotNo, BYTE byController, const std::vector<USHORT>& vecChannel, UINT uSampleCount, double dPeriod);
	/**
	 * @brief Get the sample setting
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller index
	 * @param[in] usChannel The channel number
	 * @param[in] uSampleCount The sample count
	 * @param[in] dPeriod The sample period
	 * @return Execute result
	 * - 0 Get the sample setting successfully
	 * - -1 The controller is not existed
	 * - -2 The controller memory is not set
	 * - -3 The channel is over range
	*/
	int GetSampleSetting(BYTE bySlotNo, BYTE byController, USHORT usChannel, UINT& uSampleCount, double& dPeriod);
	/**
	 * @brief Set the measurement mode
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller index
	 * @param[in] vecChannel The channel number
	 * @param[in] ucMeasureMode The measure mode
	*/
	void SetMeasureMode(BYTE bySlotNo, BYTE byController, const std::vector<USHORT>& vecChannel, unsigned char ucMeasureMode);
	/**
	 * @brief Get the measure mode of the channel
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller index
	 * @param[in] usChannel The channel number
	 * @param[out] ucMeasureMode The measurement mode, 0 is MV and 1 is MI
	 * @return Execute result
	 * - 0 Get the measurement mode successfully
	 * - -1 The controller in the slot is not record
	 * - -2 The channel number is over range
	*/
	int GetMeasureMode(BYTE bySlotNo, BYTE byController, USHORT usChannel, unsigned char& ucMeasureMode);
	/**
	 * @brief 
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller index
	 * @param[in] vecChannel The channel number
	 * @param[in] ucForceMode The force mode, 0 is FV and 1 is FI
	 * @param[in] ucIRange The current range
	 * @return Execute result
	 * - 0 Get the force mode successfully
	 * - -1 The controller in the slot is not record
	 * - -2 The channel number is over range
	*/
	int SetForceMode(BYTE bySlotNo, BYTE byController, const std::vector<USHORT>& vecChannel, unsigned char ucForceMode, unsigned char ucIRange);
	/**
	 * @brief Get the measure mode of the channel
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller index
	 * @param[in] usChannel The channel number
	 * @param[out] ucForceMode The force mode, 0 is FV and 1 is FI
	 * @param[out] ucIRange The current range
	 * @return Execute result
	 * - 0 Get the force mode successfully
	 * - -1 The controller in the slot is not record
	 * - -2 The channel number is over range
	*/
	int GetForceMode(BYTE bySlotNo, BYTE byController, USHORT usChannel, unsigned char& ucForceMode, unsigned char& ucIRange);
	/**
	 * @brief Get the average data
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] usChannel The channel number
	 * @return The average data
	 * - >=0 The average data
	 * - -1 Not get average data before
	 * - -2 The channel number is over range
	*/
	int GetAverageData(BYTE bySlotNo, BYTE byControllerIndex, USHORT usChannel);
private:
	/**
	 * @brief Constructor
	*/
	CPMU();
	/**
	 * @brief Get the controller ID
	 * @param[in][in] bySlotNo The slot number
	 * @param[in][in] byControllerIndex The controller index
	 * @return The controller ID
	*/
	inline UINT GetControllerID(BYTE bySlotNo, BYTE byControllerIndex);
private:
	/**
	 * @struct SAMPLE_SETTING
	 * @brief The sample settings
	*/
	struct SAMPLE_SETTING
	{
		double* m_pdPeriod;///<The sample period of each channel in controller
		UINT* m_puSampleTimes;///<The sample times of each channel in controller
		SAMPLE_SETTING()
		{
			m_pdPeriod = nullptr;
			m_puSampleTimes = nullptr;
		}
	};
	/**
	 * @struct FORCE_MODE
	 * @brief The force mode
	*/
	struct FORCE_MODE 
	{
		unsigned char* m_pucForceMode;///<The force mode, 0 is FV and 1 is FI
		unsigned char* m_pucIRange;///<The current range
		FORCE_MODE()
		{
			m_pucForceMode = nullptr;
			m_pucIRange = nullptr;
		}
	};
	std::map<UINT, USHORT*> m_mapAverageData;///<The PMU data of each controller, the key is controller ID and the value is PMU data
	std::map<UINT, SAMPLE_SETTING> m_mapSampleSetting;///<The sample setting of each channel
	std::map<UINT, UCHAR*> m_mapMeasureMode;///<The measure mode of each channel;
	std::map<UINT, UCHAR*> m_mapLatestMeasureMode;///<The measurement mode of each channel in latest set
	std::map<UINT, FORCE_MODE> m_mapForceMode;///<The force mode of each channel
};

