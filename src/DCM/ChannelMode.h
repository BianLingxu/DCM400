#pragma once
/**
 * @file ChannelModes.h
 * @brief Include the class using for saving channel status
 * @author Guangyun Wang
 * @date 2021/04/15
 * @copyright Copyright (c) AccoTEST business unit of Beijing Huafeng Test & Control Technology Co,. Ltd.
*/
#include "Operation.h"
/**
 * @class CChannelMode
 * @brief The class for saving and getting channel mode
*/
class CChannelMode
{
public:
	enum class CHANNEL_MODE
	{
		MCU_MODE = 0,///<The MCU mode
		PMU_MODE,///<The PMU mode
		NEITHER_MODE,///<Neither MCU mode, nor PMU mode
	};
	/**
	 * @brief Get the instance of CChannelStatus
	 * @return The point of the instance
	*/
	static CChannelMode* Instance();
	/**
	 * @brief Set the memory for saving the channel mode of the controller
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] pbyChannelMode The channel status
	 * @return Execute result
	 * - 0 Set channel status memory successfully
	 * - -1 Had set the memory of the controller already
	*/
	int SetMemory(BYTE bySlotNo, BYTE byControllerIndex, BYTE* pbyChannelMode);
	/**
	 * @brief Set the unexpect mode
	 * @param[in] Operation The operation
	 * @param[in] byController The controller index
	 * @param[in] vecChannel The channels need to be set
	 * @param[in] UnexceptMode Unexpect mode
	 * @return Execute result
	 * - 0 Set unexpect mode
	 * - -1 The memory for saving channel mode is not be set
	 * - -2 The channel number is over range
	*/
	int SetUnexpectMode(COperation& Operation, const std::vector<USHORT>& vecChannel, CHANNEL_MODE UnexceptMode);
	/**
	 * @brief Update the channel mode of the controller
	 * @param[in] Operation The operation
	 * @return Execute result
	 * - 0 Update mode successfully
	 * - -1 Not set the memory for saving channel mode of the controller
	*/
	int UpdateMode(COperation& Operation);
	/**
	 * @brief Set channel mode
	 * @param[in] Operation The operation
	 * @param[in] vecChannel The channel will be set mode
	 * @param[in] ChannelMode The channel mode
	 * @return Execute result
	 * - 0 Set channel mode successfully
	 * - -1 The channel is over range
	*/
	int SetChannelMode(COperation& Operation, const std::vector<USHORT>& vecChannel, CHANNEL_MODE ChannelMode);
	/**
	 * @brief Set channel mode
	 * @param[in] bySlotNo The slot numer
	 * @param[in] byControllerIndex The controller index
	 * @param[in] vecChannel The channels whose mode will be saved
	 * @param[in] ChannelMode The channel mode
	 * @return Execute result
	 * - 0 Save channel mode successfully
	 * - -1 Not set the memory for saving channel mode of the controller
	*/
	int SaveChannelMode(BYTE bySlotNo, BYTE byControllerIndex, const std::vector<USHORT>& vecChannel, CHANNEL_MODE ChannelMode);
	/**
	 * @brief Get the channel mode
	 * @param[in] Operation The operation class
	 * @param[in] usChannel The channel number
	 * @return The channel mode
	*/
	CHANNEL_MODE GetChannelMode(COperation& Operation, USHORT usChannel);
private:
	/**
	 * @brief Constructor
	*/
	CChannelMode();
	/**
	 * @brief Get the controller ID of the controller
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @return The controller ID
	*/
	inline UINT GetControllerID(BYTE bySlotNo, BYTE byControllerIndex);
private:
	std::map<UINT, BYTE*> m_mapChannelMode;///<The channel status of each controller, key is controller ID and value is the mode of each channel
};