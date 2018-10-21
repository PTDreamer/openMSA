/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros 2018
 * @brief      global_defs file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   %{Cpp:License:ClassName}
 * @{
 *
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>
 */
#ifndef GLOBAL_DEFS_H
#define GLOBAL_DEFS_H

#ifdef _MSC_VER
#define PACK(__Declaration__) __pragma(pack(push, 1)) __Declaration__ __pragma(pack(pop))
#else
#define PACK(__Declaration__) __Declaration__ __attribute__((__packed__))
#endif

#endif // GLOBAL_DEFS_H
