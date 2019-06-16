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

#include <geos_c.h>

#include <config.h>

/** Dummy function passed as a function pointer GEOS when it is initialised. GEOS uses this for logging. */
inline void geosprint(const char* format, ...)
{
	va_list ap;
	va_start(ap,format);
	fprintf(stderr,"GEOSError: ");
	vfprintf(stderr,format, ap);
	fflush(stderr);
	va_end(ap);
};

RenderPlayerVectorMap::RenderPlayerVectorMap(PlayerClientInterface & pci, double _Height, const ARColour& _Colour, set<int> renderLayers,int _Index) :
	Index(_Index),
	Proxy (NULL),
	Height(_Height),
	GlList(-1)
{
	Colour = _Colour;
	PlayerInterface =&pci;

	RenderLayers = renderLayers;
	initGEOS(geosprint, geosprint);
}


RenderPlayerVectorMap::~RenderPlayerVectorMap()
{
	finishGEOS();
	delete Proxy;
}


int RenderPlayerVectorMap::Initialise(bool Active)
{
	if (!initialised)
	{
		RenderObject::Initialise(Active);
		PlayerInterface->AddChild(*this);
	}
	return 0;
}

void RenderPlayerVectorMap::Terminate()
{
	RenderObject::Terminate();
	PlayerInterface->RemoveChild(*this);
}

void RenderPlayerVectorMap::PlayerInit(PlayerClient * pci)
{
	assert(pci);
	Lock();
	delete Proxy;
	Proxy = new VectorMapProxy(pci,Index);
	Proxy->GetMapInfo();
	Unlock();
}

void RenderPlayerVectorMap::PlayerTerm()
{
	Lock();
	delete Proxy;
	Proxy = NULL;
	Unlock();
}


void RenderPlayerVectorMap::Render()
{
	glColor4f(Colour.r, Colour.g, Colour.b, Colour.a);
	if (Colour.a > 0.99) // If we aren't see through
		RenderTransparent(); // Do a solid render so we fill the depth buffer
}

void RenderPlayerVectorMap::RenderTransparent()
{
	if (Proxy == NULL)
		return;
//	Proxy->Lock();
	
	bool showAll = RenderLayers.size() == 0; // If no layers specified, draw all layers (better than not showing up)

	glColor4f(Colour.r, Colour.g, Colour.b, Colour.a);

	if (GlList < 0)
	{
		GlList = glGenLists(1);
		glNewList(GlList, GL_COMPILE);
		glColor4f(Colour.r, Colour.g, Colour.b, Colour.a);

		for (int ii = 0; ii < Proxy->GetLayerCount(); ++ii)
		{
			if (showAll || RenderLayers.find(ii) != RenderLayers.end()) // If displayed
			{
				Proxy->GetLayerData(ii);
				for (int feature_index = 0; feature_index < Proxy->GetFeatureCount(ii); ++ feature_index)
				{
#if PLAYER_VER == 2
					GEOSGeom g = Proxy->GetFeatureData(ii, feature_index);
#else
					GEOSGeom g = GEOSGeomFromWKB_buf(Proxy->GetFeatureData(ii, feature_index),Proxy->GetFeatureDataCount(ii, feature_index));
#endif
					if (g)
					{
						RenderGeom(g);
#if PLAYER_VER == 3
						GEOSGeom_destroy(g);
#endif
						g = NULL; 
					}
				}
			}
		}
		glEndList();
	}
	glCallList(GlList);

//	Proxy->Unlock();
}

void RenderPlayerVectorMap::RenderGeom(const GEOSGeometry * geom)
{
	  const GEOSCoordSequence *seq;
	  unsigned numcoords;
	  double x,y,x2,y2;
      double cross_size = 0.2;
	  switch (GEOSGeomTypeId(geom))
	  {
	    case GEOS_POINT:
	      seq = GEOSGeom_getCoordSeq(geom);
	      GEOSCoordSeq_getX(seq, 0, &x);
	      GEOSCoordSeq_getY(seq, 0, &y);
	      glBegin(GL_QUADS);
	      glVertex3f(x-cross_size,y-cross_size,0);
	      glVertex3f(x+cross_size,y-cross_size,0);
	      glVertex3f(x+cross_size,y+cross_size,0);
	      glVertex3f(x-cross_size,y+cross_size,0);
	      glEnd();
	      break;
	    case GEOS_LINESTRING:
	    case GEOS_LINEARRING:
	      printf("Got a line string\n");
	      seq = GEOSGeom_getCoordSeq(geom);
	      if(GEOSCoordSeq_getSize(seq, &numcoords))
	      {
	        GEOSCoordSeq_getX(seq, 0, &x2);
	        GEOSCoordSeq_getY(seq, 0, &y2);
	        printf("first point: %f %f\n", x2,y2);
	        if (numcoords < 2)
	        {
	  	      glBegin(GL_QUADS);
	  	      glVertex3f(x2-cross_size,y2-cross_size,0);
	  	      glVertex3f(x2+cross_size,y2-cross_size,0);
	  	      glVertex3f(x2+cross_size,y2+cross_size,0);
	  	      glVertex3f(x2-cross_size,y2+cross_size,0);
	  	      glEnd();
	        }
	        else
	        {
	          // Draw as walls
	          glBegin(GL_QUADS);
	          for (unsigned ii = 1; ii < numcoords; ++ii)
	          {
	            x = x2;
	            y = y2;
	            GEOSCoordSeq_getX(seq, ii, &x2);
	            GEOSCoordSeq_getY(seq, ii, &y2);
	  	      	glVertex3f(x,y,0);
	  	      	glVertex3f(x2,y2,0);
	  	      	glVertex3f(x2,y2,Height);
	  	      	glVertex3f(x,y,Height);
	          }
	          glEnd();
	          // Draw around base in case viewed from above
	          glLineWidth(2);
	          glBegin(GL_LINE_STRIP);
	          for (unsigned ii = 0; ii < numcoords; ++ii)
			  {
				GEOSCoordSeq_getX(seq, ii, &x);
				GEOSCoordSeq_getY(seq, ii, &y);
				glVertex3f(x,y,0);
			  }
	          glEnd();
	          glLineWidth(1);
	        }
	      }
	      break;

	    case GEOS_POLYGON:
	    	RenderGeom(GEOSGetExteriorRing(geom));
	      for (int ii = 0; ii < GEOSGetNumInteriorRings(geom); ++ii)
	      {
	    	  RenderGeom(GEOSGetInteriorRingN(geom,ii));
	      }
	      break;

	    case GEOS_MULTIPOINT:
	    case GEOS_MULTILINESTRING:
	    case GEOS_MULTIPOLYGON:
	    case GEOS_GEOMETRYCOLLECTION:
	      for (int ii = 0; ii < GEOSGetNumGeometries(geom); ++ii)
	      {
	    	  RenderGeom(GEOSGetGeometryN(geom,ii));
	      }
	      break;

	    default:
	    	dbg_print(ARDBG_WARN,"unknown feature type: %d", GEOSGeomTypeId(geom));
	  }
}
