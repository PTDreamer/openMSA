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
#include <QMessageBox>

interface::interface(QObject *parent):QThread(parent)
{
	//TODO delete this?
	msa::getInstance().currentScan.configuration.LO2 = 1024;
	msa::getInstance().currentScan.configuration.appxdds1 = 10.7;
	msa::getInstance().currentScan.configuration.baseFrequency = 0;
	msa::getInstance().currentScan.configuration.PLL1phasefreq = 0.974;
	msa::getInstance().currentScan.configuration.pathCalibration.centerFreq_MHZ = 10.7;
	msa::getInstance().currentScan.configuration.masterOscilatorFrequency = 64;
	currentStatus = status_halted;
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
	msa::getInstance().currentHardwareDevices.clear();
}

void interface::commandNextStep()
{
	if(currentStatus != status_paused)
		return;
	on_commandNextStep();
}

void interface::commandPreviousStep()
{
	if(currentStatus != status_paused)
		return;
	on_commandPreviousStep();
}

void interface::pauseScan()
{
	currentStatus = status_paused;
	on_pausescan();
}

void interface::resumeScan()
{
	currentStatus = status_scanning;
	on_resumescan();
}

void interface::autoScan()
{
	currentStatus = status_scanning;
	on_autoscan();
}

void interface::cancelScan()
{
	currentStatus = status_halted;
	on_cancelscan();
}

void interface::setStatus(interface::status stat)
{
	switch (stat) {
	case status_halted:
		cancelScan();
		break;
	case status_paused:
		pauseScan();
		break;
	case status_scanning:
		autoScan();
		break;
	}
	currentStatus = stat;
}

void interface::setWriteReadDelay_us(unsigned long value)
{
	readDelay_us = value;
	on_setWriteReadDelay_us(value);
}

bool interface::initScan()
{
	msa::scanStruct scan = msa::getInstance().currentScan;
	numberOfSteps = scan.steps->keys().size();
	if(msa::getInstance().getIsInverted())
		currentStep = numberOfSteps;
	else
		currentStep = 0;
	return true;
}

void interface::hardwareInit()
{
	foreach (hardwareDevice *dev, msa::getInstance().currentHardwareDevices.values()) {
		dev->init();
	}
}

void interface::errorOcurred(msa::MSAdevice dev , QString text, bool critical, bool sendToGUI)
{
	Q_UNUSED(dev)
	emit errorTriggered(text, critical, sendToGUI);
}

int interface::getDebugLevel() const
{
	return debugLevel;
}

void interface::setDebugLevel(int value)
{
	debugLevel = value;
}
