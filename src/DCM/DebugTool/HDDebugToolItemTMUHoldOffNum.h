#pragma once
/**
 * @file HDDebugToolTMUHoldOffTime.h
 * @brief The debug tool of the TMU hold off number item
 * @author Guangyun Wang
 * @date 2020/08/31
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd. Ltd.
*/
#include "HDDebugToolItem.h"
/**
 * @class CHDDebugToolItemTMUHoldOffTime
 * @brief The debug tool class of the TMU item
*/
class CHDDebugToolItemTMUHoldOffNum :
    public HDDebugToolItem
{
public:
    /**
     * @brief Constructor
     * @param[in] pDebugTool The point of class debug tool
    */
    CHDDebugToolItemTMUHoldOffNum(HDDebugTool* pDebugTool);
    /**
     * @brief Destructor
    */
    ~CHDDebugToolItemTMUHoldOffNum();
	/**
	 * @brief Get the type of the debug tool item
	 * @return The type value
	*/
	virtual int Type() const;
	/**
	 * @brief Get the item name
	 * @return The item name
	*/
	virtual const char* Name() const;
	/**
	 * @brief Get the data of the channel
	 * @param[in] nSiteNo The site number
	 * @param[in] nLogicChannel The logic channel
	 * @param[in] Data The data of the mode
	 * @param[in] Mark The mark of th mode
	 * @return The data of the channel
	*/
	virtual int GetData(int nSiteNo, int nLogicChannel, STSVariant& Data, STSVariant& Mark) const;
};

