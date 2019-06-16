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
#include <qapplication.h>
#include "calibapp.h"

int main(int argc, char * argv[])
{
//	try
	//{
		// then create the application
		QApplication a( argc, argv );
		CalibApp * theMainWindow = new CalibApp(argc, argv, NULL,0);
	
		// These should store the cube.offest and the cube.points filenames...
		// If not specified then defaults will be used
//		theMainWindow->argc = argc;
//		theMainWindow->argv = argv;
		//TypeHandler::Parent = mw;
//		theMainWindow->LoadCalibPoints();
		theMainWindow->show();
		a.connect( &a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()) );
		
		return a.exec();
/*	}
	catch 
	{
		fprintf(stderr,"Unhandled exception, Application Terminated\n");
		return 1;
	}*/
}
