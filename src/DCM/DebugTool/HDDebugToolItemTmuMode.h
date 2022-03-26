#pragma once
/**
 * @file HDDebugToolItemTMUMode.h
 * @brief The mode item of TMU function of debug tool
 * @author Guangyun Wang
 * @date 2020/08/31
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Control Technology Co., Ltd. Ltd.
*/
#include "HDDebugToolItem.h"
/**
 * @class CHDDebugToolItemTMUMode
 * @brief The class of the debug tool item of TMU 
*/
class CHDDebugToolItemTMUMode :
    public HDDebugToolItem
{
public:
    /**
     * @brief Constructor
     * @brief pDebugTool The point of class debug tool
    */
    CHDDebugToolItemTMUMode(HDDebugTool* pDebugTool);
    /**
     * @brief Destructor
    */
	~CHDDebugToolItemTMUMode();
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

