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
#include "../genericadc.h"
#include "../msa.h"

interface::interface(QObject *parent):QThread(parent)
{
	hardwareDevice::currentScan.configuration.LO2 = 1024;
	hardwareDevice::currentScan.configuration.appxdds1 = 10.7;
	hardwareDevice::currentScan.configuration.baseFrequency = 0;
	hardwareDevice::currentScan.configuration.PLL1phasefreq = 0.974;
	hardwareDevice::currentScan.configuration.finalFilterFrequency = 10.7;
	hardwareDevice::currentScan.configuration.masterOscilatorFrequency = 64;
}

interface::~interface()
{
	foreach (hardwareDevice *dev, msa::getInstance().currentHardwareDevices) {
		foreach (hardwareDevice::devicePin *pin, dev->devicePins.values()) {
			foreach (hardwareDevice::pin_data data, pin->data.values()) {
				if(data.dataArray)
					delete data.dataArray;
				if(data.dataMask)
					delete data.dataMask;
			}
		}
		qDeleteAll(dev->devicePins);
	}
	qDeleteAll(msa::getInstance().currentHardwareDevices);
}

void interface::initScan(bool inverted, double start, double end, double step, int band)
{
	int steps = (end - start) / step;
	int thisBand = 0;
	int bandSelect = 0;
	for(int x = 0; x < steps; ++x) {
		hardwareDevice::scanStep s;
		s.realFrequency = start + (x * step);
		if(band < 0) {
			if(s.realFrequency < 1000)
				thisBand = 1;
			else if(s.realFrequency < 2000)
				thisBand = 2;
			else
				thisBand = 3;
		}
		s.translatedFrequency = s.realFrequency;
		double IF1;
		if(band < 0)
			bandSelect = thisBand;
		else
			bandSelect = band;
		switch (bandSelect) {
		case 2:
			s.translatedFrequency = s.translatedFrequency - hardwareDevice::currentScan.configuration.LO2;
			break;
		case 3:
			IF1 = hardwareDevice::currentScan.configuration.LO2 - hardwareDevice::currentScan.configuration.finalFilterFrequency;
			s.translatedFrequency = s.translatedFrequency - 2*IF1;
			break;
		default:
			break;
		}
		s.band = bandSelect;
		hardwareDevice::currentScan.steps.insert(x, s);
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
	hardwareDevice::currentScan.configuration = configuration;
}

void interface::hardwareInit()
{
	foreach (hardwareDevice *dev, msa::getInstance().currentHardwareDevices.values()) {
		dev->init();
	}
}

//QHash<hardwareDevice::MSAdevice, hardwareDevice *> interface::getCurrentHardwareDevices() const
//{
//	return currentHardwareDevices;
//}
