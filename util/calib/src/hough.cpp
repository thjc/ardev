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
#include "hough.h"
#include <math.h>
#include <Magick++.h>
#include <list>
#include <vector>
#include <map>

#include <opencv/cv.h>
using namespace std;


ImageSegment::ImageSegment(vector<Point2D> aPoints)
{
	if (aPoints.size() == 0)
		return;
	// initialise the bounding box
	bb.SetTop(static_cast<unsigned int> (floor(aPoints.front().y)));
	bb.SetBottom(static_cast<unsigned int> (ceil(aPoints.front().y)));
	bb.SetLeft(static_cast<unsigned int> (floor(aPoints.front().x)));
	bb.SetRight(static_cast<unsigned int> (ceil(aPoints.front().x)));
	for (vector<Point2D>::const_iterator itr = aPoints.begin(); itr != aPoints.end(); ++itr)
		AddPoint(*itr);
}

void ImageSegment::AddPoint(const Point2D &NewPoint)
{
	CenterOfMass=(CenterOfMass*Points.size() + NewPoint)/(Points.size()+1);
	Points.push_back(NewPoint);
	if (NewPoint.x < bb.GetLeft())
		bb.SetLeft(static_cast<unsigned int> (floor(NewPoint.x)));
	if (NewPoint.x > bb.GetRight())
		bb.SetRight(static_cast<unsigned int> (ceil(NewPoint.x)));
	if (NewPoint.y < bb.GetTop())
		bb.SetTop(static_cast<unsigned int> (ceil(NewPoint.y)));
	if (NewPoint.y > bb.GetBottom())
		bb.SetBottom(static_cast<unsigned int> (floor(NewPoint.y)));
}


int doHough(ARImage & Image, ARImage & Hough)
{
	ARImage TempHough;
	TempHough.x_size = Hough.x_size;
	TempHough.y_size = Hough.y_size;
	TempHough.ByteDepth = 4;
	TempHough.Allocate();
	
	TempHough.Erase();
	
	// Calculate furthest point from origin
	double MaxDist = sqrt(static_cast<double> (Image.x_size*Image.x_size + Image.y_size*Image.y_size));

	double p_increment = 2*MaxDist/(Hough.y_size);
	double theta_inc = M_PI/Hough.x_size;
	
	// propagate accumulator matrix
	for (unsigned int x=0;x < Image.x_size; ++x)
	{
		if (x%64 == 0)
			printf("Collumn %d",x);
		else if (x%64 == 63)
			printf(".\n");
		else
			printf(".");
		fflush(stdout);
		for (unsigned int y=0;y < Image.y_size; ++y)
		{
			if (Image.GetPixel(x,y) == 0)
				continue;
			double dTheta;
			int P;
			for (unsigned int iTheta = 0; iTheta < Hough.x_size; ++iTheta)
			{
				// get double theta value
				dTheta = theta_inc * iTheta - M_PI/2;
				P = static_cast<int> (fabs((MaxDist+x*cos(dTheta) + y*sin(dTheta))/p_increment));

				//printf("accumulating point %d %d in %d depth\n",iTheta,P,Hough.ByteDepth);
				(*reinterpret_cast<unsigned int *> (&TempHough.GetPixel(iTheta*4,P)))++;
				//if (TempHough.GetPixel(iTheta*4,P) < 255)
				//	TempHough.GetPixel(iTheta,P)++;
				
			}
		}
	}
	printf("\n");
	// Now Normalise
	int Max=0;
	for (unsigned int x=0;x < Image.x_size; ++x)
	{
		for (unsigned int y=0;y < Image.y_size; ++y)
		{
			int Val = (*reinterpret_cast<unsigned int *> (&TempHough.GetPixel(x*4,y)));
			if (Val > Max)
				Max = Val;
		}
	}
	int Div = Max/255 + 1;
	for (unsigned int x=0;x < Image.x_size; ++x)
	{
		for (unsigned int y=0;y < Image.y_size; ++y)
		{
			Hough.GetPixel(x,y) = (*reinterpret_cast<unsigned int *> (&TempHough.GetPixel(x*4,y)))/Div;
		}
	}


	
	return 0;
	
}


int doGray(ARImage & Image, ARImage & Output)
{
	if (Image.ByteDepth != 3)
	{
		printf("Not 24 bit image\n");
		Output = Image;
		return 0;
	}
	for (unsigned int x=0;x < Image.x_size * Image.ByteDepth; x+=Image.ByteDepth)
	{
		for (unsigned int y=0;y < Image.y_size; ++y)
		{
			int temp = Image.GetPixel(x,y);
			temp += Image.GetPixel(x+1,y);
			temp += Image.GetPixel(x+2,y);
			
			Output.GetPixel(x/Image.ByteDepth,y) = temp/Image.ByteDepth;
		}
	}
	return 0;
}


int doHue(Point2D TopLeft, Point2D BottomRight, int ClearColour, ARImage & Image, ARImage & Output)
{
	// clear background
	for (unsigned int x =0; x < Output.x_size; ++x)
	{
		for (unsigned int y = 0; y < Output.y_size; ++y)
		{
			Output.GetPixel(x,y) = ClearColour;
		}
	}

	if (Image.ByteDepth != 3)
	{
		printf("Not 24 bit image\n");
		Output = Image;
		return 0;
	}

	for (unsigned int x=static_cast<unsigned int> (TopLeft.x* Image.ByteDepth);x < BottomRight.x * Image.ByteDepth; x+=Image.ByteDepth)
	{
		for (unsigned int y=static_cast<unsigned int> (BottomRight.y);y < TopLeft.y; ++y)
		{
			double R = Image.GetPixel(x,y);
			double G = Image.GetPixel(x+1,y);
			double B = Image.GetPixel(x+2,y);
			
			double RmB = R-B;
			double RmG = R-G;
			double GmB = G-B;
			
			double I = (R+B+G)/3;
			double S = 1-std::min(R,std::min(G,B))/I;
			
			// if we have no saturation or no intensity then use the clear colour
			if (S> 0.01 && I > 5)
			{
				double Theta = acos(0.5 * (RmG+RmB)/sqrt(RmG*RmG+RmB*GmB));
				Theta = B <= G ? Theta : (2*M_PI)-Theta;
/*				printf("R=%f; G=%f; B=%f;\nRmB=%f; RmG=%f; GmB=%f;\n",R,G,B,RmB,RmG,GmB);
				printf("R=%f; G=%f; B=%f;\n%f %f\n",R,G,B,((R-G)*(R-G)+(R-B)*(G-B)), sqrt(((R-G)*(R-G)+(R-B)*(G-B))));
				printf("%f %f %f\n", (0.5 * ((R-G)+(R-B))/sqrt(((R-G)*(R-G)+(R-B)*(G-B)))), acos(0.5 * ((R-G)+(R-B))/sqrt(((R-G)*(R-G)+(R-B)*(G-B)))), Theta);
				printf("Hue for %d %d, from %f %f %f, is %f or %d\n",x/Image.ByteDepth,y,R,G,B,Theta, static_cast<int> (255*Theta/(2*M_PI)));
*/
				Output.GetPixel(x/Image.ByteDepth,y) = static_cast<int> (255*Theta/(1*M_PI));
			}
		}
	}
	return 0;
}

