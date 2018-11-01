/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2018
 * @brief      msa.cpp file
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
#include "msa.h"
#include "genericadc.h"
#include "ad9850.h"
#include "lmx2326.h"

void msa::hardwareInit(QHash<hardwareDevice::MSAdevice, hardwareDevice::HWdevice> devices, interface *usedInterface)
{
	currentInterface = usedInterface;
	qDeleteAll(currentHardwareDevices);
	currentHardwareDevices.clear();
	foreach (hardwareDevice::MSAdevice dev, devices.keys()) {
		switch (devices.value(dev)) {
		case hardwareDevice::LMX2326:
			currentHardwareDevices.insert(dev, new lmx2326(dev, usedInterface));
			break;
		case hardwareDevice::AD9850:
			currentHardwareDevices.insert(dev, new ad9850(dev, usedInterface));
			break;
		case hardwareDevice::AD7685:
		case hardwareDevice::LT1865:
			currentHardwareDevices.insert(dev, new genericADC(dev, devices.value(dev), usedInterface));
		default:
			break;
		}
	}
	currentInterface->hardwareInit();
}
