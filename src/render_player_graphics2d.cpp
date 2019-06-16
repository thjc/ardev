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

void Graphics2DRenderer::Render()
{
	GLfloat mat_specular[] = { 0,0,1, 1.0 };
	GLfloat mat_diffuse[] = { 0,0,1, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);		

//	glDepthFunc(GL_LEQUAL);
	glLineWidth(2);		
	
	double z = 0;
	
	for (ClientElementsMap::const_iterator clientItr = ClientElements.begin(); clientItr != ClientElements.end(); ++clientItr)
	{
		for (vector<GraphicsElement>::const_iterator itr = clientItr->second.begin(); itr != clientItr->second.end(); ++itr)
		{
			if (itr->Type == PLAYER_GRAPHICS2D_CMD_POLYGON && itr->Filled && itr->fa > 0.99)
			{
				glColor4f(itr->fr,itr->fg,itr->fb,itr->fa);		
				glBegin(GL_POLYGON);	
				for (vector<player_point_2d_t>::const_iterator point_itr = itr->Points.begin(); point_itr != itr->Points.end(); ++point_itr)
				{
					glVertex3f(point_itr->px,point_itr->py,z);
				}		
				glEnd();
			}
			
			if (itr->a < 0.99)
				continue;
				
			glColor4f(itr->r,itr->g,itr->b,itr->a);
			switch(itr->Type)
			{
				case PLAYER_GRAPHICS2D_CMD_POINTS:
					glBegin(GL_POINTS);
					break;
				case PLAYER_GRAPHICS2D_CMD_POLYLINE:
					glBegin(GL_LINE_STRIP);
					break;
				case PLAYER_GRAPHICS2D_CMD_POLYGON:
					glBegin(GL_LINE_LOOP);
					break;
				default:
					continue;
			}
			for (vector<player_point_2d_t>::const_iterator point_itr = itr->Points.begin(); point_itr != itr->Points.end(); ++point_itr)
			{
				glVertex3f(point_itr->px,point_itr->py,z+0.0005);
			}
			glEnd();
			z+=0.001;
		}
	}
}

void Graphics2DRenderer::RenderTransparent()
{
	GLfloat mat_specular[] = { 0,0,1, 1.0 };
	GLfloat mat_diffuse[] = { 0,0,1, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);		

	glLineWidth(2);		
	double z = 0;
	
	for (ClientElementsMap::const_iterator clientItr = ClientElements.begin(); clientItr != ClientElements.end(); ++clientItr)
	{
		for (vector<GraphicsElement>::const_iterator itr = clientItr->second.begin(); itr != clientItr->second.end(); ++itr)
		{
			if (itr->Type == PLAYER_GRAPHICS2D_CMD_POLYGON && itr->Filled && itr->fa < 0.99)
			{
				glColor4f(itr->fr,itr->fg,itr->fb,itr->fa);		
				glBegin(GL_POLYGON);	
				for (vector<player_point_2d_t>::const_iterator point_itr = itr->Points.begin(); point_itr != itr->Points.end(); ++point_itr)
				{
					glVertex3f(point_itr->px,point_itr->py,z);
				}		
				glEnd();
			}
			
			if (itr->a > 0.99)
				continue;
				
			glColor4f(itr->r,itr->g,itr->b,itr->a);
			switch(itr->Type)
			{
				case PLAYER_GRAPHICS2D_CMD_POINTS:
					glBegin(GL_POINTS);
					break;
				case PLAYER_GRAPHICS2D_CMD_POLYLINE:
					glBegin(GL_LINE_STRIP);
					break;
				case PLAYER_GRAPHICS2D_CMD_POLYGON:
					glBegin(GL_LINE_LOOP);
					break;
				default:
					continue;
			}
			for (vector<player_point_2d_t>::const_iterator point_itr = itr->Points.begin(); point_itr != itr->Points.end(); ++point_itr)
			{
				glVertex3f(point_itr->px,point_itr->py,z+0.0005);
			}
			glEnd();
			z+= 0.001;
		}
	}
}

void Graphics2DRenderer::Event(EventObject* event)
{
	if(event->GetEventType() == ROE_Clear)
	{
		for (ClientElementsMap::iterator clientItr = ClientElements.begin(); clientItr != ClientElements.end(); ++clientItr)
			clientItr->second.clear();
	}
}