int ExtractHue(int Colour, int Range, ARImage & Image, ARImage & Output)
{
	for (unsigned int x=0;x < Image.x_size * Image.ByteDepth; ++x)
	{
		for (unsigned int y=0;y < Image.y_size; ++y)
		{
			int temp = Image.GetPixel(x,y);
			if (temp < Colour - Range || temp > Colour + Range)
				temp = 0;
			else
				temp = 255;			
			Output.GetPixel(x,y) = temp;
		}
	}
	return 0;
}


int doThresh(ARImage & Image, ARImage & Output, int thresh)
{
	for (unsigned int x=0;x < Image.x_size * Image.ByteDepth; ++x)
	{
		for (unsigned int y=0;y < Image.y_size; ++y)
		{
			int temp = Image.GetPixel(x,y);
			if (temp < thresh)
				temp = 0;
			else
				temp = 255;
			Output.GetPixel(x,y) = temp;
		}
	}
	return 0;
}




int doHitMiss(ARImage & Input, ARImage & Output, int Size, int * Kernel)
{
	Output=Input;

	int Edge = (Size + 1)/2;
	
/*	printf("%dx%d Kernel Center at %d,%d, Input Image is %dx%d\n",Size,Size,Edge,Edge, Input.x_size,Input.y_size);
	for (int v = 0; v < Size; ++v)
	{
		for (int u = 0; u < Size; ++u)
		{
			if (Kernel[v*Size + u] == -1)
				printf("0 ");
			else if (Kernel[v*Size + u] == 1)
				printf("1 ");
			else
				printf("  ");
		}
		printf("\n");
	}*/
	
	
	int MatchCount = 0;
	for (unsigned int y = Size; y < Input.y_size - Edge; ++y)
	{		
		/*if (y%48 == 0)
			printf("Row %d",y);
		else if (y%48 == 43)
			printf(".\n");
		else
			printf(".");
		fflush(stdout);*/
		for (unsigned int x = Size; x < Input.x_size - Edge; ++x)
		{	
			bool Match = true;
			for (int v = 0; v < Size; ++v)
			{
				for (int u = 0; u < Size; ++u)
				{
					int tempK = Kernel[v*Size + u];
					int tempI = Input.GetPixel(x+u-Edge+1,y+v-Edge+1);
					if ((tempK < 0 && tempI !=0) || (tempK > 0 && tempI == 0))
					{
						Match = false;
						break;
					}
				}
				if (Match == false)
					break;
			}
			if (Match)
			{
				MatchCount++;
				Output.GetPixel(x,y) = 0;
			}
		}
	}
	//printf("\n");
	//printf("%d Pixels Eliminated\n",MatchCount);
	return 0;
}


int doThinning(ARImage & Input, ARImage & Output)
{
	Output = Input;
	ARImage Temp;
	int Count = 0;
	int Kernel[8][9] = {{-1,-1,-1,0,1,0,1,1,1},
					{0,-1,-1,1,1,-1,0,1,0},
					{1,0,-1,1,1,-1,1,0,-1},
					{0,1,0,1,1,-1,0,-1,-1},
					{1,1,1,0,1,0,-1,-1,-1},
					{0,1,0,-1,1,1,-1,-1,0},
					{-1,0,1,-1,1,1,-1,0,1},
					{-1,-1,0,-1,1,1,0,1,0}};
	
	do
	{
		printf("Thining Iteration %d\n",++Count);
		for (int i =0; i < 8; ++i)
		{
			Temp = Output;
			doHitMiss(Temp, Output,3,Kernel[i]);
		}
		//if (Count > 3) break;
	} while (memcmp(Temp.data,Output.data,Temp.GetDataSize()));
	return 0;
}

int doPruning(ARImage & Input, ARImage & Output, int Count)
{
	Output = Input;
	ARImage Temp;
	int i = 0;
	int Kernel[8][9] = {{-1,-1,-1,-1,1,-1,-1,0,0},
					{-1,-1,-1,-1,1,-1,0,0,-1},
					{-1,-1,-1,0,1,-1,0,-1,-1},
					{0,-1,-1,0,1,-1,-1,-1,-1},
					{0,0,-1,-1,1,-1,-1,-1,-1},
					{-1,0,0,-1,1,-1,-1,-1,-1},
					{-1,-1,0,-1,1,0,-1,-1,-1},
					{-1,-1,-1,-1,1,0,-1,-1,0}};
	
	do
	{
		printf("Pruning Iteration %d\n",++i);
		for (int j =0; j < 8; ++j)
		{
			Temp = Output;
			doHitMiss(Temp, Output,3,Kernel[j]);
		}
	} while (i<Count && memcmp(Temp.data,Output.data,Temp.GetDataSize()));
	return 0;
}

struct intlt
{
	bool operator () (int lhs,int rhs)
	{
		return lhs < rhs;
	}
};

list<vector<Point2D> > doLabelSegmentsGray(int range, ARImage & Input, ARImage & Output)
{
	typedef vector<Point2D> matvec;
	map<int,int,intlt> Equivalents;
	map<int,matvec,intlt> MatLabels;

	if (Input.y_size < 4 || Input.x_size < 4)
	{
		printf("Error Image too small (%dx%d)\n",Input.x_size ,Input.y_size );
		list<vector<Point2D> > Ret;
		return Ret;
	}
	
	Output.Erase();

	unsigned int * Labels = new unsigned int[Input.x_size*Input.y_size];
	bzero(Labels,Input.x_size*Input.y_size*sizeof(unsigned int));
	
	int CurrentLabel = 1;
	int TempLabelLeft;
	int TempLabelUp;
	for (unsigned int y = 1; y < Input.y_size - 1; ++y)
	{		
		for (unsigned int x = 1; x < Input.x_size - 1; ++x)
		{	
			int ThisPixel = Input.GetPixel(x,y);
			int LeftPixel = Input.GetPixel(x-1,y);
			int UpPixel = Input.GetPixel(x,y-1);
			
			// check if grayscale value is equivalent to neighbour
			if (abs(ThisPixel - LeftPixel) < range)
			{
				// find out if we already have an equivalent for this value
				TempLabelLeft = Labels[x-1+y*Input.x_size];
				while (Equivalents[TempLabelLeft] != 0)
					TempLabelLeft = Equivalents[TempLabelLeft];
			}
			else
				// no vlaid neighbour on left
				TempLabelLeft = 0;

			// check if grayscale value is equivalent to neighbour
			if (abs(ThisPixel - UpPixel) < range)
			{
				// find out if we already have an equivalent for this value
				TempLabelUp = Labels[x+(y-1)*Input.x_size];
				while (Equivalents[TempLabelUp] != 0)
					TempLabelUp = Equivalents[TempLabelUp];
			}
			else
				// no valid neighbour above
				TempLabelUp = 0;
			
			// see if we need to create a new label, or a new equivalent
			if (TempLabelLeft == 0 && TempLabelUp == 0)
				Labels[x+y*Input.x_size] = CurrentLabel++;
			else if (TempLabelLeft == 0 || TempLabelLeft == TempLabelUp)
				Labels[x+y*Input.x_size] = TempLabelUp;
			else if (TempLabelUp == 0)
				Labels[x+y*Input.x_size] = TempLabelLeft;
			else
			{
				//printf("Adding Equivalent %d ==> %d\n",TempLabelLeft, TempLabelUp);
				Equivalents[TempLabelLeft]=TempLabelUp;
				Labels[x+y*Input.x_size] = TempLabelUp;				
			}
		}
	}
	map<int,int,intlt> Equivalents2;
	printf("Normalise or compress labels to minimum number (Original count=%d\n",CurrentLabel);
	for (int i =1; i <CurrentLabel; i++)
	{
		int TempLabel = i;
		while(Equivalents[TempLabel])
			TempLabel = Equivalents[TempLabel];
		Equivalents2[i] = TempLabel;

		//printf("Compressed Equiv %d ==> %d\n",i,Equivalents2[i]);

	}
	
	printf("Build Matrix of Final Points\n");
	for (unsigned int y = 1; y < Input.y_size - 1; ++y)
	{
//		printf("Row %d\n",y);		
		for (unsigned int x = 1; x < Input.x_size - 1; ++x)
		{	
			if (Labels[x+y*Input.x_size] == 0)
				continue;
			int Label = Equivalents2[Labels[x+y*Input.x_size] ];
			MatLabels[Label].push_back(Point2D(x,y));
		}
	}		

	// generate output image of large segments
	int PixelValue = 0;
	for (map<int,vector<Point2D>,intlt>::iterator itr = MatLabels.begin(); itr != MatLabels.end();++itr)
	{
		if ((itr->second).size() > 10)
		{
			PixelValue ++;
			for (vector<Point2D>::iterator itr2 = itr->second.begin(); itr2 != itr->second.end();++itr2)
			{
				Output.GetPixel(static_cast<int> (itr2->x),static_cast<int> (itr2->y)) = PixelValue %256;
			}
		}
	}
	printf("Added %d Segments to the Output Image\n", PixelValue);
	
	printf("Convert Map of Matricies to a list\n");
	list<vector<Point2D> > Ret;
	for (map<int,vector<Point2D>,intlt>::iterator itr = MatLabels.begin(); itr != MatLabels.end();++itr)
	{
		Ret.push_back((itr->second));
	}
	delete [] Labels;
	printf("Segmented to %d Parts\n",static_cast<int> (Ret.size()));
	return Ret;
}


