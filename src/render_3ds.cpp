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
#include <ardev/render_base.h>
#include <GL/gl.h>
#include <GL/glut.h>

#include <math.h>
#include "3ds.h"

#include <ardev/debug.h>
RenderModel::RenderModel(const char * ModelFile, const char * _TextureBase, double _Scale)
{
	Scale = _Scale;
	g_Load3ds = new CLoad3DS;
	g_3DModel = new t3DModel;
	g_Texture = NULL;
	TextureBase = strdup(_TextureBase);
	ModelName = strdup(ModelFile);
	
	
	if (g_Load3ds == NULL || g_3DModel == NULL || TextureBase == NULL || ModelName == NULL)
	{
		dbg_print(ARDBG_ERR,"Failed to allocate model structures\n");
		return;
	}
	
	memset(g_3DModel, 0, sizeof(struct t3DModel));
	
    // First we need to actually load the .3DS file.  We just pass in an address to
    // our t3DModel structure and the file name string we want to load ("face.3ds").

    g_Load3ds->Import3DS(g_3DModel, ModelName);         // Load our .3DS file into our model structure
	
}

RenderModel::~RenderModel()
{
    // Go through all the objects in the scene
	if (g_3DModel)
	{
		for(int i = 0; i < g_3DModel->numOfObjects; i++)
		{
			// Free the faces, normals, vertices, and texture coordinates.
			delete [] g_3DModel->pObject[i].pFaces;
			delete [] g_3DModel->pObject[i].pNormals;
			delete [] g_3DModel->pObject[i].pVerts;
			delete [] g_3DModel->pObject[i].pTexVerts;
		}
	}	
	delete g_Load3ds;
	delete g_3DModel;
	delete [] g_Texture;
	delete TextureBase;
}

void RenderModel::ThreadInit()
{
    // Depending on how many textures we found, load each one (Assuming .BMP)
    // If you want to load other files than bitmaps, you will need to adjust CreateTexture().
    // Below, we go through all of the materials and check if they have a texture map to load.
    // Otherwise, the material just holds the color information and we don't need to load a texture.
	if (!(g_Texture = new unsigned int [g_3DModel->numOfMaterials]))
	{
		dbg_print(ARDBG_ERR,"Failed to allocate texture list\n");
		return;
	}
	
    // Go through all the materials
    for(int i = 0; i < g_3DModel->numOfMaterials; i++)
    {
        // Check to see if there is a file name to load in this material
        if(strlen(g_3DModel->pMaterials[i].strFile) > 0)
        {
			char * TempTexName = new char[strlen(g_3DModel->pMaterials[i].strFile) + strlen(TextureBase) + 1];
			strcpy(TempTexName,TextureBase);
			strcat(TempTexName,g_3DModel->pMaterials[i].strFile);
            // Use the name of the texture file to load the bitmap, with a texture ID (i).
            // We pass in our global texture array, the name of the texture, and an ID to reference it. 
            CreateTexture(g_Texture, TempTexName, i);           
			delete [] TempTexName;
        }

        // Set the texture ID for this material
        g_3DModel->pMaterials[i].texureId = i;
    }
};


