#ifndef __A5A7F7C6_DBC1_40EF_8474_F8EEF54E75E6_HDDEBUGTOOLITEMVOH_H__
#define __A5A7F7C6_DBC1_40EF_8474_F8EEF54E75E6_HDDEBUGTOOLITEMVOH_H__

#include "HDDebugToolItem.h"

class HDDebugToolItemVOH : public HDDebugToolItem
{
public :
	HDDebugToolItemVOH(HDDebugTool * debugTool);
	~HDDebugToolItemVOH();

	virtual int Type() const;
	virtual const char * Name() const;

	virtual int GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const;
};

#endif /*__A5A7F7C6_DBC1_40EF_8474_F8EEF54E75E6_HDDEBUGTOOLITEMVOH_H__*/