list<vector<Point2D> > doLabelSegments(ARImage & Input, SegmentMode Mode, int range)
{
	BoundingBox FullImage(0,Input.y_size, 0, Input.x_size);
	return doLabelSegments(Input, FullImage, Mode, range);
}

list<vector<Point2D> > doLabelSegments(ARImage & Input,  BoundingBox ROI, SegmentMode Mode, int range)
{
	printf("Segmenting in range: %d %d %d %d\n",ROI.GetTop(), ROI.GetBottom(), ROI.GetLeft(), ROI.GetRight());
	
	typedef vector<Point2D> matvec;
	map<int,int,intlt> Equivalents;
	map<int,matvec,intlt> MatLabels;

	if (Input.y_size < 4 || Input.x_size < 4 || ROI.GetRight() > Input.x_size || ROI.GetLeft() < 0 ||
			ROI.GetTop() < 0 || ROI.GetBottom() > Input.y_size)
	{
		printf("Error Image too small (%dx%d)\n",Input.x_size ,Input.y_size );
		list<vector<Point2D> > Ret;
		return Ret;
	}

	// make sure our ROI is at least 1 less than border of the image
	if (ROI.GetLeft() < 1)
		ROI.SetLeft(1);
	if (ROI.GetRight() > Input.x_size - 1)
		ROI.SetRight(Input.x_size -1);
	if (ROI.GetTop() < 1)
		ROI.SetTop(1);
	if (ROI.GetBottom() > Input.y_size -1 )
		ROI.SetBottom(Input.y_size -1);

	// our label record space needs to have a 1 pixel border to stop memory errors later
	unsigned int * Labels = new unsigned int[(ROI.GetWidth()+2)*(ROI.GetHeight()+2)];
	bzero(Labels,(ROI.GetWidth()+2)*(ROI.GetHeight()+2)*sizeof(unsigned int));
	
	int CurrentLabel = 1;
	int TempLabelLeft;
	int TempLabelUp;
	for (unsigned int y = ROI.GetTop(); y < ROI.GetBottom(); ++y)
	{		
		for (unsigned int x = ROI.GetLeft(); x < ROI.GetRight(); ++x)
		{	
			int ThisPixel = Input.GetPixel(x,y);
			if (Mode == SEG_LESS && ThisPixel > range)
			{
				Input.GetPixel(x,y) = 0xFF;
				continue;
			}
			if (Mode == SEG_MORE && ThisPixel < range)
			{
				Input.GetPixel(x,y) = 0x00;
				continue;
			}
			if (Mode == SEG_MORE)
				Input.GetPixel(x,y) = 0xFF;
			if (Mode == SEG_LESS)
				Input.GetPixel(x,y) = 0x00;

			int LeftPixel = Input.GetPixel(x-1,y);
			int UpPixel = Input.GetPixel(x,y-1);
			
			// check if grayscale value is equivalent to neighbour
			if (abs(ThisPixel - LeftPixel) <= range)
			{
				// find out if we already have an equivalent for this value
				TempLabelLeft = Labels[(x-ROI.GetLeft()-1+1)+(y-ROI.GetTop()+1)*(ROI.GetWidth()+2)];
				while (Equivalents[TempLabelLeft] != 0)
					TempLabelLeft = Equivalents[TempLabelLeft];
			}
			else
				// no vlaid neighbour on left
				TempLabelLeft = 0;

			// check if grayscale value is equivalent to neighbour
			if (abs(ThisPixel - UpPixel) <= range)
			{
				// find out if we already have an equivalent for this value
				TempLabelUp = Labels[(x-ROI.GetLeft()+1)+(y-ROI.GetTop()-1+1)*(ROI.GetWidth()+2)];
				while (Equivalents[TempLabelUp] != 0)
					TempLabelUp = Equivalents[TempLabelUp];
			}
			else
				// no valid neighbour above
				TempLabelUp = 0;
			
			// see if we need to create a new label, or a new equivalent
			if (TempLabelLeft == 0 && TempLabelUp == 0)
				Labels[(x-ROI.GetLeft()+1)+(y-ROI.GetTop()+1)*(ROI.GetWidth()+2)] = CurrentLabel++;
			else if (TempLabelLeft == 0 || TempLabelLeft == TempLabelUp)
				Labels[(x-ROI.GetLeft()+1)+(y-ROI.GetTop()+1)*(ROI.GetWidth()+2)] = TempLabelUp;
			else if (TempLabelUp == 0)
				Labels[(x-ROI.GetLeft()+1)+(y-ROI.GetTop()+1)*(ROI.GetWidth()+2)] = TempLabelLeft;
			else
			{
				//printf("Adding Equivalent %d ==> %d\n",TempLabelLeft, TempLabelUp);
				Equivalents[TempLabelLeft]=TempLabelUp;
				Labels[(x-ROI.GetLeft()+1)+(y-ROI.GetTop()+1)*(ROI.GetWidth()+2)] = TempLabelUp;				
			}
		}
	}
	map<int,int,intlt> Equivalents2;
	printf("Normalise or compress labels to minimum number (Original count=%d\n",CurrentLabel);
	for (int i =1; i <CurrentLabel; i++)
	{
		int TempLabel = i;
		while(Equivalents[TempLabel])
			TempLabel = Equivalents[TempLabel];
		Equivalents2[i] = TempLabel;

		//printf("Compressed Equiv %d ==> %d\n",i,Equivalents2[i]);

	}
	
	printf("Build Matrix of Final Points\n");
	for (unsigned int y = ROI.GetTop(); y < ROI.GetBottom(); ++y)
	{		
		for (unsigned int x = ROI.GetLeft(); x < ROI.GetRight(); ++x)
		{	
			if (Labels[(x-ROI.GetLeft()+1)+(y-ROI.GetTop()+1)*(ROI.GetWidth()+2)] == 0)
				continue;
			int Label = Equivalents2[Labels[(x-ROI.GetLeft()+1)+(y-ROI.GetTop()+1)*(ROI.GetWidth()+2)] ];
			MatLabels[Label].push_back(Point2D(x,y));
		}
	}		


	printf("Convert Map of Matricies to a list\n");
	list<vector<Point2D> > Ret;
	for (map<int,vector<Point2D>,intlt>::iterator itr = MatLabels.begin(); itr != MatLabels.end();++itr)
	{
		Ret.push_back((itr->second));
	}
	delete [] Labels;
	printf("Segmented to %d Parts\n",static_cast<int> (Ret.size()));
	//DisplaySegments(Output, Ret);
	return Ret;
}

