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
#include <ardev/player.h>

#include <math.h>

#include <ardev/debug.h>
#include <assert.h>

#include "render_grid.h"

/// This function contains the standard rendering information and allos transparency, Render() can be used it you don't require transparency
void RenderGrid::RenderTransparent()
{
	float width = Major*size;
	float height = Major*size;
	float Minor = Major/Intervals;

	float Z = 0;
	double x, y;

	// draw the grid
	glPushMatrix();

	if(!circular)
	{
		// render Minor grid
		glLineWidth(0.5f);

		glBegin(GL_LINES);
		// x
		for (float i=0; i<= width/2; i+= Minor)
		{
			glColor4f(Colour.r-0.2, Colour.g-0.2, Colour.b-0.2, Colour.a*0.40f);
			glVertex3f(i, -height/2, Z);
			glVertex3f(i, height/2, Z);
		}
		for (float i=0; i>= -width/2; i-= Minor)
		{
			glColor4f(Colour.r-0.2, Colour.g-0.2, Colour.b-0.2, Colour.a*0.40f);
			glVertex3f(i, -height/2, Z);
			glVertex3f(i, height/2, Z);
		}

		// y
		for (float j=0; j<= height/2; j+= Minor)
		{
			glColor4f(Colour.r-0.2, Colour.g-0.2, Colour.b-0.2, Colour.a*0.40f);
			glVertex3f(-width/2, j, Z);
			glVertex3f(width/2, j, Z);
		}
		for (float j=0; j>= -height/2; j-= Minor)
		{
			glColor4f(Colour.r-0.2, Colour.g-0.2, Colour.b-0.2, Colour.a*0.40f);
			glVertex3f(-width/2, j, Z);
			glVertex3f(width/2, j, Z);
		}
		glEnd();

		// render Major grid
		glLineWidth(1);


		glBegin(GL_LINES);
		// x
		for (float i=0; i<= width/2; i+= Major)
		{
			glColor4f(Colour.r, Colour.g, Colour.b, Colour.a);
			glVertex3f(i, -height/2, Z);
			glVertex3f(i, height/2, Z);
		}
		for (float i=0; i>= -width/2; i-= Major)
		{
			glColor4f(Colour.r, Colour.g, Colour.b, Colour.a);
			glVertex3f(i, -height/2, Z);
			glVertex3f(i, height/2, Z);
		}

		// y
		for (float j=0; j<= height/2; j+= Major)
		{
			glColor4f(Colour.r, Colour.g, Colour.b, Colour.a);
			glVertex3f(-width/2, j, Z);
			glVertex3f(width/2, j, Z);
		}
		for (float j=0; j>= -height/2; j-= Major)
		{
			glColor4f(Colour.r, Colour.g, Colour.b, Colour.a);
			glVertex3f(-width/2, j, Z);
			glVertex3f(width/2, j, Z);
		}
		glEnd();

		glLineWidth(1);
	}
	else
	{
		for(float c= 0;c<= size* Major; c += Major)
		{
			glColor4f(Colour.r, Colour.g, Colour.b, Colour.a);
			glBegin(GL_LINE_LOOP);

			for (int i=0; i < 360; i+=5)
			{
				float r = i * 3.14159/180;
				glVertex3f(cos(r)*c,sin(r)*c,Z);
			}
			glEnd();
		}

		glBegin(GL_LINES);
		for(float d= 0;d< 360; d += Intervals)
		{
			glColor4f(Colour.r, Colour.g, Colour.b, Colour.a);
			float r = d * 3.14159/180 ;
			glVertex3f(0,0,Z);
			glVertex3f(cos(r)*size* Major,sin(r)*size* Major,Z);
		}
		glEnd();
	}
	glPopMatrix();

	if(measuring)
	{
		glPushMatrix();
			glLineWidth(3);
			glBegin(GL_LINES);
				// first
				glColor3f(1,1,1);
				glVertex3f(first.x,first.y,-0.25);
				glVertex3f(first.x,first.y,0.25);

				// line in between
				glColor3f(1,1,0);
				glVertex3f(first.x,first.y,0);
				glVertex3f(second.x,second.y,0);

				// second
				glColor3f(1,1,1);
				glVertex3f(second.x,second.y,-0.25);
				glVertex3f(second.x,second.y,0.25);
			glEnd();
			glLineWidth(1);
		glPopMatrix();
	}
}

/// This function is used for Rendering a bounding shape
/// This function should not contain glColor* as this will completely break picking, as cause false positives when clicking
void RenderGrid::RenderBounding()
{
	float width = Major*size;
	float height = Major*size;
	float Minor = Major/Intervals;

	// draw a simple plane
	glBegin(GL_QUADS);
	glVertex3f(width/2.0f,height/2.0f,0);
	glVertex3f(-width/2.0f,height/2.0f,0);
	glVertex3f(-width/2.0f,-height/2.0f,0);
	glVertex3f(width/2.0f,-height/2.0f,0);
	glEnd();
}

