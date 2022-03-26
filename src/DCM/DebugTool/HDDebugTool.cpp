#include "HDDebugTool.h"
#include <string>
#include <windows.h>
#include "STSCoreFx.h"
#include "Sts8100Fx.h"
#include "SM8213.H"
#include "IHDDebugToolCallBackHelp.h"
#include "../HDModule.h"

#include "HDDebugToolItemVIH.h"
#include "HDDebugToolItemVIL.h"
#include "HDDebugToolItemVOH.h"
#include "HDDebugToolItemVOL.h"
#include "HDDebugToolItemOut.h"
#include "HDDebugToolItemMcuMeas.h"
#include "HDDebugToolItemPmuMode.h"
#include "HDDebugToolItemPmuForce.h"
#include "HDDebugToolItemPmuIRange.h"
#include "HDDebugToolItemPmuSamples.h"
#include "HDDebugToolItemPmuInterval.h"
#include "HDDebugToolItemPmuMeas.h"

#include "HDDebugToolItemTMUUnit.h"
#include "HDDebugToolItemTMUHoldOffNum.h"
#include "HDDebugToolItemTMUHoldOffTime.h"
#include "HDDebugToolItemTMUMode.h"
#include "HDDebugToolItemTMUTriggerEdge.h"
#include "HDDebugToolItemTMUSampleNum.h"
#include "HDDebugToolItemTMUTimeout.h"
#include "HDDebugToolItemTMUMeas.h"
#include "HDDebugToolItemPinMode.h"
#include "HDDebugToolItemSlot.h"

HDDebugTool::HDDebugTool(HDModule * parent)
    : m_pParent(parent)
    , m_pCallBackHelp(STS_NULL)
    , m_vecItems()
{
    Init();
}

HDDebugTool::~HDDebugTool()
{
	for (auto& Item : m_vecItems)
	{
		if (nullptr != Item)
		{
			delete Item;
			Item = nullptr;
		}
	}
    m_vecItems.clear();
}

int HDDebugTool::QueryInterface(const char * interfaceName, void ** ptr)
{
    int nRet = 0;
    do 
    {
        if (STS_NULL == interfaceName || STS_NULL == ptr)
        {
            nRet = -1;
            break;
        }
        const std::string strInterfaceName(interfaceName);
        if (strInterfaceName == std::string("IHDDebugTool"))
        {
            IHDDebugTool *p = this;
            *ptr = p;
            break;
        }
        if (strInterfaceName == std::string("IHDDebugToolCommon"))
        {
            IHDDebugToolCommon *p = this;
            *ptr = p;
            break;
        }

        nRet = -1;
    } while (false);
    return nRet;
}

int HDDebugTool::SetCallBackHelp(IHDDebugToolCallBackHelp * help)
{
    m_pCallBackHelp = help;
    return 0;
}

int HDDebugTool::GetSites(int & sites) const
{
	USHORT pinNum = 0;
	USHORT siteNum = 0;
	dcm_getPinNumAndsiteNum(pinNum, siteNum);
	sites = siteNum;
	
    return 0;
}

int HDDebugTool::GetValidLogicChannels(int site, STSVector<LoagicChannel> & channelsBuf) const
{
    channelsBuf.clear();
    if (STS_NULL == m_pCallBackHelp)
    {
        return -1;
    }
	USHORT usPinCount = dcm_GetPinCount();
	for (int nPinNo = 0; nPinNo < usPinCount; ++nPinNo)
	{
		short sChannel = 0;
		if (dcm_getPinChannel(nPinNo, site, sChannel))
		{
			const char* strPinName = dcm_getPinName(nPinNo);
			LoagicChannel lgcChannel;
			lgcChannel.channelID = nPinNo;
			sts_strcpy(lgcChannel.channelName, STS_ARRAY_SIZE(lgcChannel.channelName), strPinName);
			channelsBuf.push_back(lgcChannel);
		}
	}
    
    return channelsBuf.size();
}

int HDDebugTool::GetItems(STSVector<IHDDebugToolItem *> & items) const
{
	for (auto& Item : m_vecItems)
	{
		items.push_back(Item);
	}
    return items.size();
}

int HDDebugTool::ApplayRun()
{
    return -1;
}

int HDDebugTool::ApplayRun(int logicChannel, int site)
{
    return -1;
}

void HDDebugTool::Init()
{
    IHDDebugToolItem *pItem = STS_NULL;

	pItem = new CHDDebugToolItemPinMode(this);
	if (nullptr != pItem)
	{
		m_vecItems.push_back(pItem);
	}

	pItem = new CHDDebugToolItemSlot(this);
	if (nullptr != pItem)
	{
		m_vecItems.push_back(pItem);
	}

	//SetPinLevel
	pItem = new HDDebugToolItemVIH(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}
	pItem = new HDDebugToolItemVIL(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}
	pItem = new HDDebugToolItemVOH(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}
	pItem = new HDDebugToolItemVOL(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}

	//Connect / Disconnect
	pItem = new HDDebugToolItemOut(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}

	// MCU Meas
	pItem = new HDDebugToolItemMcuMeas(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}

	//SetPPMU
	pItem = new HDDebugToolItemPmuMode(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}
	pItem = new HDDebugToolItemPmuForce(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}
	pItem = new HDDebugToolItemPmuIRange(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}

	//PPMUMeasure
	pItem = new HDDebugToolItemPmuSamples(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}
	pItem = new HDDebugToolItemPmuInterval(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}

	//PPMU Meas
	pItem = new HDDebugToolItemPmuMeas(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}

	///<TMU
	pItem = new CHDDebugToolItemTMUUnit(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}
	pItem = new CHDDebugToolItemTriggerEdge(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}
	pItem = new CHDDebugToolItemTMUHoldOffTime(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}
	pItem = new CHDDebugToolItemTMUHoldOffNum(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}
	pItem = new CHDDebugToolTMUSampleNum(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}
	pItem = new CHDDebugToolItemTMUMode(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}
	pItem = new CHDDebugToolItemTMUTimeout(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}
	pItem = new CHDDebugToolItemTMUMeas(this);
	if (STS_NULL != pItem)
	{
		m_vecItems.push_back(pItem);
	}
}

AccoTest::HardwareType HDDebugTool::Type() 
{
    return m_pParent->Type();
}