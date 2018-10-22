/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2018
 * @brief      interface.cpp file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   interface
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

#include <QDebug>
#include "interface.h"
#include "../lmx2326.h"
#include "../ad9850.h"

interface::interface(QObject *parent):QObject(parent)
{
	currentScan.configuration.LO2 = 1024;
	currentScan.configuration.appxdds1 = 10.7;
	currentScan.configuration.baseFrequency = 0;
	currentScan.configuration.PLL1phasefreq = 0.974;
	currentScan.configuration.finalFrequency = 10.7;
	currentScan.configuration.masterOscilatorFrequency = 64;
}

void interface::initScan(bool inverted, double start, double end, double step)
{
	int steps = (end - start) / step;
	for(int x = 0; x < steps; ++x) {
		hardwareDevice::scanStep s;
		s.frequency = start + (x * step);
		currentScan.steps.insert(x, s);
	}
	isInverted = inverted;
	hardwareDevice::scanStruct scan = hardwareDevice::currentScan;
	numberOfSteps = scan.steps.keys().size();
	if(inverted)
		currentStep = numberOfSteps;
	else
		currentStep = 0;
}

void interface::setScanConfiguration(hardwareDevice::scanConfig configuration)
{
	currentScan.configuration = configuration;
}

void interface::hardwareInit(QHash<hardwareDevice::MSAdevice, hardwareDevice::HWdevice> devices)
{
	qDeleteAll(currentHardwareDevices);
	currentHardwareDevices.clear();
	lmx2326 *x;
	foreach (hardwareDevice::MSAdevice dev, devices.keys()) {
		switch (devices.value(dev)) {
		case hardwareDevice::LMX2326:
			currentHardwareDevices.insert(dev, new lmx2326(dev, this));
			break;
		case hardwareDevice::AD9850:
			currentHardwareDevices.insert(dev, new ad9850(dev, this));
		default:
			break;
		}
		currentHardwareDevices.value(dev)->init();
	}
}

QHash<hardwareDevice::MSAdevice, hardwareDevice *> interface::getCurrentHardwareDevices() const
{
	return currentHardwareDevices;
}
