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
#ifndef ARIDE_EXCEPTION_H
#define ARIDE_EXCEPTION_H

#include <stdio.h>
#include <stdexcept>

/* For each entry in exception type make sure and entry is added in Exception string in aride_exception.cpp
   Also make sure that ARIDE_UNKNOWN_EXCEPTION is the last entry in the enumeration */
enum ExceptionType
{
	ARDEV_FATAL_EXCEPTION,
	ARDEV_NO_FILE_SPECIFIED,
	ARDEV_FILE_NOT_FOUND,
	ARDEV_XML_PARSE_FAILED,
	ARDEV_ALLOC_ERROR,
	ARDEV_FILE_OPEN_FAILED,
	ARDEV_BAD_PARAMETER,
	ARDEV_NO_HANDLER,
	ARDEV_CLASS_NOT_FOUND,
	ARDEV_BAD_GUID,
	
	ARDEV_UNKNOWN_EXCEPTION
};

extern const char * ExceptionString[];

class aride_exception : public std::runtime_error
{
	public:
		aride_exception(ExceptionType _Type,const char* _Function="",int _Line=0,void * _Extra=NULL) 
			: std::runtime_error(ExceptionString[Type])
		{
			Type=_Type;
			Function=_Function;
			Line=_Line;
			Extra=_Extra;
		};
	
		ExceptionType Type;
		const char * Function;
		int Line;
	
		void * Extra;
	
		const void Print() const
		{
			if (Type >= ARDEV_FATAL_EXCEPTION && Type < ARDEV_UNKNOWN_EXCEPTION)
				fprintf(stderr,"Exception %d in %s:%d. %s\n",Type,Function,Line,ExceptionString[Type]);
			else
				fprintf(stderr,"Exception %d in %s:%d. Unknown Exception\n",Type,Function,Line);
		};
};

#endif
