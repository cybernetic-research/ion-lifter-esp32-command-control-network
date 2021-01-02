/*! ***************************************************************************
*   \file        CircularBuffer.c
*   \brief      an implementation of a circular buffer using leave one slot open
*
*   \copyright   Copyright (C) :  <creation date 2016-12-02>
*
*    This program is free software: you can redistribute it and/or modify
*    it under the terms of the GNU General Public License as published by
*    the Free Software Foundation, either version 3 of the License, or
*    (at your option) any later version.
*
*    This program is distributed in the hope that it will be useful,
*    but WITHOUT ANY WARRANTY; without even the implied warranty of
*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*    GNU General Public License for more details.
*
*    You should have received a copy of the GNU General Public License
*    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*   \addtogroup  AddGroupsAsRequiredForTheProject
*   \{
******************************************************************************/

/*****************************************************************************/
// standard libraries

#include <stdbool.h>
// user headers directly related to this component, ensures no dependency
#include "CircularBuffer.h"

// user headers from other components

/*****************************************************************************/
// enumerations

/*****************************************************************************/
// typedefs

/*****************************************************************************/
// structures

/*****************************************************************************/
// constants

/*****************************************************************************/
// macros

/*****************************************************************************/
// static function declarations

/*****************************************************************************/
// static variable declarations

/*****************************************************************************/
// functions


static int32_t  start[TOTAL_CIRCULAR_BUFFERS]		={0};  	/* index of oldest element              */
static int32_t  end[TOTAL_CIRCULAR_BUFFERS]			={0};    	/* index at which to write new element  */
static uint8_t	elems[TOTAL_CIRCULAR_BUFFERS][CIRCULAR_BUFFER_SIZE];


bool IsFull(uint8_t whichBuffer,bool *Error)
{
    bool bResult 		= 	false;
    int32_t endloc 		=	(end[whichBuffer] + 1) % CIRCULAR_BUFFER_SIZE;
    int32_t startloc	=	start[whichBuffer];

    *Error = false;
    if(whichBuffer<=TOTAL_CIRCULAR_BUFFERS)
    {
        if(endloc==startloc)
        {
            bResult		=	true;
        }
    }
    else
    {
        *Error 			= 	true;
    }
    return(bResult);
}





bool IsEmpty(uint8_t whichBuffer, bool *Error)
{
    bool bResult 	= 	false;
    *Error 			= 	false;
    if(whichBuffer<=TOTAL_CIRCULAR_BUFFERS)
    {
        bResult	 	= 	(end[whichBuffer] == start[whichBuffer]);
    }
    else
    {
        *Error 		=	true;
    }
    return(bResult);
}

/* Write an element, overwriting oldest element if buffer is full. App can
   choose to avoid the overwrite by checking cbIsFull(). */
void AddToBuffer(uint8_t whichBuffer,uint8_t input, bool *Error)
{
    *Error 						= 	false;
    if(whichBuffer<=TOTAL_CIRCULAR_BUFFERS)
    {
        elems[whichBuffer][end[whichBuffer]]	= input;
        end[whichBuffer] 		= (end[whichBuffer] + 1) % CIRCULAR_BUFFER_SIZE;
        if (end[whichBuffer] == start[whichBuffer])
        {
            start[whichBuffer] 	= (start[whichBuffer] + 1) % CIRCULAR_BUFFER_SIZE; /* full, overwrite */
        }
    }
    else
    {
        *Error					=	true;
    }
}




/* Read oldest element. App must ensure !cbIsEmpty() first. */
uint8_t RemoveFromBuffer(uint8_t whichBuffer, bool *Error)
{
    *Error 					= 	false;
    uint8_t	elem			=	0x00;

    if(whichBuffer<=TOTAL_CIRCULAR_BUFFERS)
    {
        elem 				= 	elems[whichBuffer][start[whichBuffer]];
        start[whichBuffer] 	= 	(start[whichBuffer] + 1) % CIRCULAR_BUFFER_SIZE;
    }
    else
    {
        *Error				=	true;
    }
    return (elem);
}






// close the Doxygen group
/**
\}
*/




/* end of file */
