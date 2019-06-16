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
/*

This file contains the doxygen global documentation including
the main page and some general description pages.

*/

/*! \mainpage ARDev Documentation
 
\section intro_sec Introduction
%ARDev is an augmented reality toolkit to assist robot
developers in debugging complex robot sensors.

It primarily provides sensor visualisations at this stage.

The currently implemented objects primarily work with
Player/Stage based robots.

%ARDev has been developed and tested in ubuntu linux,
other flavours of linux should work fine but are not
tested.

%ARDev was written by Toby Collett as part of his PhD research
at the University of Auckland. TI am keen to hear any feedback,
and should be able to provide
some personal support getting it running, so get in touch by
email to ardev _at_ plan9.net.nz

There
are three ways that you can use %ARDev:
 -# As an intelligent debugging space (IDS)
 -# As an integrated library with your robot code
 -# As a library for other augmented reality purposes

The getting started guide will focus mainly on using the toolkit as an
IDS via the aride utility or the player module. The API documentation
will help you with options 2 and 3. The test application in the src 
folder is also a good place to look for examples of using the library
directly.

\section sec_help_guide Using this manual

This manual is split into three main parts:
 - for the library API documentation check out \ref ardev_library
 - for a guide to using %ARDev as a debugging space look at \ref ardev_ids
 - the remainder of the documentation is for the included utilities:
   - \ref util_calib

\section sec_pub Related Literature

There have been a couple of papers published relating to this toolkit, at HRI2006 and ICRA2006 respectively. Biblographic references (and possibly the papers) are available from http://www.ece.auckland.ac.nz/~tcol036, just follow the menu to research->Augmented Reality.
 
\section install_sec Installation

%ARDev uses a the standard auto tools system for installation. First 
download and install libthjc (it should be available from the same place
you got ARDev).

The other required libraries are:
 - QT (>= 4.1)
 - OpenGL, GLU and GLUT
 - SDL
 - libjpeg
 - imagemagick

Other optional libraries that you may wish to install include:
 - doxygen (to generate this documentation)
 - libdc1394 (>= 2.0-pre6, for firewire camera support)
 - ffmpeg (for movie capture support, versioning for this can get interesting)
 - artoolkit (for tracking artoolkit markers)
 - artoolkitplus (for tracking artoolkitplus markers, recommended)
 - libfep (the FEP virtual face, if your interested in what this is contact the university of auckland robotics group)
 - Player/Stage (for interface to player based robots, recommended)
 - opencv (For coloured blob tracking)

After you have installed the required and desired optional libraries you simply need
to run ./configure and make.

All things going well you are now ready to proceed to the getting started guide.

 
\section sec_getting_started Getting Started
There are two basic methods described here, one just gives an idea of the
software using the stage based debug objects for capturing. This is likely
to work first try out of the box, but is not that useful.

Option 2 uses an overhead camera and a player based robot. This is more useful
but more complicated. Probably trying option 1 first and then progressing
to option 2 once thats done is a good plan.

Both the methods described here use the aride GUI for configuring ardev so to
get started go to the aride directory (util/aride), then follow either option
1 or 2 below.

\subsection sec_getting_started_1 OPTION 1
You will need player (tested with 2.0.1), with at least the fake localize
driver, and stage (2.0.1)

Run stage using the config file in util/aride/example/simple.cfg

then run aride specifing the stage aride config file
i.e. ./aride ./example/stage.aride

then click on control->run and it should be all go.
You should be able to driver the robot using playerjoy or playerv etc and
watch the visualisation of the sensor data. This is simulating the ardev
system running on the robots on board camera.

\subsection sec_getting_started_2 OPTION 2
This is based on player and the artoolkitplus fiducial tracker.

My example uses our labs pioneer robot, with a player server for serving up the camera images.
The default artoolkitplus marker is ID '10', this can be changed in the config GUI.

Change to the aride directory.
run ./aride example/overhead.aride
(sleepy is our pioneer, we have seven...sleepy, happy, grumpy ... got it?)
check the settings the UI, particularly you will need to change the server addresses 
for the player interfaces.

you will also need to specify a camera calibration file the format is:
Translation (3 floats)
Rotation Matrix (9 floats)
focal distance
sensor width (x and y)
image size (x and y)
scale factors (x and y)

Fields are sepearated by white space.
the application ./util/calib/calibapp may help you generate this file. It uses tsai calibration to get the values
and does basic detection of circles on a large cube to find the source points.
If you are not using the artoolkitplus or opencv trackers then you may be able to omit the calibration file altogether.

you could generate the file manually the most important value is the 'tilt' or y axes rotation values.
an example calibration file is also provided in the example dir.

in theory, provided your parameters are all correct and your player servers are running, simply click on
the green play button ('start') and things should be all go.

Im keen on people using this software so dont hesitate to contact me if you are wanting to try the software and
are having trouble. This project is under active development and any ideas voiced will certainly be considered.

\section sec_other Other Notes
3rd party projects used/referred to, 
Ive attempted to keep this list fairly complete,
but there may be projects I have referred to
online that I have missed out, please let me know if you spot any.

Supported Projects:

 - Player/Stage
 - HITlab NZ, ARtoolkit
 - ARToolKitPlus
 - OpenCV

References Used:

 - NeHe Studios (lots of varied reference for open gl sections
 - DigiBen/Gametutorials.com (the 3ds model loader)


 */

