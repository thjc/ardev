/* -- 2007-05-07 --
 * libthjc - utility library
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
/******************************************************************************
 * Name: logger_out_console.cpp
 * Date: 29/08/03
 * Author: Ben Moores
 * Description: logger output class that outputs to the console
 ******************************************************************************/

#include <libthjc/logger_out_console.h>
#include <libthjc/logger.h>
#include <stdio.h>

#ifdef WIN32
	#include <windows.h>
	#include <io.h>
	#include <fcntl.h>
#endif // WIN32

//initialise the static member
bool LoggerOutConsole::once=false;


LoggerOutConsole::LoggerOutConsole() {
	//register the output function with the logger
	Logger::Instance()->registerOutputFunction(output);
}

LoggerOutConsole::~LoggerOutConsole() {
}

void LoggerOutConsole::output(Logger::MessageType type, const char *message) {
	printf ("%s",message);
}


void LoggerOutConsole::Create(bool createConsole) {
	//only do this once
	if(!once) {
		new LoggerOutConsole();
		once=true;

#ifdef WIN32
		if(createConsole) {
			//create the console
			AllocConsole();
			SetConsoleTitle("Plugin Debug Messages...");
			//reassign stdout to our new console
			int hCrt=_open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE),_O_TEXT);
			FILE *hf = _fdopen(hCrt, "w");
			*stdout = *hf;
			setvbuf(stdout, NULL, _IONBF, 0);
		}
#endif // WIN32

	}

}