vector<Point2D> getBestRect(ARImage & ThresholdImage, list<vector<Point2D> > &Segments)
{
	vector<Point2D> Ret, Biggest;
	while(Segments.size() > 0)
	{
		{
			list<vector<Point2D> >::iterator itr_ret = Segments.begin();
			for (list<vector<Point2D> >::iterator itr = Segments.begin(); itr != Segments.end();++itr)
			{
				if (itr->size() > itr_ret->size())
					itr_ret = itr;
			}
			Biggest = *itr_ret;
			
			Segments.erase(itr_ret);
		}
		if (Biggest.size() < 1000)
			return Ret;
		
		CvMemStorage* storage = cvCreateMemStorage();
		CvMemStorage* storage2 = cvCreateMemStorage();
		CvSeq* ptseq = cvCreateSeq( CV_SEQ_KIND_GENERIC|CV_32FC2, sizeof(CvContour),
                                     sizeof(CvPoint), storage );
		CvSeq* ptseq2 = cvCreateSeq( CV_SEQ_POLYLINE|CV_32FC2, sizeof(CvContour),
                                     sizeof(CvPoint), storage2 );
		CvSeq* hull;

		CvPoint pt0;
		for (vector<Point2D>::iterator itr = Biggest.begin(); itr != Biggest.end();++itr)
		{
			pt0.x= static_cast<int>(itr->x);
			pt0.y= static_cast<int>(itr->y);
			cvSeqPush( ptseq, &pt0 );
		}

		hull = cvConvexHull2( ptseq, 0, CV_CLOCKWISE, 0 );
		int hullcount = hull->total;
        
		Point2D Center;
		Center.x = 0;
		Center.y = 0;
		for (int i = 0; i < hullcount; ++i)
		{
			Ret.push_back(Point2D((*CV_GET_SEQ_ELEM( CvPoint*, hull, i ))->x,(*CV_GET_SEQ_ELEM( CvPoint*, hull, i ))->y));
			//printf("%d %d\n",(*CV_GET_SEQ_ELEM( CvPoint*, hull, i ))->x,(*CV_GET_SEQ_ELEM( CvPoint*, hull, i ))->y);
			cvSeqPush(ptseq2,*CV_GET_SEQ_ELEM( CvPoint*, hull, i ));
			Center.x += (*CV_GET_SEQ_ELEM( CvPoint*, hull, i ))->x;
			Center.y += (*CV_GET_SEQ_ELEM( CvPoint*, hull, i ))->y;
		}
		Center.x /= hullcount;
		Center.y /= hullcount;
        
		printf("Now get the contour area\n");
		printf("%f %f\n",Center.x,Center.y);
		double area = 0;
		for (int i = 0; i < hullcount-1; ++i)
		{
			Point2D p1((*CV_GET_SEQ_ELEM( CvPoint*, hull, i ))->x,(*CV_GET_SEQ_ELEM( CvPoint*, hull, i ))->y);
			Point2D p2((*CV_GET_SEQ_ELEM( CvPoint*, hull, i+1 ))->x,(*CV_GET_SEQ_ELEM( CvPoint*, hull, i+1 ))->y);
			
			Line l(p1,p2);
			double h = l.DistToPoint(Center);
			double b = CalcDistance(p1,p2);
			area += h*b/2;
		}
		Point2D p1((*CV_GET_SEQ_ELEM( CvPoint*, hull, hullcount-1 ))->x,(*CV_GET_SEQ_ELEM( CvPoint*, hull, hullcount-1 ))->y);
		Point2D p2((*CV_GET_SEQ_ELEM( CvPoint*, hull, 0 ))->x,(*CV_GET_SEQ_ELEM( CvPoint*, hull, 0 ))->y);
			
		Line l(p1,p2);
		double h = l.DistToPoint(Center);
		double b = CalcDistance(p1,p2);
		area += h*b/2;
        
        
		ImageSegment BiggestSegment(Biggest);
		printf("Segcount before= %ld\n",BiggestSegment.GetPoints().size());
		ImageSegment FilledSeg = FillHoles(BiggestSegment,ThresholdImage);
		printf("Segcount after= %ld\n",FilledSeg.GetPoints().size());
	
	
		printf("hull area = %f, point coutn = %ld, ratio = %f\n",area, FilledSeg.GetPoints().size(), FilledSeg.GetPoints().size()/area);
	
		cvClearMemStorage( storage );
		cvClearMemStorage( storage2 );
		
		
		if (FilledSeg.GetPoints().size()/area > 0.6)
		{
			ARImage b = ThresholdImage;
			b.Erase();
			list<ImageSegment> c;
			c.push_back (BiggestSegment);
			DisplaySegments(b,c);
			
			int holes = CountHoles(b,BiggestSegment,100);
			printf("Segment had %d holes\n",holes);
			if (holes < 9)
				continue;
			
			return Biggest;
		}
	}
		
	return Ret;
}


vector<Point2D> getBestRect(ARImage & ThresholdImage, list<vector<Point2D> > &Segments, ARImage & Output)
{
	vector<Point2D> Ret = getBestRect(ThresholdImage,Segments);
	
	Output.Erase();
	for (vector<Point2D>::iterator itr = Ret.begin(); itr != Ret.end();++itr)
	{
		Output.GetPixel(static_cast<int> (itr->x),static_cast<int> (itr->y)) = 255;
	}
	return Ret;
}

vector<Point2D> doRemoveBiggestSegment(list<vector<Point2D> > &Segments)
{
	list<vector<Point2D> >::iterator itr_ret = Segments.begin();
	for (list<vector<Point2D> >::iterator itr = Segments.begin(); itr != Segments.end();++itr)
	{
		if (itr->size() > itr_ret->size())
			itr_ret = itr;
	}
	vector<Point2D> Ret = *itr_ret;
	Segments.erase(itr_ret);
		
	return Ret;
}

vector<Point2D> doRemoveBiggestSegment(list<vector<Point2D> > &Segments, ARImage & Output)
{
	vector<Point2D> Ret = doRemoveBiggestSegment(Segments);
	
	Output.Erase();
	for (vector<Point2D>::iterator itr = Ret.begin(); itr != Ret.end();++itr)
	{
		Output.GetPixel(static_cast<int> (itr->x),static_cast<int> (itr->y)) = 255;
	}
	return Ret;
}

