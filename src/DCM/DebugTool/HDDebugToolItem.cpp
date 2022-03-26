#include "HDDebugToolItem.h"
#include <string>
#include "STSCoreFx.h"
#include <windows.h>
#include <string>
#include "Sts8100.h"
#include "SM8213.H"

HDDebugToolItem::HDDebugToolItem(HDDebugTool * debugTool)
    : m_pDebugTool(debugTool)
{

}

HDDebugToolItem::~HDDebugToolItem()
{

}

int HDDebugToolItem::QueryInterface(const char * interfaceName, void ** ptr)
{
    int nRet = -1;
    do 
    {
        if (STS_NULL == interfaceName || STS_NULL == ptr)
        {
            break;
        }
        const std::string str = interfaceName;

        const std::string strIHDDebugToolItemCommon = "IHDDebugToolItemCommon";
        if (str == strIHDDebugToolItemCommon)
        {
            IHDDebugToolItemCommon *p = this;
            *ptr = p;
            nRet = 0;
            break;
        }
        const std::string strIHDDebugToolItem = "IHDDebugToolItem";
        if (str == strIHDDebugToolItem)
        {
            IHDDebugToolItem *p = this;
            *ptr = p;
            nRet = 0;
            break;
        }
        break;
    } while (false);

    return nRet;
}

const char * HDDebugToolItem::Caption() const
{
    return Name();
}

AccoTest::DataType HDDebugToolItem::DataType() const
{
    return AccoTest::DT_Unknown;
}

int HDDebugToolItem::GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const
{
    return -1;
}

int HDDebugToolItem::GetDataSizeAndDataType(int site, int logicChannel, STSVariant::Type & dataType, int & dataSize, STSVariant::Type & markType, int & markSize) const
{
	return -1;
}