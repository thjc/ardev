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
#ifndef RENDER_TEAPOT_H
#define RENDER_TEAPOT_H

#include <ardev/ardev.h>
#include <GL/glut.h>

class CLoad3DS;
class t3DModel;
/** Basic Model renderer, currently only supports 3ds models (3d studio max)
 */
class RenderModel : public RenderObject
{
	public:
		/// Constructor takes the model file and a scale factor
		RenderModel(const char * ModelFile, const char * TextureBase = "", double Scale = 1);
		/// Destructor
		~RenderModel();

		/// The reimplemented Render method
		void Render();
		/// Performs the portion of the Initilaisation that needs to be done in the rendering thread (ie in the correct gl context etc)
		void ThreadInit();

	protected:
		double Scale;
		unsigned int * g_Texture ;                     // This holds the texture info, referenced by an ID

		char * TextureBase;
		char * ModelName;
		CLoad3DS * g_Load3ds;                                     // This is 3DS class.  This should go in a good model class.
		t3DModel * g_3DModel;                                     // This holds the 3D Model info that we load in
};


/** Render Object that renders an openGL teapot of the Given Colour and Size
 */
class RenderTeapot : public RenderObject
{
	public:
		/// Constructor with specified colour components and size
	RenderTeapot(const ARColour & _Colour, float _size)
	{
		Size = _size;
		Colour = _Colour;
	}

	void Render() ///< Rend the teapot in GL
	{
		GLfloat mat_specular[] = { Colour.r, Colour.g, Colour.b, Colour.a };
		GLfloat mat_diffuse[] = { Colour.r, Colour.g, Colour.b, Colour.a };
		GLfloat mat_shininess[] = { 50.0 };

		glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
		glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
		glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
		glColor3f(Colour.r, Colour.g, Colour.b);
		// rotate it so that it is upright in a vertical z world
		glRotatef(90,1,0,0);
		glutSolidTeapot(Size);
	}
};

class PositionTransformControls: public PositionRenderable
{
	public:
		PositionTransformControls() :PositionRenderable(ARPosition(),RPE_TransformControls,0){};
};

class RenderTransformControls : public RenderObject
{
	// TODO: refactor this class, require update of axis mask
	public:

	enum Control
	{
		XArrow,
		YArrow,
		ZArrow,
		RWheel,
		PWheel,
		YWheel
	};

	RenderTransformControls(int _control) : RenderObject()
	{
		RegisterForEvents();
		colourConfig=false;
		SetSelectable(true);

		switch(_control)
		{
			case 0: control = XArrow; break;
			case 1: control = YArrow; break;
			case 2: control = ZArrow; break;
			case 3: control = RWheel; break;
			case 4: control = PWheel; break;
			case 5: control = YWheel; break;
		}

		x=y=z=roll=pitch=yaw=false;
		first = true;
		firstResult = ARPosition();
	};

	bool x,y,z,roll,pitch,yaw;
	Control control;
	ARPoint mouse;
	int i;

	bool first;
	ARPosition firstResult; //fisrt click location
	ARPosition initialPos; //object location on first click
	ARPoint offset;

	float firstdist;
	float currentdist;

	/// Render the X, Y and Z axes
	void Render()
	{
		if(x && control == XArrow)
			Render(XArrow,0,255,255,255);
		if(y && control == YArrow)
			Render(YArrow,255,0,255,255);
		if(z&& control == ZArrow)
			Render(ZArrow,255,255,0,255);
//		if(roll&& control == RWheel)
//			Render(RWheel,0,255,255,255);
//		if(pitch&& control == PWheel)
//			Render(PWheel,255,0,255,255);
//		if(yaw&& control == YWheel)
//			Render(YWheel,255,255,0,255);
	}
	void RenderBounding()
	{
		PickingColour c = GetPickingColour();
		if(x && control == XArrow)
		{
			c.a = 1;
			Render(XArrow,c.r,c.g,c.b,c.a);
		}
		if(y && control == YArrow)
		{
			c.a = 2;
			Render(YArrow,c.r,c.g,c.b,c.a);
		}
		if(z && control == ZArrow)
		{
			c.a = 3;
			Render(ZArrow,c.r,c.g,c.b,c.a);
		}
//		if(roll && control == RWheel)
//		{
//			c.a = 4;
//			Render(RWheel,c.r,c.g,c.b,c.a);
//		}
//		if(pitch && control == PWheel)
//		{
//			c.a = 5;
//			Render(PWheel,c.r,c.g,c.b,c.a);
//		}
//		if(yaw && control == YWheel)
//		{
//			c.a = 6;
//			Render(YWheel,c.r,c.g,c.b,c.a);
//		}
	}