void doGetCubeRegion(vector<Point2D> Segment, Point2D & TopLeft, Point2D & BottomRight, ARImage & Input, ARImage & Output)
{
	if (Segment.empty())
	{
		Output = Input;
		TopLeft = Point2D(0,0);
		BottomRight = Point2D(0,0);
		printf("No segments found\n");
		return;
	}
		
		
	TopLeft = Segment.front();
	BottomRight = Segment.front();
	for (vector<Point2D>::iterator itr = Segment.begin(); itr != Segment.end();++itr)
	{
		if (itr->x < TopLeft.x)
			TopLeft.x = itr->x;
		if (itr->y > TopLeft.y)
			TopLeft.y = itr->y;
		if (itr->x > BottomRight.x)
			BottomRight.x = itr->x;
		if (itr->y < BottomRight.y)
			BottomRight.y = itr->y;
	}

	// draw a box around the cube
	Output=Input;
	for (int x = static_cast<int> (TopLeft.x); x < BottomRight.x; x++)
	{
		Output.GetPixel(x,static_cast<int> (TopLeft.y)) = 255;
		Output.GetPixel(x,static_cast<int> (BottomRight.y)) = 255;
	}
	for (int y = static_cast<int> (BottomRight.y); y < TopLeft.y; y++)
	{
		Output.GetPixel(static_cast<int> (TopLeft.x),y) = 255;
		Output.GetPixel(static_cast<int> (BottomRight.x),y) = 255;
	}

}

Point2D doGetCenter(const vector<Point2D> & Segment)
{
	if (Segment.empty())
	{
		return Point2D(0,0);
	}

	Point2D Ret(0,0);
	for (vector<Point2D>::const_iterator itr = Segment.begin(); itr != Segment.end();++itr)
		Ret += *itr;
	return Ret / Segment.size();
}
	
void getLRU(int & Left, int & Right, int & Up, const vector<Point2D> & Segment1, const vector<Point2D> & Segment2, const vector<Point2D> & Segment3)
{
	Point2D Center1 = doGetCenter(Segment1);
	Point2D Center2 = doGetCenter(Segment2);
	Point2D Center3 = doGetCenter(Segment3);

	// find Leftmost Blob;
	if (Center1.x < Center2.x && Center1.x < Center3.x)
	{
		Left=1;
		if (Center2.x > Center3.x)
		{
			Right = 2;
			Up = 3;
		}
		else
		{
			Right = 3;
			Up = 2;
		}	
	}
	else if (Center2.x < Center3.x)
	{
		Left=2;
		if (Center1.x > Center3.x)
		{
			Right = 1;
			Up = 3;
		}
		else
		{
			Right = 3;
			Up = 1;
		}	
		
	}
	else
	{
		Left=3;	
		if (Center2.x > Center1.x)
		{
			Right = 2;
			Up = 1;
		}
		else
		{
			Right = 1;
			Up = 2;
		}	
	}	
}

vector<Point2D> getNineDotCenters(list<vector<Point2D> > & Segments)
{
	vector<Point2D> Ret;
	if (Segments.size() < 9)
	{
		printf("Not Enough Dots\n");
		return Ret;
	}
	
	// first drop the backgroup and cube
	doRemoveBiggestSegment(Segments);
	doRemoveBiggestSegment(Segments);
	for (int i = 0; i < 9; ++i)
	{
		vector<Point2D> Segment = doRemoveBiggestSegment(Segments);
		Ret.push_back(doGetCenter(Segment));
	}
	return Ret;	
}

Point2D eraseLeftDot(vector<Point2D> & Dots)
{
	vector<Point2D>::iterator ret_itr= Dots.begin();
	for (vector<Point2D>::iterator itr = Dots.begin(); itr != Dots.end();++itr)
	{
		if (itr->x < ret_itr->x)
			ret_itr = itr;
	}
	Point2D Ret = * ret_itr;
	Dots.erase(ret_itr);
	return Ret;
}

Point2D eraseRightDot(vector<Point2D> & Dots)
{
	vector<Point2D>::iterator ret_itr= Dots.begin();
	for (vector<Point2D>::iterator itr = Dots.begin(); itr != Dots.end();++itr)
	{
		if (itr->x > ret_itr->x)
			ret_itr = itr;
	}
	Point2D Ret = * ret_itr;
	Dots.erase(ret_itr);
	return Ret;
}

Point2D eraseTopDot(vector<Point2D> & Dots)
{
	vector<Point2D>::iterator ret_itr= Dots.begin();
	for (vector<Point2D>::iterator itr = Dots.begin(); itr != Dots.end();++itr)
	{
		if (itr->y < ret_itr->y)
			ret_itr = itr;
	}
	Point2D Ret = * ret_itr;
	Dots.erase(ret_itr);
	return Ret;
}

Point2D eraseBottomDot(vector<Point2D> & Dots)
{
	vector<Point2D>::iterator ret_itr= Dots.begin();
	for (vector<Point2D>::iterator itr = Dots.begin(); itr != Dots.end();++itr)
	{
		if (itr->y > ret_itr->y)
			ret_itr = itr;
	}
	Point2D Ret = * ret_itr;
	Dots.erase(ret_itr);
	return Ret;
}



Point2D ClosestDot(vector<Point2D> & Dots, Point2D Target)
{
	double dist = CalcDistance(Target,Dots.front());
	vector<Point2D>::iterator ret_itr= Dots.begin();
	
	for (vector<Point2D>::iterator itr = Dots.begin(); itr != Dots.end();++itr)
	{
		double TempDist;
		if ((TempDist = CalcDistance(Target,*itr)) < dist)
		{
			dist = TempDist;
			ret_itr = itr;
		}
	}
	Point2D Ret = *ret_itr;
	Dots.erase(ret_itr);
	return Ret;
}

Point2D rightmostOfClosestTwo(vector<Point2D> & Dots, Point2D Target)
{
	vector<Point2D> Temp = Dots;
	Point2D One = ClosestDot(Temp,Target);
	Point2D Two = ClosestDot(Temp,Target);
	
	Point2D Ret = One;
	if (One.x < Two.x)
		Ret = Two;
	
	for (vector<Point2D>::iterator itr = Dots.begin(); itr != Dots.end();++itr)
	{
		if (*itr == Ret)
		{
			Dots.erase(itr);
			return Ret;	
		}
	}
	return Ret;
}

Point2D bottommostOfClosestTwo(vector<Point2D> & Dots, Point2D Target)
{
	vector<Point2D> Temp = Dots;
	Point2D One = ClosestDot(Temp,Target);
	Point2D Two = ClosestDot(Temp,Target);
	
	Point2D Ret = One;
	if (One.y < Two.y)
		Ret = Two;
		
	for (vector<Point2D>::iterator itr = Dots.begin(); itr != Dots.end();++itr)
	{
		if (*itr == Ret)
		{
			Dots.erase(itr);
			return Ret;	
		}
	}
	return Ret;
}

Point2D centerOfNine(vector<Point2D> & Dots)
{
	Point2D Mid = doGetCenter(Dots);
	return ClosestDot(Dots, Mid);
}

