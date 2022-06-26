

#ifndef _MOD_CURSOR
#define _MOD_CURSOR

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "resource.h"

#undef GetUserName // This is a macro in windows.h, gives issues with GetUserName() of ISystem


class CModCursor : public ISystemEventListener
{
public:
	CModCursor();
	~CModCursor();
private:
		virtual void OnSystemEvent( ESystemEvent event,UINT_PTR wparam,UINT_PTR lparam );
	HCURSOR m_cursor;
};


#endif