#pragma once
/**
 * @file TMU.h
 * @brief The TMU message of all board
 * @author Guangyun Wang
 * @date 2020/08/06
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include <map>
#include "DCM400HardwareInfo.h"
/**
 * @class CTMU
 * @brief The TMU mode
*/
class CTMU
{
public:
	/**
	 * @brief Get the instance of CTMU
	 * @return The point of the instance
	*/
	static CTMU* Instance();
	/**
	 * @brief Set the memory of TMU unit channel
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] pusUnitChannel The memory of the unit channel
	 * @return Execute result
	 * - 0 Set the memory successfully
	 * - -1 Have set the memory before
	 * - -2 The memory is nullptr
	*/
	int SetUnitChannelMemory(BYTE bySlotNo, BYTE byControllerIndex, USHORT* pusUnitChannel);
	/**
	 * @brief Set the memory of unit mode
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] pbyUnitMode The memory of unit mode
	 * @return Execute result
	 * - 0 Set the memory successfully
	 * - -1 Have set the memory before
	 * - -2 The memory is nullptr
	*/
	int SetModeMemory(BYTE bySlotNo, BYTE byControllerIndex, BYTE* pbyUnitMode);
	/**
	 * @brief Set the memory of trigger edge
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] pbyTriggerEdge The memory of trigger edge
	 * @return Execute result
	 * - 0 Set the memory successfully
	 * - -1 Have set the memory before
	 * - -2 The memory is nullptr
	*/
	int SetTriggerEdgeMemory(BYTE bySlotNo, BYTE byControllerIndex, BYTE* pbyTriggerEdge);
	/**
	 * @brief Set the memory of the hold off
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] pusHolfOffTime The memory of hold off time
	 * @param[in] pusHoldOffNum The memory of the hold off number
	 * @return Execute result
	 * - 0 Set the memory successfully
	 * - -1 Have set the memory before
	 * - -2 The memory is nullptr
	*/
	int SetHoldOffMemory(BYTE bySlotNo, BYTE byControllerIndex, USHORT* pusHolfOffTime, USHORT* pusHoldOffNum);
	/**
	 * @brief Set the memory of sample
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] pusSampleNum The memory of sample number
	 * @return Execute result
	 * - 0 Set the memory successfully
	 * - -1 Have set the memory before
	 * - -2 The memory is nullptr
	*/
	int SetSampleNumberMemory(BYTE bySlotNo, BYTE byControllerIndex, USHORT* pusSampleNum);
	/**
	 * @brief Set the memory of timeout
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] pfTimeout The memory of timeout
	 * @return Execute result
	 * - 0 Set the timeout successfully
	 * - -1 Have set the memory before
	 * - -2 The memory is nullptr
	*/
	int SetTimeoutMemory(BYTE bySlotNo, BYTE byControllerIndex, float* pfTimeout);
	/**
	 * @brief Set the unit channel number
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] byUnitIndex The unit index
	 * @param[in] usChannel The channel will be bind to unit
	 * @return Execute result
	 * - 0 Set the channel successfully
	 * - -1 The memory of the controller is not be set
	 * - -2 The unit index is over range
	 * - -3 The channel is over range
	*/
	int SetChannel(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex, USHORT usChannel);
	/**
	 * @brief Get the unit channel
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] byUnitIndex The unit index
	 * @return The channel in the unit
	 * >=0 The channel number of the unit
	 * - -1 The memory of the controller is not existed
	*/
	int GetChannel(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex);
	/**
	 * @brief Set the mode of the index
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] byUnitIndex The unit index
	 * @param[in] byMode The mode of the unit
	 * @return Execute result
	 * - 0 Set the unit mode successfully
	 * - -1 The memory of the controller is not be set
	 * - -2 The unit index is over range
	*/
	int SetMode(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex, BYTE byMode);
	/**
	 * @brief Get the mode of the unit
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] byUnitIndex The unit index
	 * @return The mode of the unit
	 * - >=0 The mode of unit
	 * - -1 The memory of the controller is not be set
	 * - -2 The unit index is over range
	*/
	int GetMode(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex);
	/**
	 * @brief Set the trigger edge
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] byUnitIndex The unit index
	 * @param[in] bRaiseEdge Whether the trigger edge is raise edge
	 * @return Execute result
	 * - >=0 The mode of unit
	 * - -1 The memory of the controller is not be set
	 * - -2 The unit index is over range
	*/
	int SetTriggerEdge(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex, BOOL bRaiseEdge);
	/**
	 * @brief Get the trigger edge
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] byUnitIndex The unit index
	 * @return The trigger edge type
	 * - 0 The trigger edge is fall edge
	 * - 1 The trigger edge is raise edge
	 * - -1 The memory of the controller is not be set
	 * - -2 The unit index is over range
	*/
	int GetTriggerEdge(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex);
	/**
	 * @brief Set the hold off of TMU
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] byUnitIndex The unit index
	 * @param[in] usHoldOffTime The hold off time
	 * @param[in] usHoldOffNum The hold off number
	 * @return Execute result
	 * - >=0 The mode of unit
	 * - -1 The memory of the controller is not be set
	 * - -2 The unit index is over range
	*/
	int SetHoldOff(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex, USHORT usHoldOffTime, USHORT usHoldOffNum);
	/**
	 * @brief Get hold off
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] byUnitIndex The TMU unit index
	 * @param[out] usHoldOffTime The hold off time
	 * @param[out] usHolfOffNum The hold off number
	 * @return Execute result
	 * - 0 Get the hold off successfully
	 * - -1 The memory of the controller is not be set
	 * - -2 The unit index is over range
	*/
	int GetHoldOff(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex, USHORT& usHoldOffTime, USHORT& usHolfOffNum);
	/**
	 * @brief Set the sample number
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] byUnitIndex The unit index
	 * @param[in] usSampleNum The sample number
	 * @return Execute result
	 * - 0 Set the sample successfully
	 * - -1 The memory of the controller is not be set
	 * - -2 The unit index is over range
	*/
	int SetSampleNumber(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex, USHORT usSampleNum);
	/**
	 * @brief Ge the sample number
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] byUnitIndex The unit index
	 * @return The sample number
	 * - >=0 The sample number
	 * - -1 The memory of the controller is not be set
	 * - -2 The unit index is over range
	*/
	int GetSampleNumber(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex);
	/**
	 * @brief Set the timeout
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] byUnitIndex The unit index
	 * @param[in] fTimeout The timeout, unit is ms
	 * @return Execute result
	 * - 0 Set the timeout successfully
	 * - -1 The memory of the controller is not be set
	 * - -2 The unit index is over range
	*/
	int SetTimeout(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex, float fTimeout);
	/**
	 * @brief Get the timeout
	 * @param[in] bySlotNo The slot number
	 * @param[in] byControllerIndex The controller index
	 * @param[in] byUnitIndex The unit index
	 * @return The timeout
	 * - >=0 The timeout
	 * - -1 The memory of the controller is not be set
	 * - -2 The unit index is over range
	*/
	float GetTimeout(BYTE bySlotNo, BYTE byControllerIndex, int byUnitIndex);
private:
	/**
	 * @brief Constructor
	*/
	CTMU();
	/**
	 * @brief Get the controller ID
	 * @param[in][in] bySlotNo The slot number
	 * @param[in][in] byControllerIndex The controller index
	 * @return The controller ID
	*/
	BYTE GetControllerID(BYTE bySlotNo, BYTE byControllerIndex);	
private:
	std::map<BYTE, USHORT*> m_mapChannel;///<The channel of each TMU unit
	std::map<BYTE, BYTE*> m_mapMode;///<The mode of each TMU unit
	std::map<BYTE, BYTE*> m_mapTriggerEdge;///<The trigger edge of each TMU unit
	std::map<BYTE, USHORT*> m_mapHoldOffTime;///<The hold off time of each TMU unit
	std::map<BYTE, USHORT*> m_mapHoldOffNum;///<The hold off number of each TMU unit
	std::map<BYTE, USHORT*> m_mapSampleNum;///<The sample number of each TMU unit
	std::map<BYTE, float*> m_mapTimeout;///<The timeout of each TMU unit
};

