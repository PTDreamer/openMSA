/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2018
 * @brief      msa.h file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   msa
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
#ifndef MSA_H
#define MSA_H

#include <QHash>
#include "hardwaredevice.h"
#include "controllers/interface.h"
class msa
{
	public:
		static msa& getInstance()
		{
			static msa    instance;
			return instance;
		}
		QHash<hardwareDevice::MSAdevice, hardwareDevice *> currentHardwareDevices;
	private:
		msa() {}
		interface *currentInterface;

	public:
		msa(msa const&)               = delete;
		void operator=(msa const&)  = delete;
		void hardwareInit(QHash<hardwareDevice::MSAdevice, hardwareDevice::HWdevice> devices, interface *usedInterface);
};

#endif // MSA_H
