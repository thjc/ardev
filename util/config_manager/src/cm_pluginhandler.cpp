/* -- 2007-05-07 --
 *  ardev - an augmented reality library for robot developers
 *  Copyright 2005-2007 - Toby Collett (ardev _at_ plan9.net.nz)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#include <ardev/exception.h>
#include "cm_parameter_ardev.h"

#include "cm_pluginhandler.h"
#if HAVE_LIBLTDL

#include <stddef.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>

#include <ardev/debug.h>

#include <string>
#include <stdlib.h>
#include <unistd.h>
#include <list>

#include <dlfcn.h>
#include <ltdl.h>
#include <stdlib.h>


#include <limits.h> // for PATH_MAX

// Try to load a given plugin, using a particular search algorithm.
// Returns true on success and false on failure.
void LoadARPlugins()
{
	static int init_done = 0;

	if( !init_done )
	{
		int errors = 0;
		if((errors = lt_dlinit()))
		{
			dbg_print(ARDBG_ERR,"Error(s) initializing dynamic loader (%d, %s)",
					errors, lt_dlerror() );
			return;
		}
		else
		init_done = 1;
	}

	lt_dlhandle handle=NULL;
	vector<string> paths;

	string arpath;
	string path;

	// we got a relative path, so search for the module
	// did the user set PLAYERPATH?
	char* a = getenv("AR_PLUGIN_PATH");
	//printf("AR PLUGIN PATH:%s\n",a);
	arpath = string(a?a:"/usr/local/lib/");
	if(arpath.size()>0)
	{
		dbg_print(ARDBG_VERBOSE,"AR_PLUGIN_PATH: %s\n", arpath.c_str());
		printf("AR_PLUGIN_PATH: %s\n", arpath.c_str());
		unsigned j=0;
		for(unsigned i=0; i<arpath.length(); i++)
		{
			if(arpath[i] == ':')
			{
				paths.push_back(arpath.substr(j,i-1));
				j=i+1;
			}
		}
		paths.push_back(arpath.substr(j,arpath.length()));

		dbg_print(ARDBG_VERBOSE, "loading plugins\n");
	}

	for (unsigned i=0;i<paths.size();i++)
	{
		string library;
		string plugin;

		struct dirent *ep;
		DIR *dp;

		dp = opendir((paths[i]).c_str());
		if(dp == NULL)
		{
			dbg_print(ARDBG_WARN, "Could not open plugin dir: %s\n",(paths[i]).c_str());
			continue;
		}
			

		while ((ep = readdir(dp)))
		{
			string name = string(ep->d_name);
			if ( name.find(".so") == string::npos ) // continue if not .so
				continue;
			if ( name.find("libar_") == string::npos ) // continue if not starting libar_
				continue;
			
			string fullpath = paths[i] + '/' + name; // Doesn't hurt having an extra /

			if((handle = lt_dlopenext( fullpath.c_str() )))
			{
				if(InitARPlugin(handle))
				{
					dbg_print(ARDBG_VERBOSE, "loaded plugin: %s\n",ep->d_name);
				}
				else
				{
					dbg_print(ARDBG_WARN, "failed to load ar plugin: %s, %s\n",ep->d_name, lt_dlerror());
				}
			}
			else
			{
				dbg_print(ARDBG_VERBOSE, "not usable as plugin library: %s ltdl error: %s\n",ep->d_name, lt_dlerror());
			}

		}
		closedir(dp);
	}

}

// Try to load a given plugin, using a particular search algorithm.
// Returns true on success and false on failure.
lt_dlhandle LoadARPlugin(const char* pluginname)
{
	static int init_done = 0;

	if( !init_done )
	{
		int errors = 0;
		if((errors = lt_dlinit()))
		{
			dbg_print(ARDBG_ERR,"Error(s) initializing dynamic loader (%d, %s)",
					errors, lt_dlerror() );
			return NULL;
		}
		else
		init_done = 1;
	}

	lt_dlhandle handle=NULL;
	vector<string> paths;

	string arpath;
	string path;
	string plugin = string(pluginname);
	//unsigned int i,j;

	// allocate a buffer to put the searched paths in so we can just display the error at the end
	// rather than during plugin loading
	// see if we got an absolute path
	if(plugin[0] == '/' || plugin[0] == '~')
	{
		paths.push_back(plugin);
	}
	else
	{
		// we got a relative path, so search for the module
		// did the user set PLAYERPATH?
		char* a = getenv("AR_PLUGIN_PATH");
		arpath = string(a?a:"");
		if(arpath.size()>0)
		{
			dbg_print(ARDBG_INFO,"AR_PLUGIN_PATH: %s\n", arpath.c_str());
			unsigned j=0;
			for(unsigned i=0; i<arpath.length(); i++)
			{
				if(arpath[i] == ':')
				{
					paths.push_back(arpath.substr(j,i-1) + "lib" + plugin);
					j=i+1;
				}
			}
			paths.push_back(arpath.substr(j,arpath.length()) + "lib" + plugin);

			dbg_print(ARDBG_INFO, "loading plugin %s", pluginname);
		}
	}
	for (unsigned i=0;i<paths.size();i++)
	{
		if((handle = lt_dlopenext(paths[i].c_str())))
		{
			break;
		}
	}

	if(!handle)
	{
		dbg_print(ARDBG_ERR,"failed to load plugin %s, tried paths:",pluginname);
		for (unsigned i=0;i<paths.size();i++)
			dbg_print(ARDBG_ERR,"\t%s\n", paths[i].c_str());
	}
	else
		dbg_print(ARDBG_INFO,"loaded plugin %s\n",pluginname);

	return handle;
}


// Initialise a driver plugin
bool InitARPlugin(lt_dlhandle handle)
{
	PluginInitFn initfunc;
	// Invoke the initialization function
	if(handle)
	{
		dbg_print(ARDBG_VERBOSE, "invoking ar_plugin_init()...");

		initfunc = (PluginInitFn)lt_dlsym(handle,"ar_plugin_init");
		if( !initfunc )
		{
			dbg_print(ARDBG_VERBOSE,"failed to resolve ar_plugin_init: %s\n", lt_dlerror());
			return(false);
		}
		dbg_print(ARDBG_VERBOSE, "success\n");
		(*initfunc)();

		return(true);
	}
	else
		return(false);
}

void RegisterPluginObjectHandlers()
{
	dbg_print(ARDBG_INFO,"Registering Plugin Object Handlers\n");

	LoadARPlugins();
}

#endif
