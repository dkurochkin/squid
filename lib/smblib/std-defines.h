#ifndef _SMBLIB_STD_DEFINES_H
#define _SMBLIB_STD_DEFINES_H

#include "config.h"

/* RFCNB Standard includes ... */
/*

   SMBlib Standard Includes

   Copyright (C) 1996, Richard Sharpe
*/
/* One day we will conditionalize these on OS types ... */

/*
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#define BOOL int

#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <strings.h>

#define TRUE 1
#define FALSE 0

#endif /* _SMBLIB_STD_DEFINES_H */
