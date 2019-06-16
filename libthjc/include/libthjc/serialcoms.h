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
#ifndef _SERIALCOMS_H_
#define _SERIALCOMS_H_

#include <termios.h>

class SerialComs {
public:
	SerialComs();
	~SerialComs();

	const char *SetDevice(const char *val);
	const char *GetDevice(void);
	int SetBitRate(int val) {BitRate = val; return val;};
	int GetBitRate(void) {return BitRate;};
	int SetMode(int val) {Mode = val; return val;};
	int GetMode(void) {return Mode;};
	
	bool GetIsOpen(void) {return IsOpen;};
	
	bool Open(void);
	void Close(void);

	/// returns true if data is available on the port or becomes available within timeout seconds
	bool DataAvailable(double aTimeout=0);
	int ReadData(char * Buffer,const int maxLength);
	int ReadUntilData(char * Buffer,const int Length);
	void WriteData(const char *buffer, const int len=0);

private:
	//file descriptor of the serial port
	int FD;

	//device name, e.g. "/dev/ttys0"
	char *Device;
	//bitrate of the port
	int BitRate;
	//bits/parity/stopbits
	int Mode;

	//true if the device is open
	bool IsOpen;
	
	//initial IO settings, we revert to them on close
	struct termios InitialIOS;
	
};


#endif //_SERIALCOMS_H_
