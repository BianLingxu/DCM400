#pragma once
/**
 * @file ATE305.h
 * @brief Include the operation of ATE305
 * @author Guangyun Wang
 * @date 2020/07/10
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "Register.h"
/**
 * @class CAD7606
 * @brief The operation of AD7606 chip
*/
class CAD7606 : 
	public CRegister
{
public:
	/**
	 * @class SAMP_MODE
	 * @brief The sample mode
	*/
	enum class SAMP_MODE
	{
		SINGLE = 0,///<Single sample
		MULTIPLE,///<Multi-sample
		CONTINOUS,///<Continuous
	};

public:
	/**
	 * @brief Constructor
	 * @param[in] bySlotNo The slot number
	*/
	CAD7606(BYTE bySlotNo);
	/**
	 * @brief Write data
	 * @param[in] usRegisterAddress The register address
	 * @param[in] ulData The data written
	 * @return Execute result
	 * - 0 Write data successfully
	*/
	int Write(USHORT usRegisterAddress, ULONG ulData);
	/**
	 * @brief Read data
	 * @param[in] usChannel The channel number
	 * @param[in] uSampDepth The sample depth
	 * @param[out] pulDataBuff The memory address for saving data
	 * @return Execute result
	 * - 0 Read data successfully
	 * - -1 Measuring now
	*/
	int Read(USHORT usChannel, UINT uSampDepth, ULONG* pulDataBuff);
};

