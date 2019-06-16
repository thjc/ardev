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
 * Name: logger.cpp
 * Date: 29/08/03
 * Author: Ben Moores
 * Description: Logger class
 ******************************************************************************/

#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <list>

#include <libthjc/logger.h>

#ifndef WIN32
#define _snprintf snprintf
#define _vsnprintf vsnprintf
#endif

//for the stl list
using namespace std;

//initialise the instance
Logger *Logger::theInstance=NULL;

//initialize static members
int Logger::maxMessageSize=2048;
list<Logger::OutputFunction> Logger::outputFunctionList;
Logger::LoggerFunctions Logger::loggerFunctions;
time_t Logger::lastMessageTime=0;

Logger::Logger() {
	//now that an instance exists
	//fill the function struct that can be passed to the loaded dll's
	loggerFunctions.log=log;
	loggerFunctions.logFL=logFL;

}


Logger *Logger::Instance() {
	//if the logger hasnt been created yet, create a new one and return it
	if(theInstance==NULL) {
		theInstance=new Logger();
	}

	return theInstance;
}

Logger::~Logger() {

}


void Logger::log(MessageType type, const char *format, ...) {
    va_list argptr;	//variable argument list
	char *buffer;	//buffer for formatting message into

	//create the buffer
	buffer = new char[maxMessageSize+1];	//+1 for terminating null
	memset(buffer,' ',maxMessageSize+1);  //fill the message with ' '

	//get the start of the variable argument list
    va_start(argptr,format);

	//print the message into the buffer (after the time)
    _vsnprintf(buffer,maxMessageSize,format, argptr);
	strcat(buffer,"\n");


	//check to see if time should be printed
	checkPrintTime();

	//send message
	sendToOutput(type, buffer);

	//clean up after the variable argument list
	va_end(argptr);

	//free the message buffer
	delete buffer;
}

void Logger::logFL(MessageType type, const char *file, int line, const char *format, ...) {
    va_list argptr;	//variable argument list
	char *buffer;	//buffer for formatting message into
	char linebuffer[11]={"          "};	//buffer for converting line number to string

	//create the buffer
	buffer = new char[maxMessageSize+1];	//+1 for null terminator
	memset(buffer,' ',maxMessageSize+1);	//fill the message with ' '

	//get the start of the variable argument list
    va_start(argptr,format);

	//put file name and line number in
	_snprintf(linebuffer,10,"%d",line);
	strcpy(buffer,file);
	strcat(buffer,":");
	strcat(buffer,linebuffer);
	strcat(buffer," ");

	//print the message into the buffer (after the time)
    _vsnprintf(buffer+(int)strlen(buffer),maxMessageSize,format, argptr);
	strcat(buffer,"\n");

	//check to see if time should be printed
	checkPrintTime();

	//send message
	sendToOutput(type, buffer);

	//clean up after the variable argument list
	va_end(argptr);

	//free the message buffer
	delete buffer;
}

//return pointer to the log functions
Logger::LoggerFunctions *Logger::getLogFunctions(void) {
	return &loggerFunctions;
}

//push an output function onto the list
void Logger::registerOutputFunction(Logger::OutputFunction fn) {
	if(fn!=NULL) {
		outputFunctionList.push_back(fn);
	}
}

//sends the message to all output functions
void Logger::sendToOutput(MessageType type, char *message) {
	list<OutputFunction>::iterator itr;
	OutputFunction fn;	//output function pointer

	for(itr=outputFunctionList.begin();itr!=outputFunctionList.end();itr++) {
		fn=*itr;
		fn(type,message);
	}
}

//prints the time if needed
void Logger::checkPrintTime(void) {
	time_t currentTime;
	char buffer[128];
	char buffer2[256];

	time(&currentTime);

	//if times differ
	if(currentTime!=lastMessageTime) {
		//print to string
	    strftime(buffer,127,"%F",localtime(&currentTime));
		_snprintf(buffer2,255," ----- %s ----- \n",buffer);
		//sent to output functions
		sendToOutput(Logger::Time,buffer2);
	}

	//save this time
	lastMessageTime=currentTime;
}
