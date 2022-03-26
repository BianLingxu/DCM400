#ifndef __A218352C_39D3_4EE7_B12B_661E1CE11DBE_HDDEBUGTOOLITEMVIL_H__
#define __A218352C_39D3_4EE7_B12B_661E1CE11DBE_HDDEBUGTOOLITEMVIL_H__

#include "HDDebugToolItem.h"

class HDDebugToolItemVIL : public HDDebugToolItem
{
public :
	HDDebugToolItemVIL(HDDebugTool * debugTool);
	~HDDebugToolItemVIL();

	virtual int Type() const;
	virtual const char * Name() const;

	virtual int GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const;
};

#endif /*__A218352C_39D3_4EE7_B12B_661E1CE11DBE_HDDEBUGTOOLITEMVIL_H__*/