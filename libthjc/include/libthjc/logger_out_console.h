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
 * Name: logger_out_console.h
 * Date: 29/08/03
 * Author: Ben Moores
 * Description: logger output class that outputs to the console
 ******************************************************************************/

#ifndef __LOGGER_OUT_CONSOLE_H
#define __LOGGER_OUT_CONSOLE_H

#include <libthjc/logger.h>

//singleton class for console output from logger class
class LoggerOutConsole {
public:
	//creates an instance of the class.
	static void Create(bool createConsole=false);

private:
	//keep these private so extra instances cant be made
	LoggerOutConsole();
	~LoggerOutConsole();

	//the output function
	static void output(Logger::MessageType type, const char *message);

	//stores whether ad instance has been made
	static bool once;

};

#endif
