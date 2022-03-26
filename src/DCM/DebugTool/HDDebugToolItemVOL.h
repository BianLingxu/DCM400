#ifndef __B6CC4A34_E7B1_45FB_A158_5197C3061193_HDDEBUGTOOLITEMVOL_H__
#define __B6CC4A34_E7B1_45FB_A158_5197C3061193_HDDEBUGTOOLITEMVOL_H__

#include "HDDebugToolItem.h"

class HDDebugToolItemVOL : public HDDebugToolItem
{
public :
	HDDebugToolItemVOL(HDDebugTool * debugTool);
	~HDDebugToolItemVOL();

	virtual int Type() const;
	virtual const char * Name() const;

	virtual int GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const;
};

#endif /*__B6CC4A34_E7B1_45FB_A158_5197C3061193_HDDEBUGTOOLITEMVOL_H__*/