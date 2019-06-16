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
 * Misc Useful functions
 ********************************************************/

#ifndef _LIBTHJC_MISC_H
#define _LIBTHJC_MISC_H

#include <sys/time.h>
#include <libthjc/geometry.h>
#include <list>
using namespace std;

//////////////////////////////////////////////////////////////
//	Functions
//////////////////////////////////////////////////////////////

void PrintList (char *listTitle, list<int> data);
void PrintList (char *listTitle, list<Point2D> data);
void PrintList (char *listTitle, list<Vector3D> data);
void PrintList (char *listTitle, list<Line> data);

class StopWatch
{
	public:
		StopWatch()
		{	
			HaveAFrameRate = false;
			FrameRateAverageCount = 1;
			CurrentCount = 0;
			FrameRateAccumulator = 0;
			if(gettimeofday(&Before,NULL))
				throw "Get time of day Failed";
		}		

		StopWatch(int aFrameRateAverageCount)
		{	
			HaveAFrameRate = false;
			FrameRateAverageCount = aFrameRateAverageCount;
			CurrentCount = 0;
			FrameRateAccumulator = 0;
			if(gettimeofday(&Before,NULL))
				throw "Get time of day Failed";
		}
		
		double GetElapsedDouble()			
		{	
			struct timeval Now;
			if(gettimeofday(&Now,NULL))
				throw "Get time of day Failed";
			double ret = static_cast<double> (Now.tv_sec - Before.tv_sec) * 1000000.0 + static_cast<double>(Now.tv_usec - Before.tv_usec);
			Before = Now;
			return ret;
		}

		double GetElapsedSeconds()			
		{	
			struct timeval Now;
			if(gettimeofday(&Now,NULL))
				throw "Get time of day Failed";
			double ret = static_cast<double> (Now.tv_sec - Before.tv_sec) + static_cast<double>(Now.tv_usec - Before.tv_usec)/1000000.0;
			Before = Now;
			return ret;
		}

		
		unsigned int GetElapsedUInt()			
		{	
			struct timeval Now;
			if(gettimeofday(&Now,NULL))
				throw "Get time of day Failed";
			unsigned int ret = (Now.tv_sec - Before.tv_sec) * 1000000 + (Now.tv_usec - Before.tv_usec);
			Before = Now;
			return ret;
		}

		double GetFrameRate()
		{
			FrameRateAccumulator += 1000000/GetElapsedDouble();
			if (++CurrentCount > FrameRateAverageCount)
			{
				LastFrameRate = FrameRateAccumulator/CurrentCount;
				CurrentCount = 0;
				FrameRateAccumulator = 0;
				HaveAFrameRate = true;
			}
			else if (!HaveAFrameRate)
			{
				LastFrameRate = FrameRateAccumulator/CurrentCount;
			}
			return LastFrameRate;
		}
		
		struct timeval Before;	

	private:
		bool HaveAFrameRate;
		int FrameRateAverageCount;
		int CurrentCount;
		double FrameRateAccumulator;
		double LastFrameRate;
		
};

double log2(double in);
double logn(double in, double n);

double RoundPow2(double in);///< Round value up to nearest power of 2
double RoundPowN(double in, double n); ///< Round value up to nearest power of n

#endif
