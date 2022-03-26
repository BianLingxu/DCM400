#pragma once
/**
 * @file Period.h
 * @brief The peiod of each controller
 * @author Guangyun Wang
 * @date 2022/02/22
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "DCM400HardwareInfo.h"
#include <map>
/**
 * @class CPeriod
 * @brief The singleton class for saving the period of each controller
*/
class CPeriod
{
public:
	/**
	 * @brief Get the instance of the 
	 * @return The point of class
	*/
	static CPeriod* Instance();
	/**
	 * @brief Set the memory of period
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller index
	 * @param[in] pfPeriod The start address of the memory for current controller
	 * @return Execute result
	 * - 0 Set memory successfully
	 * - -1 Have set the memory before
	*/
	int SetMemory(BYTE bySlotNo, BYTE byController, float* pfPeriod);
	/**
	 * @brief Destructor
	*/
	~CPeriod();
	/**
	 * @brief Get the period of the controller
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller index
	 * @param[in] bySeries The series index
	 * @return The period of the controller
	 * - >=0 The period
	 * - -1 Not record the controller
	 * - -2 The series index is over range
	*/
	double GetPeriod(BYTE bySlotNo, BYTE byController, BYTE bySeries);
	/**
	 * @brief Set the period of the controller
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller
	 * @param[in] bySeries The timeset series index
	 * @param[in] fPeriod The period
	 * @return Execute result
	 * - 0 Set the period successfully
	 * - -1 The timeset index is over range
	*/
	int SetPeriod(BYTE bySlotNo, BYTE byController, BYTE bySeries, float fPeriod);
private:
	/**
	 * @brief Constructor
	*/
	CPeriod();
	/**
	 * @brief Get the controller ID of the controller
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller index
	 * @return The controller ID
	*/
	inline USHORT GetControllerID(BYTE bySlotNo, BYTE byController);
private:
	std::map<USHORT, float*> m_mapPeriod;///<The period of each controller, the key is controller ID and value is its period
};

