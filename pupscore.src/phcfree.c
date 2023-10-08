/*---------------------------------------------------------------------------
    Copyright (C) 1991, 1993, 1994 Free Software Foundation, Inc.
    This file is part of the GNU C Library.

    The GNU C Library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.

    The GNU C Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with the GNU C Library; see the file COPYING.LIB.  If
    not, write to the Free Software Foundation, Inc., 675 Mass Ave,
    Cambridge, MA 02139, USA.

    Shared heap code by Mark O'Neill (mao@tumblingdice.co.uk)
    (C) 1998-2022 M.A. O'Neill, Tumbling Dice
--------------------------------------------------------------------------*/

#ifndef	_PHMALLOC_INTERNAL
#define _PHMALLOC_INTERNAL
#include <phmalloc.h>
#endif /* _PHMALLOC_INTERNAL */

#include <xtypes.h>

#ifdef _LIBC
#include <ansidecl.h>
#include <gnu-stabs.h>
#undef	cfree

function_alias(cfree, free, void, (ptr), DEFUN(cfree, (ptr), PTR ptr))

#else

_PUBLIC void phcfree (int hdes,  __ptr_t ptr)
{   phfree (hdes, ptr);
}

#endif /* LIBC */
