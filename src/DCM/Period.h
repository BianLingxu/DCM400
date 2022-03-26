#pragma once
#include <windows.h>
#include <map>
#include "HardwareInfo.h"
/**
 * @file Period.h
 * @brief The peiod of each controller
 * @author Guangyun Wang
 * @date 2020/08/28
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd. Ltd.
*/
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
	 * @brief Destructor
	*/
	~CPeriod();
	/**
	 * @brief Get the period of the controller
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller index
	 * @param[in] byTimeset The timeset index
	 * @return The period of the controller
	 * - >=0 The period
	 * - -1 Not record the controller
	 * - -2 The timeset index is over range
	*/
	double GetPeriod(BYTE bySlotNo, BYTE byController, BYTE byTimeset);
	/**
	 * @brief Set the period of the controller
	 * @param[in] bySlotNo The slot number
	 * @param[in] byController The controller
	 * @param[in] byTimeset The timeset index
	 * @param[in] fPeriod The period
	 * @return Execute result
	 * - 0 Set the period successfully
	 * - -1 The timeset index is over range
	*/
	int SetPeriod(BYTE bySlotNo, BYTE byController, BYTE byTimeset, float fPeriod);
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
	inline UINT GetControllerID(BYTE bySlotNo, BYTE byController);

private:
	std::map<UINT, float*> m_mapPeriod;///<The period of each controller, the key is controller ID and value is its period
};

