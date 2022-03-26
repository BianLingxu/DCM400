#pragma once
/**
 * @file HDDebugToolItemTMUMode.h
 * @brief The timeout item of TMU function of debug tool
 * @author Guangyun Wang
 * @date 2020/08/31
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd.
*/
#include "HDDebugToolItem.h"
/**
 * @class CHDDebugToolItemTMUTimeout
 * @brief The class of the debug tool timeout item of TMU
*/
class CHDDebugToolItemTMUTimeout :
    public HDDebugToolItem
{
public:
    /**
     * @brief Constructor
     * @param[in] pDebugTool The point of DebugTool
    */
    CHDDebugToolItemTMUTimeout(HDDebugTool* pDebugTool);
    /**
     * @brief Destructor
    */
    ~CHDDebugToolItemTMUTimeout();
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

