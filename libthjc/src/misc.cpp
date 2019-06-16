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
 *	Misc useful stuff    								*
 *														*
 *	Toby Collett & Geoffrey Biggs(list prints)			*
 ********************************************************/

#include <libthjc/misc.h>

#include <string.h>
#include <stdio.h>

#include <math.h>

extern "C"
{
	void Nothing() {};

}

void PrintList (char *listTitle, list<int> data)
{
	printf ("%s = [", listTitle);
	for (list<int>::iterator itr = data.begin (); itr != data.end (); itr++)
	{
		printf ("%d, ", *itr);
		itr++;
		if (itr != data.end ())
			printf ("%d;\n", *itr);
	}
	printf ("]\n");
}

void PrintList (char *listTitle, list<Point2D> data)
{
	printf ("%s = [", listTitle);
	for (list<Point2D>::iterator itr = data.begin (); itr != data.end (); itr++)
	{
		printf ("%f, %f;\n", itr->x, itr->y);
	}
	printf ("]\n");
}

void PrintList (char *listTitle, list<Vector3D> data)
{
	printf ("%s = [", listTitle);
	for (list<Vector3D>::iterator itr = data.begin (); itr != data.end (); itr++)
	{
		printf ("%f, %f, %f;\n", itr->x, itr->y, itr->z);
	}
	printf ("]\n");
}

void PrintList (char *listTitle, list<Line> data)
{
	printf ("%s = [", listTitle);
	for (list<Line>::iterator itr = data.begin (); itr != data.end (); itr++)
	{
		printf ("[%f, %f] [%f, %f];\n", itr->P1.x, itr->P1.y, itr->P2.x, itr->P2.y);
	}
	printf ("]\n");
}

double log2(double in)
{
	return log(in)/log(2.0);	
}

double logn(double in, double n)
{
	return log(in)/log(n);
}

double RoundPow2(double in) ///< Round value up to nearest power of 2
{
	return pow(2.0,ceil(log2(in)));
}

double RoundPowN(double in, double n) ///< Round value up to nearest power of n
{
	return pow(n,ceil(logn(in,n)));
}
