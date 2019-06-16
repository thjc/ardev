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
#ifndef ARIDE_REGISTRY_H
#define ARIDE_REGISTRY_H

#include <string.h>

#include <map>
using namespace std;

#include "cm_project.h"

class ObjectHandler;
typedef ObjectHandler * (create_objecthandler)(void);

class ObjectRegistryEntry
{
	public:
		ObjectRegistryEntry() {CreateHandler=NULL;};
		ObjectRegistryEntry(create_objecthandler * inCreateHandler,ArideSection inSection)
		{
			CreateHandler = inCreateHandler;
			Section = inSection;
		};
		~ObjectRegistryEntry() {;};
	
		ArideSection Section;
		create_objecthandler * CreateHandler;		
};

struct ltstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    return strcmp(s1, s2) < 0;
  }
};

typedef map<const char *, ObjectRegistryEntry,ltstr> ObjectHandlerRegistryMap;
extern map<const char *, ObjectRegistryEntry,ltstr> ObjectHandlerRegistry;

#endif
