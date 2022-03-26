#pragma once
/**
 * @file HDDebugToolItemPinMode
 * @brief The debug tool item for pin mode
 * @author Guangyun Wang
 * @date 2021.06.07
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "HDDebugToolItem.h"
/**
 * @class CHDDebugToolItemPinMode
 * @brief The debug tool item for pin mode of the pin
*/
class CHDDebugToolItemPinMode
	: public HDDebugToolItem
{
public:
	/**
	 * @brief Constructor
	 * @param[in] pDebugTool The point of the debug tool
	*/
	CHDDebugToolItemPinMode(HDDebugTool* pDebugTool);
	/**
	 * @brief Destructor
	*/
	~CHDDebugToolItemPinMode();
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