vector<Point2D> orderNineDotsLeft(vector<Point2D> & Dots)
{
	vector<Point2D> Ret;
	vector<Point2D> Rights;
	if (Dots.size() < 9)
		return Ret;
	
	Point2D Center = centerOfNine(Dots);
	Point2D Top_ = eraseTopDot(Dots);
	Point2D Next = rightmostOfClosestTwo(Dots, Top_);
	
//	cout << Center << Top << Next << endl;
	Ret.push_back(Center);
	Ret.push_back(Top_);
	Ret.push_back(Next);

	Rights.push_back(eraseRightDot(Dots));
	Rights.push_back(eraseRightDot(Dots));
	Rights.push_back(eraseRightDot(Dots));

	Next = eraseTopDot(Rights);
	Ret.push_back(Next);

	Next = eraseTopDot(Rights);
	Ret.push_back(Next);

	Next = eraseTopDot(Rights);
	Ret.push_back(Next);

	Next = ClosestDot(Dots,Next);
	Ret.push_back(Next);
	
	Next = eraseRightDot(Dots);
	Ret.push_back(Next);
	
	Next = ClosestDot(Dots,Next);
	Ret.push_back(Next);
	

	return Ret;
}	

vector<Point2D> orderNineDotsRight(vector<Point2D> & Dots)
{
	vector<Point2D> Ret;
	if (Dots.size() < 9)
		return Ret;
	
	Point2D Center = centerOfNine(Dots);
	Point2D Top_ = eraseTopDot(Dots);
	Point2D Next = rightmostOfClosestTwo(Dots, Top_);
	
//	cout << Center << Top << Next << endl;
	Ret.push_back(Center);
	Ret.push_back(Top_);
	Ret.push_back(Next);

	Next = eraseRightDot(Dots);
	Ret.push_back(Next);

	Next = ClosestDot(Dots,Next);
	Ret.push_back(Next);
	
	Next = eraseBottomDot(Dots);
	Ret.push_back(Next);
	
	Next = ClosestDot(Dots,Next);
	Ret.push_back(Next);
	
	Next = eraseLeftDot(Dots);
	Ret.push_back(Next);

	Next = ClosestDot(Dots,Next);
	Ret.push_back(Next);

	return Ret;
}	

vector<Point2D> orderNineDotsTop(vector<Point2D> & Dots)
{
	vector<Point2D> Ret;
	if (Dots.size() < 9)
		return Ret;
	
	Point2D Center = centerOfNine(Dots);
	Point2D Top = eraseTopDot(Dots);
	Point2D Right = eraseRightDot(Dots);
	
	Point2D TopMid = (Top+Right)/2;
	TopMid = ClosestDot(Dots, TopMid);
	
//	cout << Center << Top << Next << endl;
	Ret.push_back(Center);
	Ret.push_back(Top);
	Ret.push_back(TopMid);
	Ret.push_back(Right);

	Point2D Next = ClosestDot(Dots,Next);
	Ret.push_back(Next);
	
	Next = eraseBottomDot(Dots);
	Ret.push_back(Next);
	
	Next = ClosestDot(Dots,Next);
	Ret.push_back(Next);
	
	Next = eraseLeftDot(Dots);
	Ret.push_back(Next);

	Next = ClosestDot(Dots,Next);
	Ret.push_back(Next);

	return Ret;
}	

// order dots using radial angle
vector<Point2D> orderNineDots(vector<Point2D> & Dots)
{
	vector<Point2D> Ret;
	if (Dots.size() < 9)
		return Ret;
	
	Point2D Center = centerOfNine(Dots);
	Ret.push_back(Center);
	Point2D Current = eraseTopDot(Dots);
	Ret.push_back(Current);

	while(Dots.size() > 0)
	{
		double MinAngle = 2*M_PI;
		double StartAngle = atan2((Center.y - Current.y), Center.x - Current.x);
		while (StartAngle < 0)
			StartAngle += 2*M_PI;
		vector<Point2D>::iterator best = Dots.begin();
		for (vector<Point2D>::iterator itr = Dots.begin(); itr != Dots.end(); ++itr)
		{
			double AngleDiff = StartAngle - atan2((Center.y - itr->y), Center.x - itr->x);
			while (AngleDiff < 0)
				AngleDiff += 2*M_PI;
			while (AngleDiff > 2*M_PI)
				AngleDiff -= 2*M_PI;
			//printf("(%f %f) (%f %f) (%f %f) %f %f %f\n",Center.x, Center.y, Current.x, Current.y, itr->x, itr->y, StartAngle, AngleDiff,MinAngle);
			if (AngleDiff < MinAngle)
			{
				best = itr;
				MinAngle = AngleDiff;
			}
		}
		Current = *best;
		Dots.erase(best);
		Ret.push_back(Current);
	}
	
	return Ret;
}	


Matrix doCenterBlobs(list<Matrix> & InputBlobs, ARImage & Output)
{
	Output.Erase();
	Matrix Ret(0,3);
	
	Matrix Temp(1,3);
	
	for (list<Matrix>::iterator itr = InputBlobs.begin(); itr != InputBlobs.end(); ++itr)
	{
		Temp.SetValue(0,0,itr->GetCol(0).Avg());
		Temp.SetValue(0,1,itr->GetCol(1).Avg());
		Temp.SetValue(0,2,itr->Rows());

		Ret.InsertRows(Ret.Rows(),Temp);
		Output.GetPixel(static_cast<unsigned int>(Temp.GetValue(0,0)),static_cast<unsigned int>(Temp.GetValue(0,1)))=255;
	}
	
	return Ret;	
}

Matrix doGetTopPoints(Matrix & InputPoints, unsigned int NumPoints, ARImage & Output)
{
	Output.Erase();
	//Matrix Ret(InputPoints);
	Matrix Ret(InputPoints.SortCol(2));
	while (Ret.Rows() > NumPoints)
		Ret = Ret.RemoveRow(Ret.Rows()-1);
	
	for (unsigned int i = 0; i<Ret.Rows(); i++)
		Output.GetPixel(static_cast<unsigned int>(Ret.GetValue(i,0)),static_cast<unsigned int>(Ret.GetValue(i,1)))=255;
		
	return Ret;
}

Matrix doOrderLinesByAngle(Matrix & InputPoints, ARImage & Output)
{
	Output.Erase();
	//Matrix Ret(InputPoints);
	Matrix Ret(InputPoints.SortCol(0));
	
	for (unsigned int i = 0; i<Ret.Rows(); i++)
	{
		printf("Point %d: %f %f\n",i,Ret.GetValue(i,0),Ret.GetValue(i,1));
		Output.GetPixel(static_cast<unsigned int>(Ret.GetValue(i,0)),static_cast<unsigned int>(Ret.GetValue(i,1)))=i*255/Ret.Rows();
	}
		
	return Ret;
}

Matrix doOrderLinesByNormal(Matrix & InputPoints, ARImage & Output)
{
	//Output.Erase();
	//Matrix Ret(InputPoints);
	Matrix Ret(InputPoints.SortCol(1));
	
	for (unsigned int i = 0; i<Ret.Rows(); i++)
	{
		printf("Point %d: %f %f\n",i,Ret.GetValue(i,0),Ret.GetValue(i,1));
		Output.GetPixel(static_cast<unsigned int>(Ret.GetValue(i,0)),static_cast<unsigned int>(Ret.GetValue(i,1)))=i*255/Ret.Rows();
	}
		
	return Ret;
}

