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
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <termios.h>
#include <math.h>

#include <libthjc/serialcoms.h>
#include <libthjc/logger.h>
#include <stdlib.h>

SerialComs::SerialComs() {

	FD = -1;
	Device = NULL;
	BitRate = B57600;
	Mode = CS8;

	IsOpen = false;


}

SerialComs::~SerialComs() {
	Close();
	free(Device);
	Device = NULL;

}

//////////////////////
//getters and setters

const char *SerialComs::SetDevice(const char *val) {
	if(Device != NULL) {
		delete [] Device;
	}

	Device = strdup(val);

	return Device;
}

const char *SerialComs::GetDevice(void) {
	return Device;
}

//////////////////
//serial functions

//open serial port
//returns true if port opened
bool SerialComs::Open(void) {
    struct termios newtio;

	//if already open, close it
	if(GetIsOpen()) {
		Close();
	}


	//check we have a device set
	if(GetDevice() == NULL) {
		Logger::logFL(Logger::Error,__FILE__,__LINE__,"Device not set, so cant open it\n");
		return false;
	}


	//open device
    FD = open(GetDevice(), O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);

    if (FD < 0) {
    	Logger::logFL(Logger::Error,__FILE__,__LINE__,"Failed to open com port %s\n",GetDevice());
    	return false;
    }

	// save the current io settings so we can revert to them later
	tcgetattr(FD, &InitialIOS);

    //setup the newtio struct
    bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
 	newtio.c_cflag = Mode | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;
	newtio.c_lflag = 0;

	// activate new settings
	tcflush(FD, TCIFLUSH);
	if (cfsetispeed(&newtio, BitRate) < 0 || cfsetospeed(&newtio, BitRate) < 0)
	{
		Logger::logFL(Logger::Error,__FILE__,__LINE__,"Failed to set serial baud rate: %d\n", BitRate);
		tcsetattr(FD, TCSANOW, &InitialIOS);
		close(FD);
		FD = -1;
		return false;
	}

	tcsetattr(FD, TCSANOW, &newtio);
	tcflush(FD, TCIOFLUSH);

	// clear the input buffer in case junk data is on the port
	usleep(10000);
	tcflush(FD, TCIFLUSH);

	IsOpen=true;

	return true;
}

void SerialComs::Close(void) {
	if(!IsOpen) {
		return;
	}

	// restore old port settings
	if (FD > 0)
	{
		tcsetattr(FD, TCSANOW, &InitialIOS);
		close(FD);
	}

	IsOpen = false;

	return;
}

// returns true if data is available on the port, or becomes availble within timout
bool SerialComs::DataAvailable(double aTimeout) {
	fd_set readFS;
	struct timeval timeout;
	int retval;

	if(!GetIsOpen()) {
		Logger::logFL(Logger::Error,__FILE__,__LINE__,"No data available because port not open\n");
		return false;
	}

	//setup the structs
	FD_ZERO(&readFS);
	FD_SET(FD, &readFS);
	timeout.tv_sec = static_cast<unsigned int> (trunc(aTimeout));
	timeout.tv_usec = static_cast<unsigned int> ((aTimeout - timeout.tv_sec) * 1000000);

	retval = select(FD+1,&readFS,NULL,NULL,&timeout);
	if(retval>0) {
		return true;
	}

	return false;
}

int SerialComs::ReadData(char * Buffer,const int maxLength) {
	if(!GetIsOpen()) {
		Logger::logFL(Logger::Error,__FILE__,__LINE__,"Cant read data because port isnt open\n");
		return -1;
	}

//	char *readData = new char[maxLength];

	int length = read(FD,Buffer,maxLength);

	return length;
}

int SerialComs::ReadUntilData(char * Buffer,const int Length) {
	if(!GetIsOpen()) {
		Logger::logFL(Logger::Error,__FILE__,__LINE__,"Cant read data because port isnt open\n");
		return -1;
	}

	for(int Current = 0; Current < Length; Current += read(FD,&Buffer[Current],Length-Current));

	return Length;
}

void SerialComs::WriteData(const char *buffer, const int len) {
	if(!GetIsOpen()) {
		Logger::logFL(Logger::Error,__FILE__,__LINE__,"Cant read data because port isnt open\n");
	}

	write(FD,buffer,len);
}


