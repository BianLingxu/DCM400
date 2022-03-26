#pragma once
/**
 * @file FPGAA.h
 * @brief Include the class of CFPGAA
 * @detail The class provide the read and write operation of FPGAA
 * @author Guangyun Wang
 * @date 2020/05/31
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "BaseOperation.h"
/**
 * @class CFPGAA
 * @brief The pure virtual operation of controller
*/
class CFPGAA :
	public CBaseOperation
{
public:
	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number of board
	*/
	CFPGAA(BYTE bySlotNo);
	/**
	 * @brief Set the controller index
	 * @param[in] byControllerIndex The controller index
	 * @return Execute result
	 * - 0 Set the controller index successfully
	 * - -1 The controller index is over range
	*/
	int SetControllerIndex(BYTE byControllerIndex);
protected:
	/**
	 * @enum MODULE_TYPE
	 * @brief The module type
	*/
	enum class MODULE_TYPE
	{
		SYS_MODULE = 0,///<The system register
		COM_MODULE,///<The COM module
		FUN_MODULE,///<The function module
		TMU_MODULE,///<The TMU module
		CAL_MODULE,///<The calibration module
	};
	/**
	 * @brief Set the module type
	 * @param[in] ModuleType The module type
	 * @return Execute result
	 * - 0 Set module type successfully
	 * - -1 The module type is not supported
	*/
	int SetModuleType(MODULE_TYPE ModuleType);
	/**
	 * @brief Read data
	 * @param[in] usREGAddress The register address
	 * @return The data read
	*/
	ULONG Read(USHORT usREGAddress);
	/**
	 * @brief Write data to module
	 * @param[in] usREGAddress The register address
	 * @param[in] ulData The data will be written
	 * @return Execute result
	 * - 0 Write data successfully
	*/
	int Write(USHORT usREGAddress, ULONG ulData);
private:
	MODULE_TYPE m_byFunctionAddress;///<The function address of the module
};