list<Line> doMatrixToLines(Matrix & InputPoints, ARImage & Reference)
{
	list<Line> Ret;
	
	// Calculate furthest point from origin
	double MaxDist = sqrt(static_cast<double> (Reference.x_size*Reference.x_size + Reference.y_size*Reference.y_size));

	double p_increment = 2*MaxDist/(Reference.y_size);
	double theta_inc = M_PI/Reference.x_size;

	for (unsigned int i = 0; i < InputPoints.Rows(); ++i)
	{
		double Theta = InputPoints.GetValue(i,0) * theta_inc - M_PI/2;
		double P = InputPoints.GetValue(i,1) * p_increment - MaxDist;
		
		double y1 = (P - cos(Theta))/sin(Theta);
		double y2 = (P - Reference.x_size*cos(Theta))/sin(Theta);
		
		
		Point2D P1(1,y1);
		Point2D P2(Reference.x_size,y2);
		
		Line Temp(P1,P2);
		
		Ret.push_back(Temp);
	}
	return Ret;
}

list<Point2D> doIntersectLines(list<Line> & InputLines)
{
	list<Point2D> Ret;
	
	if (InputLines.size() != 18)
		return Ret;
	
	Line TempLines[18];
	int i = 0;
	for (list<Line>::iterator itr = InputLines.begin(); itr != InputLines.end(); ++itr)
		TempLines[i++] = *itr;
	
	// Left
	Ret.push_back(Intersection(TempLines[2],TempLines[6]).front());
	Ret.push_back(Intersection(TempLines[2],TempLines[7]).front());
	Ret.push_back(Intersection(TempLines[2],TempLines[8]).front());
	Ret.push_back(Intersection(TempLines[1],TempLines[6]).front());
	Ret.push_back(Intersection(TempLines[1],TempLines[7]).front());
	Ret.push_back(Intersection(TempLines[1],TempLines[8]).front());
	Ret.push_back(Intersection(TempLines[0],TempLines[6]).front());
	Ret.push_back(Intersection(TempLines[0],TempLines[7]).front());
	Ret.push_back(Intersection(TempLines[0],TempLines[8]).front());
	
	// Right
	Ret.push_back(Intersection(TempLines[15],TempLines[9]).front());
	Ret.push_back(Intersection(TempLines[15],TempLines[10]).front());
	Ret.push_back(Intersection(TempLines[15],TempLines[11]).front());
	Ret.push_back(Intersection(TempLines[16],TempLines[9]).front());
	Ret.push_back(Intersection(TempLines[16],TempLines[10]).front());
	Ret.push_back(Intersection(TempLines[16],TempLines[11]).front());
	Ret.push_back(Intersection(TempLines[17],TempLines[9]).front());
	Ret.push_back(Intersection(TempLines[17],TempLines[10]).front());
	Ret.push_back(Intersection(TempLines[17],TempLines[11]).front());
	
	// Top
	Ret.push_back(Intersection(TempLines[12],TempLines[3]).front());
	Ret.push_back(Intersection(TempLines[12],TempLines[4]).front());
	Ret.push_back(Intersection(TempLines[12],TempLines[5]).front());
	Ret.push_back(Intersection(TempLines[13],TempLines[3]).front());
	Ret.push_back(Intersection(TempLines[13],TempLines[4]).front());
	Ret.push_back(Intersection(TempLines[13],TempLines[5]).front());
	Ret.push_back(Intersection(TempLines[14],TempLines[3]).front());
	Ret.push_back(Intersection(TempLines[14],TempLines[4]).front());
	Ret.push_back(Intersection(TempLines[14],TempLines[5]).front());

	return Ret;
}


void PutDotsOnOriginal(const vector<Point2D> & Dots, ARImage & Output)
{
	vector<Point2D> Block1, Block2, Block3;
	int i = 0;
	for (vector<Point2D>::const_iterator itr = Dots.begin() ; itr != Dots.end(); ++i, ++itr)
	{
		if (i/9 == 0)
			Block1.push_back(*itr);
		else if (i/9 == 1)
			Block2.push_back(*itr);
		else
			Block3.push_back(*itr);
	}
	PutDotsOnOriginal("cyan",Block1,Output);
	PutDotsOnOriginal("magenta",Block2,Output);
	PutDotsOnOriginal("yellow",Block3,Output);
}


void PutDotsOnOriginal(const char * colour, const vector<Point2D> & Dots, ARImage & Output)
{
	Magick::Image im;
	double RValue =1.0;
	im.read(Output.x_size,Output.y_size, "RGB", Magick::CharPixel, Output.data);
	if (Dots.size() > 1)
	{
		for (vector<Point2D>::const_iterator itr = Dots.begin(); itr != --Dots.end();++itr)
		{
			vector<Point2D>::const_iterator itr2 = itr;
			itr2++;
			if (itr == Dots.begin())
			{
				im.strokeColor(colour); // Outline color
				im.strokeWidth(2);						
			}
			else
			{
				im.strokeColor(Magick::ColorRGB(RValue,0,0)); // Outline color
				RValue*=0.9;
				im.strokeWidth(2);						
			}
			im.draw(Magick::DrawableLine(itr->x,itr->y,itr2->x,itr2->y));
		}		
	}
	im.write(0,0,im.columns(),im.rows(), "RGB", Magick::CharPixel, Output.data);
}

void DisplaySegments(ARImage & Output, list<vector<Point2D> > & Segments)
{
	// generate output image of large segments
	int PixelValue = 0;
	for (list<vector <Point2D> >::iterator itr = Segments.begin(); itr != Segments.end(); ++itr)
	{
		PixelValue ++;
		for (vector<Point2D>::iterator itr2 = itr->begin(); itr2 != itr->end();++itr2)
		{
			Output.GetPixel(static_cast<int> (itr2->x),static_cast<int> (itr2->y)) = PixelValue %256;
		}
	}
}

void DisplaySegments(ARImage & Output, list<ImageSegment> & Segments)
{
	// generate output image of large segments
	int PixelValue = 0;
	for (list<ImageSegment>::iterator itr = Segments.begin(); itr != Segments.end(); ++itr)
	{
		PixelValue ++;
		for (vector<Point2D>::const_iterator itr2 = itr->GetPoints().begin(); itr2 != itr->GetPoints().end();++itr2)
		{
			Output.GetPixel(static_cast<int> (itr2->x),static_cast<int> (itr2->y)) = PixelValue %256;
		}
	}
}

void ThresholdSegments(list<vector<Point2D> > & Segments, unsigned int MinSize)
{
	for (list<vector<Point2D> >::iterator itr = Segments.begin(); itr != Segments.end(); itr->size() < MinSize ? itr = Segments.erase(itr) : ++itr);
}

list<ImageSegment> GetSegmentInfo(list<vector<Point2D> > & Segments)
{
	list<ImageSegment> Ret;
	for (list<vector <Point2D> >::iterator itr = Segments.begin(); itr != Segments.end(); ++itr)
	{
		Ret.push_back(ImageSegment(*itr));
	}
	return Ret;
}

// apply a set of rules to the segments, hard coded for now
void ApplyRules(ARImage & ThresholdImage, list<ImageSegment> & Segments)
{
	bool Fail;
	int i = 0;
	for (list<ImageSegment>::iterator itr = Segments.begin(); itr != Segments.end(); (Fail ? itr = Segments.erase(itr) : ++itr), ++i)
	{	
		Fail = false;
		double HWRatio = itr->GetBoundingBox().GetHWRatio();
		double Solidity = itr->GetSolidity();
		printf("% 3d: HW=%f Solid=%f\n",i, HWRatio, Solidity);
		if (HWRatio < 0.5 || HWRatio > 2)
		{
			printf("Fail HW\n");
			Fail = true;
			continue;
		}
		if (Solidity < 0.7 || Solidity > 0.95)
		{
			printf("Fail Solid\n");
			Fail = true;
			continue;
		}
		int NumHoles = CountHoles(ThresholdImage, *itr);
		if (NumHoles != 1)
		{
			printf("% 3d: Fail %d\n",++i, NumHoles);
			Fail = true;
			continue;
		}
	}
}

