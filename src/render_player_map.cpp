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
#include <GL/gl.h>
#include <GL/glut.h>

#include <math.h>

#include <ardev/debug.h>
#include <assert.h>

RenderPlayerMap::RenderPlayerMap(PlayerClientInterface & pci, double _Height)
{
	Height = _Height;
	Proxy = NULL;
	PlayerInterface = &pci;
	GlList = -1;
}


RenderPlayerMap::~RenderPlayerMap()
{
	delete Proxy;
}


int RenderPlayerMap::Initialise(bool Active)
{
	if (!initialised)
	{
		RenderObject::Initialise(Active);
		PlayerInterface->AddChild(*this);
	}
	return 0;
}

void RenderPlayerMap::Terminate()
{
	RenderObject::Terminate();
	PlayerInterface->RemoveChild(*this);
}

void RenderPlayerMap::PlayerInit(PlayerClient * pci)
{
	assert(pci);
	Lock();
	delete Proxy;
	Proxy = new MapProxy(pci,0);
	Proxy->RequestMap();
	Unlock();
}

void RenderPlayerMap::PlayerTerm()
{
	Lock();
	delete Proxy;	
	Proxy = NULL;
	Unlock();
}


void RenderPlayerMap::RenderTransparent()
{
	if (Proxy == NULL)
		return;
		
//	Proxy->Lock();
	
	if (Proxy->GetWidth() <= 0 || Proxy->GetHeight() <= 0)
	{
//		Proxy->Unlock();
		return;
	}	
	
	if (GlList < 0)
	{
		GlList = glGenLists(1);
		glNewList(GlList, GL_COMPILE);
		
		glColor4f(0.6,0.6,1,0.5);
		
		double res = Proxy->GetResolution();
		double half_res = res/2.0;
		
		glBegin(GL_QUADS);
		for (unsigned int x = 0; x < Proxy->GetWidth(); x++)
		{
			for (unsigned int y = 0; y < Proxy->GetHeight(); ++y)
			{
				double dx = x - Proxy->GetWidth()/2 ;
				double dy = Proxy->GetHeight()/2 - y;

				if (Proxy->GetCell(x,y) ==1)
				{
					// render the base
					//printf("%f %f\n",dx*res-half_res, dy*res-half_res);
					glVertex3f(dx*res-half_res, dy*res-half_res,0);
					glVertex3f(dx*res-half_res, dy*res+half_res,0);
					glVertex3f(dx*res+half_res, dy*res+half_res,0);
					glVertex3f(dx*res+half_res, dy*res-half_res,0);
					if (Height > 0)
					{
						glColor4f(0.7,0.7,1,0.2);
						// if a height is specified render the top and sides as well
						glVertex3f(dx*res-half_res, dy*res-half_res,Height);
						glVertex3f(dx*res-half_res, dy*res+half_res,Height);
						glVertex3f(dx*res+half_res, dy*res+half_res,Height);
						glVertex3f(dx*res+half_res, dy*res-half_res,Height);
						
						
						if (x==0 || (Proxy->GetCell(x-1,y) !=1))
						{
							glVertex3f(dx*res-half_res, dy*res-half_res,0);
							glVertex3f(dx*res-half_res, dy*res+half_res,0);
							glVertex3f(dx*res-half_res, dy*res+half_res,Height);
							glVertex3f(dx*res-half_res, dy*res-half_res,Height);
						}
	
						if (y == Proxy->GetHeight() || (Proxy->GetCell(x,y+1) !=1))
						{
							glVertex3f(dx*res-half_res, dy*res+half_res,0);
							glVertex3f(dx*res+half_res, dy*res+half_res,0);
							glVertex3f(dx*res+half_res, dy*res+half_res,Height);
							glVertex3f(dx*res-half_res, dy*res+half_res,Height);
						}
	
						if (x == Proxy->GetWidth() || (Proxy->GetCell(x+1,y) !=1))
						{
							glVertex3f(dx*res+half_res, dy*res+half_res,0);
							glVertex3f(dx*res+half_res, dy*res-half_res,0);
							glVertex3f(dx*res+half_res, dy*res-half_res,Height);
							glVertex3f(dx*res+half_res, dy*res+half_res,Height);
						}
						
						if (y==0 || (!Proxy->GetCell(x,y-1) !=1))
						{
							glVertex3f(dx*res+half_res, dy*res-half_res,0);
							glVertex3f(dx*res-half_res, dy*res-half_res,0);
							glVertex3f(dx*res-half_res, dy*res-half_res,Height);
							glVertex3f(dx*res+half_res, dy*res-half_res,Height);
						}
					}
				}
			}
		}
		glEnd();
		glEndList();		
	}
	glCallList(GlList);
	
//	Proxy->Unlock();
}
