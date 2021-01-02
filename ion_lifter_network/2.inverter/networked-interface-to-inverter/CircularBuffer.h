/*! ***************************************************************************
*   \file        Circular Buffer.h
*   \brief      Circular buffer (leave one slot open)
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

#ifndef CIRCULARBUFFER_H_
#define CIRCULARBUFFER_H_


#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
// standard libraries first

#include <stdint.h>
#include <stdbool.h>


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
// function declarations

/*****************************************************************************/
// variables

/*****************************************************************************/
// functions


#define TOTAL_CIRCULAR_BUFFERS	1
#define CIRCULAR_BUFFER_SIZE 	50//255

uint8_t	RemoveFromBuffer(uint8_t whichBuffer, bool *Error);
void 	AddToBuffer(uint8_t whichBuffer,uint8_t input, bool *Error);
bool 	IsEmpty(uint8_t whichBuffer, bool *Error);
bool 	IsFull(uint8_t whichBuffer,bool *Error);








#ifdef __cplusplus
}
#endif




#endif /* CIRCULARBUFFER_H_ end of file */

// close the Doxygen group
/**
\}
*/
