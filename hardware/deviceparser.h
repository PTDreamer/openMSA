/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2018
 * @brief      deviceparser.h file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   deviceParser
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
#ifndef DEVICEPARSER_H
#define DEVICEPARSER_H

#include <QObject>
#include "hardwaredevice.h"
#include <QHash>

class deviceParser:public QObject
{
	Q_OBJECT
public:
	deviceParser(hardwareDevice::MSAdevice dev, hardwareDevice *parent);
	double parsePLLRCounter(hardwareDevice::scanConfig config);
	double parsePLLNCounter(hardwareDevice::scanConfig configuration, hardwareDevice::scanStep &step, int stepNumber);
	quint32 parseDDSOutput(hardwareDevice::scanConfig configuration, int stepNumber, bool &error);
	static const QHash<hardwareDevice::MSAdevice, hardwareDevice *> getDeviceList();
	hardwareDevice::HWdevice getDeviceType() {return hwdev;}
	~deviceParser();
private:
	static QHash<hardwareDevice::MSAdevice, hardwareDevice *> deviceList;
	hardwareDevice::MSAdevice msadev;
	hardwareDevice::HWdevice hwdev;
	hardwareDevice *device;
};

#endif // DEVICEPARSER_H