int CountHoles(ARImage & ThresholdImage, ImageSegment aSegment,unsigned int MinSize)
{
	BoundingBox StartBB = aSegment.GetBoundingBox();
	list<vector<Point2D> > Holes = doLabelSegments(ThresholdImage, StartBB, SEG_EQUAL);
	list<ImageSegment> HoleDetails = GetSegmentInfo(Holes);
// remove segments that touch the edge
	int HoleCount = 0;
	for (list<ImageSegment>::iterator itr = HoleDetails.begin(); itr != HoleDetails.end(); ++itr)
	{
		BoundingBox bb = itr->GetBoundingBox();
		if (bb.GetLeft() <= StartBB.GetLeft()+1)
			continue;
		if (bb.GetRight() >= StartBB.GetRight()-1)
			continue;
		if (bb.GetTop() <= StartBB.GetTop()+1)
			continue;
		if (bb.GetBottom() >= StartBB.GetBottom()-1)
			continue;
		if (itr->GetPoints().size() >= MinSize)
			++HoleCount;
	}

	return HoleCount;
}

void LabelSegments(list<ImageSegment> & Segments)
{
	for (list<ImageSegment>::iterator itr = Segments.begin(); itr != Segments.end(); ++itr)
	{
		Point2D Center = itr->GetBoundingBox().GetCenter();
		Point2D CenterOfMass = itr->GetCenterOfMass();
		Point2D Offset = Center-CenterOfMass;
		// check if we are left right or up down
		if (fabs(Offset.x) > fabs(Offset.y))
		{
			if (Offset.x < 0)
				itr->SetLabel(MARKER_LEFT);
			else
				itr->SetLabel(MARKER_RIGHT);
		}
		else
		{
			if (Offset.y < 0)
				itr->SetLabel(MARKER_UP);
			else
				itr->SetLabel(MARKER_DOWN);
		}
		printf("%f %f %f %f %f %f %d\n", Center.x, Center.y, CenterOfMass.x,CenterOfMass.y, Offset.x, Offset.y, itr->GetLabel());
	}
}

void DrawLabelledSegments(ARImage & Output, list<ImageSegment> & Segments)
{
	// generate output image of large segments
	for (list<ImageSegment>::iterator itr = Segments.begin(); itr != Segments.end(); ++itr)
	{
		for (vector<Point2D>::const_iterator itr2 = itr->GetPoints().begin(); itr2 != itr->GetPoints().end();++itr2)
		{
			Output.GetPixel(static_cast<int> (itr2->x),static_cast<int> (itr2->y)) = 128;
		}
		BoundingBox bb = itr->GetBoundingBox();
		switch (itr->GetLabel())
		{
			case MARKER_LEFT:
				for (unsigned int y = bb.GetTop(); y < bb.GetBottom(); ++y)
					Output.GetPixel(bb.GetLeft(),y) = 255;
				break;

			case MARKER_RIGHT:
				for (unsigned int y = bb.GetTop(); y < bb.GetBottom(); ++y)
					Output.GetPixel(bb.GetRight(),y) = 255;
				break;

			case MARKER_UP:
				for (unsigned int x = bb.GetLeft(); x < bb.GetRight(); ++x)
					Output.GetPixel(x,bb.GetTop()) = 255;
				break;

			case MARKER_DOWN:
				for (unsigned int x = bb.GetLeft(); x < bb.GetRight(); ++x)
					Output.GetPixel(x,bb.GetBottom()) = 255;
				break;
		}
	}
}


void DrawLabelledSegmentsGreen(ARImage & Output, list<ImageSegment> & Segments)
{
	// generate output image of large segments
	for (list<ImageSegment>::iterator itr = Segments.begin(); itr != Segments.end(); ++itr)
	{
		BoundingBox bb = itr->GetBoundingBox();
		switch (itr->GetLabel())
		{
			case MARKER_LEFT:
				for (unsigned int y = bb.GetTop(); y < bb.GetBottom(); ++y)
				{
					Output.GetPixel(bb.GetLeft()*3,y) = 255;
					Output.GetPixel((bb.GetLeft()+1)*3,y) = 255;
				}
				break;

			case MARKER_RIGHT:
				for (unsigned int y = bb.GetTop(); y < bb.GetBottom(); ++y)
				{
					Output.GetPixel((bb.GetRight())*3,y) = 255;
					Output.GetPixel((bb.GetRight()+1)*3,y) = 255;
				}
				break;

			case MARKER_UP:
				for (unsigned int x = bb.GetLeft(); x < bb.GetRight(); ++x)
				{
					Output.GetPixel(x*3,bb.GetTop()) = 255;
					Output.GetPixel((x+1)*3,bb.GetTop()) = 255;
				}
				break;

			case MARKER_DOWN:
				for (unsigned int x = bb.GetLeft(); x < bb.GetRight(); ++x)
				{
					Output.GetPixel(x*3,bb.GetBottom()) = 255;
					Output.GetPixel((x+1)*3,bb.GetBottom()) = 255;
				}
				break;
		}
	}
}

void Normalise(ARImage & Input, ARImage & Output)
{
	uint8_t Min = 0xff;
	uint8_t Max = 0;
	for (unsigned int ii = 0; ii < Input.GetDataSize(); ++ii)
	{
		if (Input.data[ii] > Max)
			Max = Input.data[ii];
		if (Input.data[ii] < Min)
			Min = Input.data[ii];
	}
	if (Min == Max)
		Output = Input;
	else
		for (unsigned int ii = 0; ii < Input.GetDataSize(); ++ii)
			Output.data[ii] = (Input.data[ii] - Min) * 255/(Max-Min);
}

ImageSegment FillHoles(ImageSegment & Input, ARImage & Output)
{
	ImageSegment Ret;
	
	BoundingBox StartBB = Input.GetBoundingBox();
	StartBB.SetTop(StartBB.GetTop()-2);
	StartBB.SetBottom(StartBB.GetBottom()+2);
	StartBB.SetLeft(StartBB.GetLeft()-2);
	StartBB.SetRight(StartBB.GetRight()+2);
	
	ARImage ThresholdImage;
	ThresholdImage = Output;
	ThresholdImage.Erase();

	list<ImageSegment> TempList;
	TempList.push_back (Input);
	DisplaySegments(ThresholdImage,TempList);
	list<vector<Point2D> > Holes = doLabelSegments(ThresholdImage, StartBB, SEG_EQUAL);	
	list<ImageSegment> HoleDetails = GetSegmentInfo(Holes);
	
	// merge segments that dont touch the edge
	for (list<ImageSegment>::iterator itr = HoleDetails.begin(); itr != HoleDetails.end(); ++itr)
	{
		BoundingBox bb = itr->GetBoundingBox();
		if (bb.GetLeft() <= StartBB.GetLeft()+1)
			continue;
		if (bb.GetRight() >= StartBB.GetRight()-1)
			continue;
		if (bb.GetTop() <= StartBB.GetTop()+1)
			continue;
		if (bb.GetBottom() >= StartBB.GetBottom()-1)
			continue;
		
		Ret.GetPointsRef().insert(Ret.GetPointsRef().begin(),itr->GetPoints().begin(),itr->GetPoints().end());
	}
	return Ret;
}