/// This function is called when the GUI is setup
/// and allows a render object to have configurable variables
#ifdef HAVE_ANTTWEAKBAR
void RenderGrid::DisplaySetup(TweakBar* tweakBar, const char* Name)
{

	static int guii=0;
	char uid[30];
	char args[500];
	
	this->tweakBar = tweakBar; // Keep track of tweakbar so we can add to it.

	snprintf(uid,30,"rendergrid%d",++guii);
	snprintf(args,500," group='%p' label='Major' min='1' max='100' step='0.5' help='Major grid markings' ",this);
	/// Add a variable to the display bar, with callback functions
	TwAddVarCB(tweakBar->GetDisplayBar(), strdup(uid) ,TW_TYPE_FLOAT,SetMajorTW,GetMajorTW,this, strdup(args) );

	snprintf(uid,30,"rendergrid%d",++guii);
	snprintf(args,500," group='%p' label='Minor' min='0' max='100' step='1' help='Minor grid markings' ",this);
	/// Add a variable to the display bar, with callback functions
	TwAddVarCB(tweakBar->GetDisplayBar(), strdup(uid) ,TW_TYPE_FLOAT,SetMinorTW,GetMinorTW,this, strdup(args) );

	snprintf(uid,30,"rendergrid%d",++guii);
	snprintf(args,500," group='%p' label='Size' min='1' max='100' step='2' help='Size of the grid' ",this);
	/// Add a variable to the display bar, with callback functions
	TwAddVarCB(tweakBar->GetDisplayBar(), strdup(uid) ,TW_TYPE_INT8,SetSizeTW,GetSizeTW,this, strdup(args) );

}
#endif

/// This function handles events
void RenderGrid::Event(EventObject* event)
{
	/// If event is Transform events
	if (event->GetEventType() == ROE_HideTransformControls)
	{
		measuring = false;
	}
	else if(event->GetEventType() == ROE_ShowTransformControls)
	{
		measuring = true;
	}
	else if(event->GetEventType() == POE_Transform)
	{
		// Cast to picked event object to retrieve info
		PickedEventObject* e = reinterpret_cast<PickedEventObject*>(event);

		// Caution: This code here is reasonable complex and not required for event handling which is actually just the above code
		if(Enabled() && measuring && e->GetPickedObject() == this)
		{
			ARPoint mouse = e->GetMouseCoordinates();

			// World position
			Vector3D world= OutputObject::UnProject(mouse.x,mouse.y);

			// Camera position
			ARPosition cam = e->GetCameraPosition();
			Vector3D camera = Vector3D(cam.Origin.x,cam.Origin.y,cam.Origin.z);

			ARPoint zaxis = ARPoint(0,0,1);
			ARPosition pos = GetParent()->Pos->GetPosition();

			zaxis = zaxis.RotateRPY(pos.Direction.x,pos.Direction.y,pos.Direction.z);
			Vector3D dir = Vector3D(zaxis.x,zaxis.y,zaxis.z);
			Plane p = Plane(Ray(Vector3D(pos.Origin.x,pos.Origin.y,pos.Origin.z),dir));

			Vector3D intersect = p.Intersect(Ray(camera, world-camera))[0];
			printf("World Coordinate: %f %f %f\n",intersect.x,intersect.y,intersect.z);

			if(i%2==0)
				first = ARPoint(intersect.x,intersect.y,intersect.z);
			else
				second = ARPoint(intersect.x,intersect.y,intersect.z);

			i++;
			distance = sqrt ((pow (second.x - first.x, 2) + pow (second.y - first.y, 2) + pow (second.z - first.z, 2)));
			//printf("Distance: %.2f m\n",distance);
#ifdef HAVE_ANTTWEAKBAR
			/// Add a variable to the select bar, to show distance
			TwRemoveAllVars(tweakBar->GetSelectBar());
			char label[100];
			snprintf(label,100," label='%s' ",GetParent()->Name);
			TwAddButton(tweakBar->GetSelectBar(),"bName",NULL,NULL,label);
			TwAddVarRO(tweakBar->GetSelectBar(), "selectgrid0" ,TW_TYPE_FLOAT,&distance," label='Distance Measured' help='Distance Measured' " );
#endif // HAVE_ANTTWEAKBAR
		}
	}
}

/// This is the render grid handler and sets up the code required to add configurations to aride
RenderGridHandler::RenderGridHandler() :
	// Variable("Name","Description","DefaultValue")
	Colour("Colour", "Colour", "1 1 1 0.2"),
	Major("Major", "Major Intervals", "1"),
	Minor("Divisions", "Major Interval Divisions", "2"),
	Size("Size", "Grid Size", "10"),
	Text("Labels","Grid Labels","0"),
	Rect("Circular","Grid Shape","0")
{
	/// Add to parameters list
	Parameters.push_back(&Colour);
	Parameters.push_back(&Major);
	Parameters.push_back(&Minor);
	Parameters.push_back(&Size);
	Parameters.push_back(&Text);
	Parameters.push_back(&Rect);
	obj = NULL;
}

/// cleans up the resources
RenderGridHandler::~RenderGridHandler()
{
	delete obj;
}

/// This is how the handler works as a factory to produce a RenderGrid object
RenderObject & RenderGridHandler::GetObject()
{
	if (obj == NULL)
	{
		obj = new RenderGrid(Colour.Value,Major.Value,Minor.Value,Size.Value,Text.Value,Rect.Value);

		if (obj == NULL)
			throw aride_exception(ARDEV_ALLOC_ERROR,__FUNCTION__,__LINE__);
	}
	BaseObject = obj;
	CurrentProject->InitialiseObject(obj);
	return *obj;
}

/// This is the funciton to cleanup the RenderGrid object
void RenderGridHandler::RemoveObject()
{
	delete obj;
	obj = NULL;
	BaseObject = obj;
}


