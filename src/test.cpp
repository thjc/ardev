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


#include <ardev/ardevconfig.h>
#ifdef HAVE_ARTOOLKITPLUS
#include <ardev/artoolkitplus.h>
#endif

#include <fstream>
#include <iostream>

#include <ardev/opencv_blobtrack.h>
#include <ardev/player.h>
#include <ardev/ardev.h>
#include <ardev/capture.h>
#include <ardev/output_x11.h>
#include <ardev/output_ffmpeg.h>
#include <ardev/render_base.h>


#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <ardev/debug.h>

using namespace std;


int main(int argc, const char** argv)
{
	ARDevInit(argc, argv );

	ARDev::DebugLevel = ARDBG_INFO;
	bool UsePlayer = false;
	bool UseSonar = false;
	bool UsePath = false;
	bool Use1394 = false;
	bool UseARToolkitPlus = false;
	bool ShowAxes = false;
	bool CVDebug = false;
	bool Virtual = false;
	bool FullScreen = false;
	bool JustVideo = false;
	const char * CalibFile = "tv.calib";
	const char * RobotHost = "sleepy";
	const char * CameraHost = "camerahost";
	const char * StaticImage = NULL;
	const char * MovieFile = NULL;

	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i],"-d")==0 && ++i < argc)
		{
			ARDev::DebugLevel = atoi(argv[i]);
		}
		else if (strcmp(argv[i],"--player")==0)
		{
			UsePlayer = true;
		}
		else if (strcmp(argv[i],"--camera-host")==0 && ++i < argc)
		{
			CameraHost = argv[i];
		}
		else if (strcmp(argv[i],"--movie-file")==0 && ++i < argc)
		{
			MovieFile = argv[i];
		}
		else if (strcmp(argv[i],"--1394")==0)
		{
			Use1394 = true;
		}
		else if (strcmp(argv[i],"--calib")==0 && ++i < argc)
		{
			CalibFile = argv[i];
		}
		else if (strcmp(argv[i],"--host")==0 && ++i < argc)
		{
			RobotHost = argv[i];
		}
		else if (strcmp(argv[i],"--cvdebug")==0)
		{
			CVDebug = true;
		}
		else if (strcmp(argv[i],"--virtual")==0)
		{
			Virtual = true;
		}
		else if (strcmp(argv[i],"--axes")==0)
		{
			ShowAxes = true;
		}
		else if (strcmp(argv[i],"--fullscreen")==0)
		{
			FullScreen = true;
		}
		else if (strcmp(argv[i],"--sonar")==0)
		{
			UseSonar = true;
		}
		else if (strcmp(argv[i],"--path")==0)
		{
			UsePath = true;
		}
		else if (strcmp(argv[i],"--artoolkitplus")==0)
		{
			UseARToolkitPlus = true;
		}
		else if (strcmp(argv[i],"--justvideo")==0)
		{
			JustVideo = true;
		}
		else if (strcmp(argv[i],"--static-image")==0 && ++i < argc)
		{
			StaticImage = argv[i];
		}
	}

	try
	{
		//set up the capture, camera, output and pre process objects
		CaptureObject * cap;
		if (Virtual)
		{
			cap = new CaptureNull(ARColour(0,0,0));
		}
#ifdef HAVE_IMAGEMAGICK
		else if (StaticImage)
		{
			cap = new CaptureFile(StaticImage);
		}
#endif
		else if (UsePlayer)
		{
			printf("connecting to player camera\n");
			cap = new CapturePlayer(CameraHost,6665,0);
		}
		else if (Use1394)
		{
#ifdef HAVE_DC1394
			printf("connecting to 1394 camera\n");
			cap = new CaptureDC1394();
#else
			printf("dc1394 support not built, terminating\n");
			exit (0);
#endif
		}
		else
		{
#ifdef HAVE_LINUX_VIDEODEV_H
			printf("creating video for linux capture device\n");
			cap = new CaptureV4L("/dev/video0",640,480,2,0);
#else
			printf("v4l support not built, terminating\n");
			exit (0);
#endif

		}
		assert(cap);
		printf("Now initalise the capture device\n");
		cap->Initialise();

		ARCamera arcam(CalibFile);
		CameraConstant cam(arcam);

		printf("Connecting to robot: %s\n",RobotHost);
		int port = 6665;
		PlayerClientInterface * player = new PlayerClientInterface("PlayerClientInterface0",1,(const char**)&RobotHost,&port);
		player->Initialise();

		PositionConstant * CamPosition;
		// set up the geometry of the camera
		if (Virtual)
		{
			ARPosition CameraOffsetConst(ARPoint(-3,0,2),ARPoint(0,-M_PI/6,0));
			CamPosition = new PositionConstant(CameraOffsetConst);
			CamPosition->Initialise();
		}
		else
		{
			ARPosition CameraOffsetConst(arcam.Origin,arcam.Direction);
			CamPosition = new PositionConstant(CameraOffsetConst);
			CamPosition->Initialise();
		}

		OutputObject * out = new OutputX11((CaptureObject*)cap,&cam,CamPosition,800,600,":0","",FullScreen);
		ARDev::Start(out,"overhead");

		PositionObject * RobotPos = NULL;
		PositionObject * RobotPos2 = NULL;

		if (MovieFile)
		{
#ifdef HAVE_FFMPEG
			OutputMovie * Recorder = new OutputMovie(MovieFile,800,600);
			Recorder->Initialise(true);
			out->AddPost(Recorder);
#else
			printf("ffmpeg support not compiled\n");
#endif
		}

		if (Virtual)
		{
			ARPosition TempOffest(ARPoint(0,0,0),ARPoint(0,0,0));
			RobotPos = new PositionConstant(TempOffest);
		}
		else if (UseARToolkitPlus)
		{
#ifdef HAVE_ARTOOLKITPLUS

			ARToolKitPlusPreProcess * artkp_pre;
			artkp_pre = new ARToolKitPlusPreProcess(cam);
			artkp_pre->Initialise();
			printf("Initialised ARToolKitPlus preprocess, now add it to the output object\n");
			out->AddPre(artkp_pre);

			RobotPos = new ARToolKitPlusPosition(*artkp_pre,13,0.31);
			RobotPos->Initialise();
//			RobotPos2 = new ARToolKitPlusPosition(*artkp_pre,0,0.31);
//			RobotPos2->Initialise();
			printf("Initialised ARToolKitPlus position\n");
#else
			printf("ARToolkitPlus support not compiled\n");
			return 0;
#endif
		}
		else
		{
#ifdef WITH_OPENCV
			OpenCVBlobTrackPreProcess pre(cam,200,10000,CVDebug);
			pre.Initialise();
			out->AddPre(&pre);

			BlobPair_t RobotBlob;
			RobotBlob.Hue1_Min = 165;
			RobotBlob.Hue1_Max = 175;
			RobotBlob.Hue2_Min = 95;
			RobotBlob.Hue2_Max = 105;
			RobotBlob.Height = 0.31;
			RobotPos = new OpenCVBlobTrackPosition(pre,RobotBlob);
			RobotPos->Initialise();
#else
			printf("OpenCV support not built\n");
#endif
		}
		if (!RobotPos2)
			RobotPos2 = RobotPos;

		ARPosition OriginOffsetConst(ARPoint(0,0,0),ARPoint(0,0,0));
		PositionConstant OriginOffset(OriginOffsetConst);

		// set up the geometry of the robot
		ARPosition RobotOffsetConst(ARPoint(0.15,0,-0.31),ARPoint(0,0,0));
		PositionConstant RobotOffset(RobotOffsetConst);

		ARPosition SonarPosConst(ARPoint(0,0,0.20),ARPoint(0,0,0));
		PositionConstant SonarPos(SonarPosConst);

		ARPosition LaserPosConst(ARPoint(0,0,0.13),ARPoint(0,0,0));
		PositionConstant LaserPos(LaserPosConst);

		RobotOffset.Next = RobotPos;
		LaserPos.Next = &RobotOffset;
		SonarPos.Next = &RobotOffset;

		// create render objects
		RenderObject * path;
		if (UsePath)
		{
			ARColour PathColour(0,1,0);
			path = new RenderPlayerPath(*player,PathColour);
			path->Initialise();
		}
		else
			path = NULL;
		RenderObject * sonar;
		if (UseSonar)
		{
			sonar = new RenderPlayerSonar(*player);
			sonar->Initialise();
		}
		else
			sonar = NULL;
		RenderObject * laser = new RenderPlayerLaser(*player, ARColour(1,0,0));
		laser->Initialise();
		RenderObject * axes = new RenderAxes();
		axes->Initialise();

		printf("Created Render Objects\n");

	 	if (!JustVideo)
	 	{
		 	if (ShowAxes)
		 	{
				ARDev::Add(RenderPair(axes,RobotPos),"overhead");
				ARDev::Add(RenderPair(axes,RobotPos2),"overhead");
				ARDev::Add(RenderPair(axes,&OriginOffset),"overhead");
		 	}
			if (UsePath)
				ARDev::Add(RenderPair(path,&RobotOffset),"overhead");
			ARDev::Add(RenderPair(laser,&LaserPos),"overhead");
			if (UseSonar)
				ARDev::Add(RenderPair(sonar,&SonarPos),"overhead");
	 	}
		getchar();

		printf("Stopping stuff\n");
		ARDev::DebugLevel = ARDBG_VERBOSE;

		ARDev::Stop("overhead");

		delete axes;
		delete sonar;
		delete laser;
		delete path;
		delete out;
		delete cap;
		delete player;

		return 0;
	}
	catch (const char * e)
	{
		printf("Unhandled Exception: %s\n",e);
		return 1;
	}
	catch (const PlayerCc::PlayerError & e)
	{
		cout << e;
		printf("Unahndled error in c++ client\n");
	}
	catch (...)
	{
		cout << "Unknown exception caught" << endl;
	}


}
