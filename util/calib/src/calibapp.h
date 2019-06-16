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

/*! \page util_calib The ARDev Calibration utility

The calibration utility included with ardev is based on the tsai calibration algorithm. The 
calib application can be used in two modes, either specifying the calibration points manually or have
the utility detect them.

For either method, you will need an image from the camera. You can either:
Capture this image yourself and load it in.
File->Open Image
Or:
Grab an image from a Player server. To do this, you must first run a Player server with the camera driver.
(player ~/PlayerConfig/1394.cfg on gin if you are at the UoA Robotics lab)
Then enter the server address in the Video Server box and the camera index (probably 0) into the Index box.
Toggle Acquire to start the video feed. Once you are happy with the image, untoggle Acquire.

\section manual_calib Manual Point Entry

For manual calibration you will need two point files. The first contains the 3D locations of 27
calibration points and the second file contains the pixel coordinates of the same 27 points (in the same
order).

First load the 3D points
File->Load 3D Points

Then position the 2d points by placing them on the image. Toggle the Place Points button to begin. Click
on each marker in the image to place a point there. The number of the next point to be placed is
displayed, along with its position in 3D space. If a point is not correctly placed, enter its number and
click in the correct position.

The 2D points can be saved using
File->Save 2D Points
And loaded at a later date using
File->Load 2D Points

You should now be able to check the 2D points are at the correct locations. they will be divided into 
three sets because the automatic method uses three faces of a cube, but this is not really important here.

Now proceed to the calibration section

\section auto_calib Automatic Point Acquisition

If you are using the magic calibartion cube in the UoA robotics lab, or have a similar large cube with 
white dots on it (add a picture of the cube) then you will want to use the automatic method.

Make sure you have an image as described earlier.

Press Process Image to detect the markers. After its chugged away for a while the detected points will be
displayed, confirm they seem correct then proceed to the next section.

If you dont get good points then use the Show stage button and stage spin box to get an idea of what went 
wrong.

\section calibrate Camera Calibration

After you have followed the steps for manual or automatic point acquisition all you should need to do is 
click calibrate. When the calibration is complete the 3D point set will be projected with the new
camera model and the results displayed as black dots on the original image. Confirm these are accurate
and then save your calibration.
File->Save

*/



#ifndef CALIBAPP_H
#define CALIBAPP_H

#include "../ui/calibapp_base.h"


class Q3CanvasSprite;
class Q3Canvas;
class Q3CanvasView;

#include <qlistview.h>

#undef signals
#include <libplayerc++/playerc++.h>
#define signals protected

using namespace PlayerCc;

#include "camera.h"
#include <ardev/ardev.h>
#include <ardev/capture.h>
#include <libthjc/geometry.h>


class CalibApp : public QMainWindow, public Ui_CalibApp
{
    Q_OBJECT

public:
	CalibApp(int argc, char **argv, QWidget* parent = 0, Qt::WFlags flags = 0);
	virtual ~CalibApp() {};

	void LoadCalibPoints();

	// Command Line args
	int argc;
	char ** argv;

public slots:
	void Idle();	

	void OriginalClicked();
	void ProcessImageClicked();
	void ShowStageClicked();
	void CalibrateClicked();

	void calibChanged();

	
	void AcquireToggled(bool);
	
	void PlacingToggled(bool);

	void fileOpen();
	void fileSaveAs();
	
	void Load2DPoints();
	void Save2DPoints();
	void Load3DPoints();

	void ImageClicked(int x, int y);
	void PointChanged();

protected:
	void ShowImage(const ARImage & im);

	ARImage Original;//, Threshold, Hough;
	ARImage Original_Backup;//, Threshold, Hough;
#define NumStages 15
	ARImage Stages[NumStages];

	list<Line> Lines;
	vector<Point2D> OrderedPoints;

	bool Capturing;
	QTimer * AcquireTimer;

	// Player Position Variables
	PlayerClient * PClient;
	Position3dProxy * Pos;
	
	// Image Capture
	PlayerClient * camServer;
	CameraProxy * cp;
	ARImage Acquired;
	
	// Storage for real world calibration Points
	double CubePoints[27][3];
	double CubeOffset[3];
	bool LoadedPoints;

	// Calibration Results
	Camera cam;

	bool CalibrationDone;
};



#endif // CALIBAPP_H
