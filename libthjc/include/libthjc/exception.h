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
/********************************************************
 *	Base exception class for Car Control Server.		*
 *	Header file.										*
 *														*
 *	Geoffrey Biggs										*
 ********************************************************/

#include <string>
using namespace std;

typedef enum { GENERAL, OPENGL, NETWORK, CAR } EX_TYPE;

//	The base exception class. Used to throw stuff around the Car
//	Control Server. Preferably errors. No food fights.
class Exception
{
public:
	Exception (string &newMsg, EX_TYPE newType = GENERAL)		{ message = newMsg; type = newType; }
	Exception (const char *newMsg, EX_TYPE newType = GENERAL)			{ message = newMsg; type = newType; }

	string TypeToString (void)
	{
		switch (type)
		{
		case GENERAL:
			return "General";
		case OPENGL:
			return "OpenGL";
		case NETWORK:
			return "Network";
		case CAR:
			return "Car";
		default:
			return "Unknown";
		}
	}

	string message;
	EX_TYPE type;
};