void RenderModel::Render()
{
	// first we scale the model
	glScalef(0.1*Scale,0.1*Scale,0.1*Scale);
	glRotatef(-90,0,0,1);
	glRotatef(-90,1,0,0);

	// then render it
    glEnable(GL_LIGHT0);                                // Turn on a light with defaults set
    glEnable(GL_LIGHTING);                              // Turn on lighting
    glEnable(GL_COLOR_MATERIAL);                        // Allow color

    // I am going to attempt to explain what is going on below up here as not to clutter the 
    // code below.  We have a model that has a certain amount of objects and textures.  We want 
    // to go through each object in the model, bind it's texture map to it, then render it.
    // To render the current object, we go through all of it's faces (Polygons).  
    // What is a face you ask?  A face is just (in this case) a triangle of the object.
    // For instance, a cube has 12 faces because each side has 2 triangles.
    // You might be thinking.  Well, if there are 12 faces in a cube, that makes
    // 36 vertices that we needed to read in for that object.  Not really true.  Because
    // a lot of the vertices are the same, since they share sides, they only need to save
    // 8 vertices, and ignore the duplicates.  Then, you have an array of all the
    // unique vertices in that object.  No 2 vertices will be the same.  This cuts down
    // on memory.  Then, another array is saved, which is the index numbers for each face,
    // which index in to that array of vertices.  That might sound silly, but it is better
    // than saving tons of duplicate vertices.  The same thing happens for UV coordinates.
    // You don't save duplicate UV coordinates, you just save the unique ones, then an array
    // that index's into them.  This might be confusing, but most 3D files use this format.
    // This loop below will stay the same for most file formats that you load, so all you need
    // to change is the loading code.  You don't need to change this loop (Except for animation).

    // Since we know how many objects our model has, go through each of them.
    for(int i = 0; i < g_3DModel->numOfObjects; i++)
    {
        // Make sure we have valid objects just in case. (size() is in the vector class)
        if(g_3DModel->pObject.size() <= 0) break;

        // Get the current object that we are displaying
        t3DObject *pObject = &g_3DModel->pObject[i];
            
        // Check to see if this object has a texture map, if so bind the texture to it.
        if(pObject->bHasTexture) {

            // Turn on texture mapping and turn off color
            glEnable(GL_TEXTURE_2D);

            // Reset the color to normal again
            glColor3ub(255, 255, 255);

            // Bind the texture map to the object by it's materialID
            glBindTexture(GL_TEXTURE_2D, g_Texture[pObject->materialID]);
        } else {

            // Turn off texture mapping and turn on color
            glDisable(GL_TEXTURE_2D);

            // Reset the color to normal again
            glColor3ub(255, 255, 255);
        }

        // This determines if we are in wireframe or normal mode
        glBegin(GL_TRIANGLES);                    // Begin drawing with our selected mode (triangles or lines)

            // Go through all of the faces (polygons) of the object and draw them
            for(int j = 0; j < pObject->numOfFaces; j++)
            {
                // Go through each corner of the triangle and draw it.
                for(int whichVertex = 0; whichVertex < 3; whichVertex++)
                {
                    // Get the index for each point of the face
                    int index = pObject->pFaces[j].vertIndex[whichVertex];
            
                    // Give OpenGL the normal for this vertex.
                    glNormal3f(pObject->pNormals[ index ].x, pObject->pNormals[ index ].y, pObject->pNormals[ index ].z);
                
                    // If the object has a texture associated with it, give it a texture coordinate.
                    if(pObject->bHasTexture) {

                        // Make sure there was a UVW map applied to the object or else it won't have tex coords.
                        if(pObject->pTexVerts) {
                            glTexCoord2f(pObject->pTexVerts[ index ].x, pObject->pTexVerts[ index ].y);
                        }
                    } else {

                        // Make sure there is a valid material/color assigned to this object.
                        // You should always at least assign a material color to an object, 
                        // but just in case we want to check the size of the material list.
                        // if the size is at least one, and the material ID != -1,
                        // then we have a valid material.
                        if(g_3DModel->pMaterials.size() && pObject->materialID >= 0) 
                        {
                            // Get and set the color that the object is, since it must not have a texture
                            BYTE *pColor = g_3DModel->pMaterials[pObject->materialID].color;

                            // Assign the current color to this model
                            glColor3ub(pColor[0], pColor[1], pColor[2]);
                        }
                    }

                    // Pass in the current vertex of the object (Corner of current face)
                    glVertex3f(pObject->pVerts[ index ].x, pObject->pVerts[ index ].y, pObject->pVerts[ index ].z);
                }
            }

        glEnd();                                // End the drawing
    }
	

    glDisable(GL_LIGHT0);                                // Turn on a light with defaults set
    glDisable(GL_LIGHTING);                              // Turn on lighting
    glDisable(GL_COLOR_MATERIAL);                        // Allow color
}
