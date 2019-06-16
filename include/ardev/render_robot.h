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
 
#ifndef RENDER_ROBOT_H
#define RENDER_ROBOT_H
 
#include <ardev/ardev.h>

/** Render base of b21r robot, helps to obscure data that is 'behind' the robot
 */
class RenderB21r : public RenderObject
{
	public:
		/// Constructor with specified colour components and size
		RenderB21r(bool Visible = false);
		
		void Render(); ///< Rend the robot in GL
		void RenderBase(); ///< Rend the robot in GL (depth info only)
	
	private:
		void Draw(); ///< Actually draw the robot
		bool Visible; ///< Should we actually draw the robot, or only depth info
		
};

#endif
