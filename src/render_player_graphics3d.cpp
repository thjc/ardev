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
#include <ardev/debug.h>


void Graphics3DRenderer::Render()
{
	GLfloat mat_specular[] = { 0,0,1, 1.0 };
	GLfloat mat_diffuse[] = { 0,0,1, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);		

	glLineWidth(1);		
		
	for (ClientElementsMap::const_iterator clientItr = ClientElements.begin(); clientItr != ClientElements.end(); ++clientItr)
	{
		for (vector<player_graphics3d_cmd_draw_t*>::const_iterator itr = clientItr->second.begin(); itr != clientItr->second.end(); ++itr)
		{
			if (static_cast<float> ((*itr)->color.alpha/255) < 0.01)
				continue;
				
			RenderOne(**itr);
		}
	}
}

void Graphics3DRenderer::RenderTransparent()
{
	GLfloat mat_specular[] = { 0,0,1, 1.0 };
	GLfloat mat_diffuse[] = { 0,0,1, 1.0 };
	GLfloat mat_shininess[] = { 50.0 };

	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);		

	glLineWidth(1);		
	
	for (ClientElementsMap::const_iterator clientItr = ClientElements.begin(); clientItr != ClientElements.end(); ++clientItr)
	{
		for (vector<player_graphics3d_cmd_draw_t*>::const_iterator itr = clientItr->second.begin(); itr != clientItr->second.end(); ++itr)
		{
			if (static_cast<float> ((*itr)->color.alpha/255) >= 0.01)
				continue;
			
			RenderOne(**itr);
		}
	}
}

void Graphics3DRenderer::RenderOne(player_graphics3d_cmd_draw_t & item)
{
	glColor4f(static_cast<float> (item.color.red)/255.0,static_cast<float> (item.color.green)/255.0,static_cast<float> (item.color.blue)/255.0,1.0-static_cast<float> (item.color.alpha)/255.0);
	switch(item.draw_mode)
	{
		case PLAYER_DRAW_POINTS:
			glBegin(GL_POINTS);
			break;
		case PLAYER_DRAW_LINES:
			glBegin(GL_LINES);
			break;
		case PLAYER_DRAW_LINE_STRIP:
			glBegin(GL_LINE_STRIP);
			break;
		case PLAYER_DRAW_LINE_LOOP:
			glBegin(GL_LINE_LOOP);
			break;
		case PLAYER_DRAW_TRIANGLES:
			glBegin(GL_TRIANGLES);
			break;
		case PLAYER_DRAW_TRIANGLE_STRIP:
			glBegin(GL_TRIANGLE_STRIP);
			break;
		case PLAYER_DRAW_TRIANGLE_FAN:
			glBegin(GL_TRIANGLE_FAN);
			break;
		case PLAYER_DRAW_QUADS:
			glBegin(GL_QUADS);
			break;
		case PLAYER_DRAW_QUAD_STRIP:
			glBegin(GL_QUAD_STRIP);
			break;
		case PLAYER_DRAW_POLYGON:
			glBegin(GL_POLYGON);
			break;
		default:
		{
			dbg_print(ARDBG_ERR,"Unknown graphics 3d draw mode\n");
			return;
		}
	}
		
	for (unsigned int i = 0; i < item.points_count; ++i)
	{
		glVertex3f(item.points[i].px,item.points[i].py,item.points[i].pz);
	}
	glEnd();	
	
}

void Graphics3DRenderer::Event(EventObject* event)
{
	if(event->GetEventType() == ROE_Clear)
	{
		for (ClientElementsMap::iterator clientItr = ClientElements.begin(); clientItr != ClientElements.end(); ++clientItr)
			clientItr->second.clear();
	}
}

