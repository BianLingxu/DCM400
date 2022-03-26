#pragma once
/**
 * @file HDDebugToolItemSlot.h
 * @brief The item for the slot of DCM pin in DebugTool
 * @author Guangyun Wang
 * @date 2021/07/14
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Co., Ltd.
*/
#include "HDDebugToolItem.h"
/**
 * @class CHDDebugToolItemSlot
 * @brief The debug tool item for the slot and channel of pin
*/
class CHDDebugToolItemSlot :
    public HDDebugToolItem
{
public:
	/**
	 * @brief Constructor
	 * @param[in] pDebugTool The point of the debug tool
	*/
	CHDDebugToolItemSlot(HDDebugTool* pDebugTool);
	/**
	 * @brief Destructor
	*/
	~CHDDebugToolItemSlot();
	/**
	 * @brief Get the item type
	 * @return The type value
	*/
	virtual int Type() const;
	/**
	 * @brief Get the item name
	 * @return The name of the item
	*/
	virtual const char* Name() const;
	virtual int GetData(int nSiteNo, int nPinNo, STSVariant& data, STSVariant& mark) const;
};