	void Render(Control c,unsigned char r, unsigned char  g, unsigned char  b, unsigned char  a)
	{
		switch(c)
		{
			case XArrow:
				glPushMatrix();
					glBegin(GL_LINES);
					glColor4ub(r,g,b,a);
					glVertex3f(0,0,0);
					glVertex3f(1.000,0,0);
					glEnd();
					glTranslatef(1.000,0,0);
					glRotatef(90,0,1,0);
					glutSolidCone(0.1,0.2,5,3);
				glPopMatrix();
				break;
			case YArrow:
				glPushMatrix();
					glBegin(GL_LINES);
					glColor4ub(r,g,b,a);
					glVertex3f(0,0,0);
					glVertex3f(0,1.000,0);
					glEnd();
					glTranslatef(0,1.000,0);
					glRotatef(-90,1,0,0);
					glutSolidCone(0.1,0.2,5,3);
				glPopMatrix();
				break;
			case ZArrow:
				glPushMatrix();
					glBegin(GL_LINES);
					glColor4ub(r,g,b,a);
					glVertex3f(0,0,0);
					glVertex3f(0,0,1.000);
					glEnd();
					glTranslatef(0,0,1.000);
					glutSolidCone(0.1,0.2,5,3);
				glPopMatrix();
				break;
			case RWheel:
				glPushMatrix();
					glRotatef(90,0,1,0);
					glColor4ub(r,g,b,a);
					glutSolidTorus(0.05,1.5,10,32);
				glPopMatrix();
				break;
			case PWheel:
				glPushMatrix();
				glColor4ub(r,g,b,a);
					glRotatef(90,1,0,0);
					glutSolidTorus(0.05,1.5,10,32);
				glPopMatrix();
				break;
			case YWheel:
				glPushMatrix();
				glColor4ub(r,g,b,a);
					glutSolidTorus(0.05,1.5,10,32);
				glPopMatrix();
				break;
		}
	};

