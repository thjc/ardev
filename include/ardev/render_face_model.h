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
#include "config.h"
#ifdef HAVE_LIBFEP

/**
 * Model.h	(Model Header File)
 *
 * Author:			Christopher Bertram 
 * Created:			15th May 2004
 * Modified:		18th July 2004
 */
#if !defined(_MODEL_H)
#define _MODEL_H

using namespace std;
#define MAX_TEXTURES 1

#include <fep/fep.h>
#include <ardev/ardev.h>


/** Render Object that renders an openGL face of the Given Colour and Size
 */
class RenderFace : public RenderObject
{
	public:
		/// Constructor  
		RenderFace(const char * ModelFile = "model/chris.wfm",float _Size = 0.1, bool debug=false)
		{
			face = new VirtualFace(ModelFile, 800, 600, (VIRTUALFACE_MANUAL), debug);		
			Size=_Size;
		};
		
		~RenderFace()
		{
			delete face;
		}; ///< Destructor
	
		/// Randers the face in GL
		void Render()
		{
			if (face)				
			{
				glEnable(GL_TEXTURE_2D);
        		glShadeModel(GL_SMOOTH);// Enables Smooth Color Shading
        		glEnable(GL_LINE_SMOOTH);
        		glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);	
				glScalef(Size,Size,Size);
				glRotatef(-90,0,0,1);
				glRotatef(-90,1,0,0);			
				face->Render(TextureID);
				glDisable(GL_TEXTURE_2D);
				glDisable(GL_LINE_SMOOTH);
			}
			
		}; 
		
		void ThreadInit()
		{
			glEnable(GL_TEXTURE_2D);
			TextureID = face->createTexture();
			glDisable(GL_TEXTURE_2D);			
		}
	
		/// Pointer to the Face
		VirtualFace * face;
		/// Face texture ID
		unsigned int TextureID;

};


#endif //- !defined(_MODEL_H)
#endif
