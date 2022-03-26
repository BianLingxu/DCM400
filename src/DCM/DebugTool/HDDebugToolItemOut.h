#ifndef __3BB62C76_FA6D_471A_807A_436C064BAA0E_HDDEBUGTOOLITEMOUT_H__
#define __3BB62C76_FA6D_471A_807A_436C064BAA0E_HDDEBUGTOOLITEMOUT_H__

#include "HDDebugToolItem.h"

class HDDebugToolItemOut : public HDDebugToolItem
{
public :
	HDDebugToolItemOut(HDDebugTool * debugTool);
	~HDDebugToolItemOut();

	virtual int Type() const;
	virtual const char * Name() const;

	virtual int GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const;
};

#endif /*__3BB62C76_FA6D_471A_807A_436C064BAA0E_HDDEBUGTOOLITEMOUT_H__*/