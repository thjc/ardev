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

RenderPlayerLocalise::RenderPlayerLocalise(PlayerClientInterface & pci)
{
	Proxy = NULL;
	PlayerInterface = &pci;
}

RenderPlayerLocalise::~RenderPlayerLocalise()
{
	delete Proxy;
}

int RenderPlayerLocalise::Initialise(bool Active)
{
	if (!initialised)
	{
		RenderObject::Initialise(Active);
		PlayerInterface->AddChild(*this);
	}
	return 0;
}

void RenderPlayerLocalise::Terminate()
{
	RenderObject::Terminate();
	PlayerInterface->RemoveChild(*this);
}

void RenderPlayerLocalise::PlayerInit(PlayerClient * pci)
{
	assert(pci);
	Lock();
	delete Proxy;
	Proxy = new LocalizeProxy(pci,0);
	if (Proxy == NULL)
		dbg_print(ARDBG_ERR,"Error Creating Ptz Proxy\n");
	Unlock();
}

void RenderPlayerLocalise::PlayerTerm()
{
	Lock();
	delete Proxy;	
	Proxy = NULL;
	Unlock();
}

// eigen function from player stage playerv
// Compute eigen values and eigenvectors of a 2x2 covariance matrix
static void eigen(double cm[][2], double values[], double vectors[][2])
{
  double s = (double) sqrt(cm[0][0]*cm[0][0] - 2*cm[0][0]*cm[1][1] +
                           cm[1][1]*cm[1][1] + 4*cm[0][1]*cm[0][1]);
  values[0] = 0.5 * (cm[0][0] + cm[1][1] + s);
  values[1] = 0.5 * (cm[0][0] + cm[1][1] - s);
  vectors[0][0] = -0.5 * (-cm[0][0] + cm[1][1] - s);
  vectors[0][1] = -0.5 * (-cm[0][0] + cm[1][1] + s);
  vectors[1][0] = vectors[1][1] = cm[0][1];
}

void RenderPlayerLocalise::RenderTransparent()
{
	if (Proxy == NULL)
		return;

//	Proxy->Lock();
	
	// render each hypothesis
	for (unsigned int i = 0 ; i < Proxy->GetHypothCount(); ++i)
	{
		double alpha = 0.1+Proxy->GetHypoth(i).alpha*0.8;
		if (i == 0)
			glColor4f(0.1,0.8,0.1,alpha);
		else
			glColor4f(0.8,0.8,0.1,alpha);

		double cov[2][2], eval[2],evec[2][2];
		cov[0][0] = Proxy->GetHypoth(i).cov[0];//[0];
		cov[0][1] = 0;
		cov[1][0] = 0;
		//cov[0][1] = Proxy->GetHypoth(i).cov[0][1];
		//cov[1][0] = Proxy->GetHypoth(i).cov[1][0];
		cov[1][1] = Proxy->GetHypoth(i).cov[1];//[1];
		eigen(cov, eval, evec);
		// 3 sigma
		double sx = 0;
		double sy = 0;
		if (eval[0] > 0)
			sx = 3*sqrt(eval[0]);
		if (eval[1] > 0)
			sy = 3*sqrt(eval[1]);
		
		if (sx < 0.05)
			sx = 0.05;
		if (sy < 0.05)
			sy = 0.05;
		
		glPushMatrix();
		glTranslatef(Proxy->GetHypoth(i).mean.px,Proxy->GetHypoth(i).mean.py,0);
		glRotatef(RTOD(Proxy->GetHypoth(i).mean.pa),0,0,1);
		glBegin(GL_TRIANGLE_FAN);
		glVertex3f(0,0,0);
		for (double x = -sx; x < sx; x+=sx/50)
		{
			double ratio = x*x/(sx*sx);
			double y;
			if (ratio >= 0.99)
				y = 0;
			else
				y = sy*sqrt(1-ratio);
			glVertex3f(x,y,0);
		}
		for (double x = sx; x > -sx; x-=sx/50)
		{
			double ratio = x*x/(sx*sx);
			double y;
			if (ratio >= 0.99)
				y = 0;
			else
				y = sy*sqrt(1-ratio);
			glVertex3f(x,-y,0);
		}
		glVertex3f(-sx,0,0);
		glEnd();
		if (i == 0)
		{
			glColor4f(0.0,1.0,0.0,1.0);
			glBegin(GL_LINES);
			glVertex3f(0,0,0);
			glVertex3f(sx,0,0);
			glEnd();
		}
		glPopMatrix();
	}
	
//	Proxy->Unlock();
	
}
