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
#include <ardev/debug.h>
#include <ardev/ardevconfig.h>

//#include <qapplication.h>
#include "mainwindow.h"

#ifdef HAVE_PLAYER
#include <cm_playerhandlers.h>
#include <libplayerc++/playerc++.h>
#include <libplayerc++/playererror.h>
using namespace PlayerCc;
#endif
#include <cm_objecthandler.h>
#include <cm_artoolkithandlers.h>
#include <cm_artoolkitplushandlers.h>
#ifdef HAVE_OPENCV
#include <cm_opencvblobhandlers.h>
#endif
#if HAVE_LIBLTDL
#include <cm_pluginhandler.h>
#endif



//int DebugLevel = ARDBG_INFO;
int DebugLevel = ARDBG_VERBOSE;
MainWindow * theMainWindow = NULL;

int main(int argc, const char * argv[])
{
#ifdef HAVE_PLAYER
	try
	{
		PlayerClient p("sdfgsdf");
		printf("Starting up...\n");
	}
	catch (const PlayerError & e)
	{
		printf("Got a PlayerError\n");

	}
	catch (...)
	{
		printf("Got a generic\n");

	}
	printf("Cleared the handlers\n");
#endif

	try
	{
		ARDevInit(argc, argv);
		
		// Set the debug level before we do anything
		ARDev::DebugLevel = ARDBG_INFO;
		for (int i = 1; i < argc; ++i)
		{
			// check if debug level specified
			if (strcmp(argv[i],"-d")==0)
			{
				if (i+1 < argc)
					ARDev::DebugLevel = atoi(argv[++i]);
				dbg_print(ARDBG_WARN,"Debug Level set to %d\n", ARDev::DebugLevel);
			}
		}

		// first we register our object and type handers
		RegisterDefaultObjectHandlers();
#ifdef HAVE_PLAYER
		RegisterPlayerObjectHandlers();
#endif
#ifdef HAVE_LIBAR
		RegisterARToolKitObjectHandlers();
#endif
#ifdef HAVE_OPENCV
		RegisterOpenCVBlobObjectHandlers();
#endif
#ifdef HAVE_ARTOOLKITPLUS
		RegisterARToolKitPlusObjectHandlers();
#endif
#if HAVE_LIBLTDL
		RegisterPluginObjectHandlers();
#endif
		// then create the application
		QApplication a( argc, const_cast<char**>( argv ) );
		theMainWindow = new MainWindow();
		assert(theMainWindow);
		theMainWindow->Initialise(argc, const_cast<char**>( argv ) );
		//TypeHandler::Parent = mw;
		theMainWindow->show();
		a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );


		return a.exec();
	}
	catch (const aride_exception & e)
	{
		e.Print();
		fprintf(stderr,"Unhandled exception, Application Terminated\n");
		return 1;
	}
#ifdef HAVE_PLAYER
	catch (const PlayerCc::PlayerError & e)
	{
		cout << e;
		fprintf(stderr,"Unhandled exception, Application Terminated\n");
		return 1;
	}
#endif
	catch (...)
	{
		fprintf(stderr,"Unknown exception caught\n");
	}
}