/*! \page ardev_library Using ARDev as a library

This page describes the use of ARDev as a library integrated into an exisitng application. This usage of the system also allows for the powerful creation of 
custom visualisations, and also for highly customised uses of the library.

The debugging space concept is one example of a custom use of the ARDev library
there are many more possibilities both using robots, and also in other application areas.

Detailed documentation of the individual classes is described in the classes page.
This is generated with doxygen directly from the source, so should be kept up to date. The purpose of this page is to give an overview of how the library is expected
to be used, hoepfully people will also start using it in some unexpected ways, in which case this page can be used as a starting point to break the mold from.

\section ardev_library_basics The ARDev Library Basics
The ARDev system is designed to be very flexible and modular, designed with the goal of supporting a wide range of AR hardware setups. The three specific setups in mind are, video see through AR, both head mounted and fixed camera, and optical see through.

There are seven key objects that are used in the AR system, any object that is optional has a NULL implementation that can be used as a place holder. The key objects are:
 - ARDev: this manages the AR session and performs high level control operations.
 - OutputObject: Creates the display and manages the OpenGL context
 - CaptureObject: Grabs the real world frame, NULL for optical see through
 - FrameProcessObject: Allows for pre and post processing of frames, particularly for fiducial tracking.
 - CameraObject: Encapsulates the intrinsic camera parameters, this can work with the calibration utility, or alternatively existing calibration applications can be used.
 - PositionObject: Defines a 3d position and orientation in space, these can be chained together to create a tree structure.
 - RenderObject: Responsible for the actual rendering of the virtual object.

The two key objects from this list are the ARDev object which manages the Augmented reality system and the OutputObject that manages the individual rendering pipeline for a display.

\section ardev_library_usage Using the library

This section provides a fairly simple example of using the toolkit in an application.
This example will look at rendering the openGL teapot on a fiducial marker in the environment. In this case we are using a fixed camera with a static calibration.

This first step is to set up the AR environment, we need to have some standard includes:
\code
#include <ardev/ardev.h>
#include <ardev/capture.h>
#include <ardev/output_x11.h>
#include <ardev/render_base.h>
\endcode

It is probably a good idea to turn on a reasonable level of debugging.
\code
	ARDev::DebugLevel = ARDBG_INFO;
\endcode
You could also set this to ARDBG_VERBOSE for more debugging info.

Then we get into the real initialisation, first set up the capture object, we are going to use firewire in this case.

\code
	CaptureObject * cap = new CaptureDC1394();
\endcode

Next we initialise a static camera object based on a static calibration.
\code
	ARCamera arcam("static.calib");
	CameraConstant cam(arcam);

	PositionConstant * CamPosition;
	ARPosition CameraOffsetConst(arcam.Origin,arcam.Direction);
	CamPosition = new PositionConstant(CameraOffsetConst);
	CamPosition->Initialise();
\endcode

Now that we have the basics for grabbing a realworld snapshot we can create the output object and initialise the display.

\code
	OutputObject * out = new OutputX11((CaptureObject*)cap,&cam,CamPosition,800,600,":0",FullScreen);
\endcode

Having created our display we can now can inistialise the ARDev object

\code
	ARDev::Start(out,"overhead");
\endcode

At this stage we should now have a live stream from the camera, albiet an overly complex way of getting this if that was all we wanted. So now we want to render our augmented data. So the first step is to create the fiducial tracker and register it with the system.

\code
	ARToolKitPlusPreProcess * artkp_pre;
	artkp_pre = new ARToolKitPlusPreProcess(cam);
	artkp_pre->Initialise();

	out->AddPre(artkp_pre);
	
	PositionObject * RobotPos = new ARToolKitPlusPosition(*artkp_pre,11,0.31);
	RobotPos->Initialise();
\endcode

And now to create the render object.

\code
	ARColour Blue(0,0,255,0);
	RenderTeapot Teapot(Blue,0.01);
\endcode

Finally we create the link between the position object and render object and add them to the render list.

\code
	ARDev::Add(RenderPair(&Teapot,RobotPos),"overhead");
\endcode

After this you would have your normal application code, once you are ready to stop the AR rendering simply call the following.

\code
	ARDev::Stop("overhead");
\endcode

\section ardev_library_extension Extending ARDev

To add new objects to ARDev simply subclass the base for the object type and implement the requried methods.
The methods that are required will vary from class to class but there generally are only one or two.

The new objects can then simply be used as described above. 

*/

/*! \page ardev_ids Using ARDev as an Intelligent Debugging Space (IDS)

There are two steps to using %ARDev as an intelligent debugging space. The 
first step uses the \ref util_aride tool to create an XML configuration file for the
space, this includes the AR environment (output, capture, camera and processing objects)
and the displaylists that are to be rendered. You could create the XML file by hand, but
I wouldnt recommend it, and wont be documenting the process at this stage.

The second stage is running the AR environment. If you want support for the graphics_2d or graphics_3d
renderings then you will need to run it as a player module(\ref util_ardev_player). 
If you dont need this support (or want to test your configuration without it) you can use the 
\ref util_aride tool to create the AR renderings.

@todo document utilities

*/



