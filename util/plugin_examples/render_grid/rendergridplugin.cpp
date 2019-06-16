
#include <stdio.h>

#include <cm_registry.h>
#include "render_grid.h"

#define REG(X,Y,Z) dbg_print(ARDBG_INFO,"\t%s\n",X);ObjectHandlerRegistry[X] = ObjectRegistryEntry(Y,Z);

extern "C"
{
	void ar_plugin_init()
	{
		REG("RenderGrid",RenderGridHandler::CreateHandler,ARIDE_RENDER);
	}
}