	virtual void Event(EventObject* event)
	{
		if (event->GetEventType() == ROE_HideTransformControls)
		{
			SetEnabled(false);
		}
		else if(event->GetEventType() == ROE_ShowTransformControls)
		{
			SetEnabled(true);
		}
		else if(event->GetEventType() == ROE_ResetControls)
		{
			first = true;
			firstResult = ARPosition();

		}
		else if(event->GetEventType() == POE_Transform)
		{
			// Here be horrible code
			PickedEventObject* e = reinterpret_cast<PickedEventObject*>(event);
			if(Enabled() && e->GetPickedObject() == this)
			{
				if(first)
					initialPos = GetParent()->Pos->GetPosition();
				mouse = e->GetMouseCoordinates();

				// World position
				glClear( GL_DEPTH_BUFFER_BIT);
				GLint viewport[4];					// Where The Viewport Values Will Be Stored
				glGetIntegerv(GL_VIEWPORT, viewport);			// Retrieves The Viewport Values (X, Y, Width, Height)

				Vector3D world= OutputObject::UnProject(mouse.x,mouse.y);

				// Camera position
				ARPosition cam = e->GetCameraPosition();
				Vector3D camera = Vector3D(cam.Origin.x,cam.Origin.y,cam.Origin.z);

				Vector3D resultLeft,second;
				ARPoint resultDirection;
				double distLeft,distRight;
				double angle =0;
				ARPoint unit;
				if(control==XArrow || control==YArrow || control==ZArrow)
				{// this section calculates translation

					// Setup unit vectors for direction of movement
					switch(control)
					{
						case XArrow: unit = ARPoint(1,0,0); break;
						case YArrow: unit = ARPoint(0,1,0); break;
						case ZArrow: unit = ARPoint(0,0,1); break;
						default:break;
					}

					Vector3D rotated;
					Vector3D centre;

					ARPoint axis = unit.RotateRPY(-initialPos.Direction.x,-initialPos.Direction.y,-initialPos.Direction.z) + initialPos.Origin;
					rotated= Vector3D(axis.x,axis.y,axis.z);
					centre= Vector3D(initialPos.Origin.x,initialPos.Origin.y,initialPos.Origin.z);

					IntersectLines(camera,world,centre,rotated,resultLeft,second,distLeft,distRight);
					currentdist = distRight;
					printf("\n left: %f right: %f \nMovement <%f %f %f>\n", distLeft,distRight,second.x,second.y,second.z );
				}
				else
				{ /// this section calculates rotation
					switch(control)
					{
						case RWheel: resultDirection = ARPoint(1,0,0);break;
						case PWheel: resultDirection = ARPoint(0,1,0);break;
						case YWheel: resultDirection = ARPoint(0,0,1);break;
						default:break;
					}
					Vector3D left,right;

					ARPoint axis = resultDirection.RotateRPY(-initialPos.Direction.x,-initialPos.Direction.y,-initialPos.Direction.z);
					Vector3D rotated = Vector3D(axis.x,axis.y,axis.z);
					Vector3D centre = Vector3D(initialPos.Origin.x,initialPos.Origin.y,initialPos.Origin.z);

					Vector3D init = Vector3D(firstResult.Origin.x,firstResult.Origin.y,firstResult.Origin.z);

					// Intersect world with plane
					Plane p = Plane(Ray(centre,rotated));

					Vector3D dir = (world-camera).Normalise();
					printf("World: %f %f %f Dir %f %f %f\n",world.x,world.y,world.z,dir.x,dir.y,dir.z);
					printf("Camera: %f %f %f Dir %f %f %f\n",camera.x,camera.y,camera.z,dir.x,dir.y,dir.z);
					vector<Vector3D> hits = p.Intersect(Ray(camera,dir));
					if(!hits.empty())
					{
						second = hits[0];
						angle = -RTOD(atan2(second.y-init.y,second.x-init.x));
						printf("\npos: %f %f %f quanternion: %f %f %f %f",second.x, second.y,second.z,resultDirection.x,resultDirection.y,resultDirection.z, angle );
					}

				}
				printf("first %d\n",first);
				// Set position if not the first input mouse point
				if(!first)
				{
					ARPosition pos = GetParent()->Pos->GetPosition();
					if(control==XArrow || control==YArrow || control==ZArrow)
					{
						pos.Origin.x = initialPos.Origin.x + unit.x * -(firstdist - currentdist);
						pos.Origin.y = initialPos.Origin.y + unit.y * -(firstdist - currentdist);
						pos.Origin.z = initialPos.Origin.z + unit.z * -(firstdist - currentdist);
					}
					else
					{
						pos.Direction.x = resultDirection.x * (initialPos.Direction.x + angle );
						pos.Direction.y = resultDirection.y * (initialPos.Direction.y + angle );
						pos.Direction.z = resultDirection.z * (initialPos.Direction.z + angle );
					}
					// Publish event to move object
					printf("\nInitial Position: %f %f %f  %f %f %f\n",initialPos.Origin.x,initialPos.Origin.y,initialPos.Origin.z,initialPos.Direction.x,initialPos.Direction.y,initialPos.Direction.z);
					printf("Position: %f %f %f  %f %f %f\n",pos.Origin.x,pos.Origin.y,pos.Origin.z,pos.Direction.x,pos.Direction.y,pos.Direction.z);
					ARDev::PublishEvent(new PositionEventObject(POE_SelectedPosition,pos,ARPosition()));
				}
				else
				{
					angle=0;
					first = false;
					// Get position on click

					// Setup initial position based on the Intersect Lines first point, so all movement can be relative to that
					// offset to correct slightly offset trig
					firstResult = ARPosition(ARPoint(second.x,second.y,second.z),initialPos.Direction);
					firstdist = currentdist;
					printf("\nInitial Position: %f %f %f  %f %f %f\n",initialPos.Origin.x,initialPos.Origin.y,initialPos.Origin.z,initialPos.Direction.x,initialPos.Direction.y,initialPos.Direction.z);
					//printf("Position: %f %f %f  %f %f %f\n",pos.Origin.x,pos.Origin.y,pos.Origin.z,pos.Direction.x,pos.Direction.y,pos.Direction.z);

				}
			}
		}
		else if(event->GetEventType() == ROE_TransformMask)
		{
			PickedEventObject* p = reinterpret_cast<PickedEventObject*>(event);
			ARPosition m = p->GetRenderMask();
			x=m.Origin.x;
			y=m.Origin.y;
			z=m.Origin.z;

			roll=m.Direction.x;
			pitch=m.Direction.y;
			yaw=m.Direction.z;
		}
	};
};

#endif
