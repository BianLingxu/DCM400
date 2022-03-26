#pragma once
/**
 * @file Calibration.h
 * @brief Include the class for the calibration of all controller
 * @date 2020/07/21
 * @author Guangyun Wang
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "HardwareInfo.h"
#include <map>
/**
 * @class CCalibration
 * @brief The calibration data of all board
*/
class CCalibration
{
public:
	/**
	 * @enum CAL_TYPE
	 * @brief The calibration data type
	*/
	enum class CAL_TYPE
	{
		CAL_DVH = 0,///Driver voltage high
		CAL_DVL,///<Driver voltage low
		CAL_FV,///<Force voltage
		CAL_FI,///<Force current
		CAL_MV,///<Measure voltage
		CAL_MI,///<Measure current
	};
	/**
	 * @brief Get the instance of CCalibration
	 * @return The point of class CCalibration
	*/
	static CCalibration* Instance();
	/**
	 * @brief Set the calibration memory of the controller
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller index
	 * @param[in] pCalData The calibration data memory address of the controller
	 * @return Execute result
	 * - 0 Set calibration memory successfully
	 * - -1 The memory of calibration data in this controller had been set before
	*/
	int SetCalibrationMemory(BYTE bySlotNo, BYTE byController, CAL_DATA* pCalData);
	/**
	 * @brief Set the calibration data of the controller
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller index
	 * @param[in] pCalData The calibration data
	 * @return Execute result
	 * - 0 Set calibration data successfully
	 * - -1 The controller in the slot is not existed
	*/
	int SetCalibration(BYTE bySlotNo, BYTE byController, const CAL_DATA* pCalData);
	/**
	 * @brief Set the calibration data of channel
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller index
	 * @param[in] usChannel The channel number
	 * @param[in] pCalData The calibration data
	 * @return Execute result
	 * 0 Set calibration data successfully
	 * - -1 The controller in the slot is not existed
	 * - -2 The channel is over range
	*/
	int SetCalibration(BYTE bySlotNo, BYTE byController, USHORT usChannel, const CAL_DATA* pCalData);
	/**
	 * @brief Set the calibration data of channel
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller
	 * @param[in] usChannel The channel number
	 * @param[in] CalType The calibration data type
	 * @param[in] IRange The PMU current range type
	 * @param[in] fGain The gin value
	 * @param[in] fOffset The offset value
	 * @return Execute result
	 * - 0 Set calibration data successfully
	 * - -1 The controller in the slot is not existed
	 * - -2 The channel number is over range
	 * - -3 The calibration data type is error
	 * - -4 The current range is error
	*/
	int SetCalibration(BYTE bySlotNo, BYTE byController, USHORT usChannel, CAL_TYPE CalType, PMU_IRANGE IRange, float fGain, float fOffset);
	/**
	 * @brief Reset the calibration data of channel
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller index
	 * @param[in] usChannel The channel number
	 * @return Execute result
	 * - 0 Reset calibration data successfully
	 * - 1 The controller in the slot is not existed
	 * - -2 The channel is over range
	*/
	int ResetCalibrationData(BYTE bySlotNo, BYTE byController, USHORT usChannel);
	/**
	 * @brief Reset calibration data of all channel in controller
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller
	 * @return Execute result
	 * - 0 Reset calibration data successfully
	 * - 1 The controller in the slot is not existed
	*/
	int ResetCalibrationData(BYTE bySlotNo, BYTE byController);
	/**
	 * @brief Get the calibration data of the channel
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller
	 * @param[in] usChannel The channel number
	 * @param[out] CalData The calibration data of the channel
	 * @return Execute result
	 * - 0 Get the calibration data successfully
	 * - -1 The controller in the slot is not existed
	 * - -2 The channel is over range
	*/
	int GetCalibration(BYTE bySlotNo, BYTE byController, USHORT usChannel, CAL_DATA& CalData);
	/**
	 * @brief Get the calibration data
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller
	 * @param[out] pCalData The calibration data of the channels in the controller
	 * @return Execute result
	 * - 0 Get the calibration data successfully
	 * - -1 The point of calibration data is nullptr
	*/
	int GetCalibration(BYTE bySlotNo, BYTE byController, CAL_DATA* pCalData);
private:
	/**
	 * @brief Constructor
	*/
	CCalibration();
	/**
	 * @brief Get the controller ID
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller
	 * @return The controller ID
	*/
	inline UINT GetControllerID(BYTE bySlotNo, BYTE byController);
	/**
	 * @brief Get the default calibration data
	 * @param[out] CalData The calibration data
	*/
	inline void GetDefaultCalibrationData(CAL_DATA& CalData);
private:
	std::map<UINT, CAL_DATA*> m_mapCalibrationData;///<The calibration data of the controller, the key is controller ID and value is its calibration data

};

