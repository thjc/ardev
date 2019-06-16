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
#ifndef HOUGH_H
#define HOUGH_H

#include <ardev/ardev.h>

#include <list>
#include <vector>

#include <libthjc/matrix.h>
#include <libthjc/geometry.h>

using namespace std;


class BoundingBox
{
	public:
		BoundingBox(unsigned int aTop=0,unsigned int aBottom=0,unsigned int aLeft=0,unsigned int aRight=0) {top=aTop;bottom=aBottom;left=aLeft;right=aRight;};
		~BoundingBox() {};	

		unsigned int GetTop() const {return top;};
		unsigned int GetBottom() const {return bottom;};
		unsigned int GetLeft() const {return left;};
		unsigned int GetRight() const {return right;};

		unsigned int GetWidth() const {return right-left;};
		unsigned int GetHeight() const {return bottom-top;};

		double GetHWRatio() const {if (GetWidth() > 0) return static_cast<double> (GetHeight())/static_cast<double> (GetWidth()); else return 1000000;};
		unsigned int GetArea() const {return GetHeight()*GetWidth();};
		Point2D GetCenter() const {return Point2D(static_cast<double>(right+left)/2,static_cast<double>(bottom+top)/2);};

		void SetTop(int aTop) {top = aTop;};
		void SetBottom(int aBottom) {bottom = aBottom;};
		void SetLeft(int aLeft) {left = aLeft;};
		void SetRight(int aRight) {right = aRight;};
	

		BoundingBox & operator = (const BoundingBox & rhs) {top = rhs.top; bottom = rhs.bottom; left = rhs.left; right = rhs.right; return *this;};
	private:
		unsigned int top;
		unsigned int bottom;
		unsigned int left;
		unsigned int right;
};

class ImageSegment
{
	public:
		ImageSegment() {};
		ImageSegment(vector<Point2D> aPoints);
		~ImageSegment() {};

		void AddPoint(const Point2D & NewPoint);
		BoundingBox GetBoundingBox() const {return bb;};

		double GetSolidity() const {return static_cast<double> (Points.size())/bb.GetArea();};
		Point2D GetCenterOfMass() const {return CenterOfMass;};

		const vector<Point2D> & GetPoints() const {return Points;};
		vector<Point2D> & GetPointsRef() {return Points;};

		unsigned int GetLabel() const {return Label;};
		void SetLabel(unsigned int aLabel) {Label = aLabel;};


		ImageSegment & operator = (const ImageSegment & rhs) {bb = rhs.bb; Points = rhs.Points; CenterOfMass = rhs.CenterOfMass; Label = rhs.Label; return * this;}
	private:
		BoundingBox bb;
		vector<Point2D> Points;
		Point2D CenterOfMass;
		unsigned int Label;
};


int doHough(ARImage & Image, ARImage & Hough);

int doGray(ARImage & Image, ARImage & Output);

int doHue(Point2D TopLeft, Point2D BottomRight, int ClearColour, ARImage & Image, ARImage & Output);

int ExtractHue(int Colour, int Range, ARImage & Image, ARImage & Output);

int doThresh(ARImage & Image, ARImage & Output, int thresh);

int doHitMiss(ARImage & Input, ARImage & Output, int Size, int * Kernel);

int doThinning(ARImage & Input, ARImage & Output);

int doPruning(ARImage & Input, ARImage & Output, int Count);

list<vector<Point2D> > doLabelSegmentsGray(int range, ARImage & Input, ARImage & Output);

typedef enum SegmentMode {SEG_LESS, SEG_MORE, SEG_EQUAL};
list<vector<Point2D> > doLabelSegments(ARImage & Input, SegmentMode Mode, int range=0);
list<vector<Point2D> > doLabelSegments(ARImage & Input, BoundingBox ROI, SegmentMode Mode, int range=0);

vector<Point2D> getBestRect(ARImage & ThresholdImage, list<vector<Point2D> > & Segments, ARImage & Output);
vector<Point2D> getBestRect(ARImage & ThresholdImage, list<vector<Point2D> > & Segments);

vector<Point2D> doRemoveBiggestSegment(list<vector<Point2D> > & Segments, ARImage & Output);
vector<Point2D> doRemoveBiggestSegment(list<vector<Point2D> > & Segments);

void doGetCubeRegion(vector<Point2D> Segment, Point2D & TopLeft, Point2D & BottomRight, ARImage & Input, ARImage & Output);

Point2D doGetCenter(const vector<Point2D> & Segment);

void getLRU(int & Left, int & Right, int & Up, const vector<Point2D> & Segment1, const vector<Point2D> & Segment2, const vector<Point2D> & Segment3);

vector<Point2D> getNineDotCenters(list<vector<Point2D> > & Segments);

vector<Point2D> orderNineDotsLeft(vector<Point2D> & Dots);
vector<Point2D> orderNineDotsRight(vector<Point2D> & Dots);
vector<Point2D> orderNineDotsTop(vector<Point2D> & Dots);
vector<Point2D> orderNineDots(vector<Point2D> & Dots);

Matrix doGetTopPoints(Matrix & InputPoints, unsigned int NumPoints, ARImage & Output);

Matrix doOrderLinesByAngle(Matrix & InputPoints, ARImage & Output);

Matrix doOrderLinesByNormal(Matrix & InputPoints, ARImage & Output);

list<Line> doMatrixToLines(Matrix & InputPoints, ARImage & Reference);

list<Point2D> doIntersectLines(list<Line> & InputLines);

void PutDotsOnOriginal(const char * colour, const vector<Point2D> & Dots, ARImage & Output);
void PutDotsOnOriginal(const vector<Point2D> & Dots, ARImage & Output);

// display segments
void DisplaySegments(ARImage & Output, list<vector<Point2D> > & Segments);
void DisplaySegments(ARImage & Output, list<ImageSegment> & Segments);

// remove segments less than desired size
void ThresholdSegments(list<vector<Point2D> > & Segments, unsigned int MinSize);

list<ImageSegment> GetSegmentInfo(list<vector<Point2D> > & Segments);

// apply a set of rules to the segments, hard coded for now
void ApplyRules(ARImage & ThresholdImage, list<ImageSegment> & Segments);

int CountHoles(ARImage & ThresholdImage, ImageSegment aSegment, unsigned int MinSize=0);

enum {MARKER_UNKNOWN, MARKER_LEFT, MARKER_RIGHT, MARKER_UP, MARKER_DOWN};
void LabelSegments(list<ImageSegment> & Segments);

void DrawLabelledSegments(ARImage & Output, list<ImageSegment> & Segments);
void DrawLabelledSegmentsGreen(ARImage & Output, list<ImageSegment> & Segments);

void Normalise(ARImage & Input, ARImage & Output);

ImageSegment FillHoles(ImageSegment & Input, ARImage & Output);

#endif
