
#include <ardev/player.h>

#include "cm_objecthandler.h"
#include "cm_parameter_ardev.h"

#if HAVE_LIBLTDL
#include <ltdl.h>

typedef void (*PluginInitFn) (void);

//Loads of Plugin give an absolute path or plugin name
lt_dlhandle LoadARPlugin(const char* pluginname);

//Initialises the plugin and runs it
bool InitARPlugin(lt_dlhandle handle);

//Registers plugins with config manager to be used with aride
void RegisterPluginObjectHandlers();

#endif
