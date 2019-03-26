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
#include "controllers/interface.h"
#include "hardwaredevice.h"
#include <QDebug>

bool msa::getIsInverted() const
{
	return isInverted;
}

int msa::getResolution_filter_bank() const
{
	return resolution_filter_bank;
}

void msa::setResolution_filter_bank(int value)
{
	resolution_filter_bank = value;
}

void msa::hardwareInit(QHash<MSAdevice, int> devices, interface *usedInterface)
{
	currentInterface = usedInterface;
	qDeleteAll(currentHardwareDevices);
	currentHardwareDevices.clear();
	foreach (msa::MSAdevice dev, devices.keys()) {
		switch (devices.value(dev)) {
		case hardwareDevice::LMX2326:
			currentHardwareDevices.insert(dev, new lmx2326(dev, usedInterface));
			break;
		case hardwareDevice::AD9850:
			currentHardwareDevices.insert(dev, new ad9850(dev, usedInterface));
			break;
		case hardwareDevice::AD7685:
		case hardwareDevice::LT1865:
			currentHardwareDevices.insert(dev, new genericADC(dev, (hardwareDevice::HWdevice)devices.value(dev), usedInterface));
		default:
			break;
		}
	}
	currentInterface->hardwareInit();
}

void msa::initScan(bool inverted, double start, double end, int steps, int band)
{
	double step = (end - start) / (double)steps;
	int thisBand = 0;
	int bandSelect = 0;
	for(int x = 0; x < steps; ++x) {
		msa::scanStep s;
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
			s.translatedFrequency = s.translatedFrequency - msa::getInstance().currentScan.configuration.LO2;
			break;
		case 3:
			IF1 = msa::getInstance().currentScan.configuration.LO2 - msa::getInstance().currentScan.configuration.finalFilterFrequency;
			s.translatedFrequency = s.translatedFrequency - 2*IF1;
			break;
		default:
			break;
		}
		s.band = bandSelect;
		//qDebug() << "step:" << x << "real frequency:" << s.realFrequency << "translated frequency:" << s.translatedFrequency;
		msa::getInstance().currentScan.steps.insert(x, s);
	}
	isInverted = inverted;
	currentInterface->initScan();
}

void msa::initScan(bool inverted, double start, double end, double step_freq, int band)
{
	int steps = (end - start) / step_freq;
	initScan(inverted, start, end, steps, band);
}

void msa::setScanConfiguration(msa::scanConfig configuration)
{
	msa::getInstance().currentScan.configuration = configuration;
}
