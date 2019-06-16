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
 * Name: logger.h
 * Date: 29/08/03
 * Author: Ben Moores
 * Description: Logger class. Singleton class for the logger.
 ******************************************************************************/

#ifndef __LOGGER_H
#define __LOGGER_H

#include <list>
#include <sys/time.h>
using namespace std;

class Logger {
public:
	//types of messages
	typedef enum MessageType { Time, Verbose, Notice, Warning, Error } MessageType;

	//type type of an output function
	typedef void(*OutputFunction)(MessageType type, const char *message);

	//standard log function type
	typedef void (* Log)(MessageType type, const char *format, ... );
	//extended log function type
	typedef void (* LogFL)(MessageType type, const char *file, int line, const char *format, ... );

	//struct of logging functions returned from getLogFunctions(void)
	//used by classes that want to create a new input to the logger
	typedef struct LoggerFunctions {
		Log log;
		LogFL logFL;
	} LoggerFunctions;

	//returns the singleton instance of the logger
	static Logger *Instance();

	//register a function to recieve output
	static void registerOutputFunction(OutputFunction fn);

	//get a pointer to the log functions
	static LoggerFunctions *getLogFunctions(void);

	//log functions
	static void log(MessageType type, const char *format, ...);
	static void logFL(MessageType type, const char *file, int line, const char *format, ...);

protected:
	//keep the constructor protected, so that the new operator cant be used on it. have to go through the Instance() function
	Logger();

private:
	~Logger();

	//the instance of the logger
	static Logger* theInstance;

	//maximum size of a message. anything past this will be ignored
	static int maxMessageSize;

	//sends message to all output
	static void sendToOutput(MessageType type, char *message);

	//list of output functions
	static list<OutputFunction> outputFunctionList;

	//list of logging functions. no point re-creating it every time its requested
	static LoggerFunctions loggerFunctions;

	//time of last message
	static time_t lastMessageTime;

	//print time if its different from when last message was taken
	static void checkPrintTime(void);

};

#endif
