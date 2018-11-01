/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2018
 * @brief      genericadc.h file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   genericADC
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
#ifndef GENERICADC_H
#define GENERICADC_H

#include "deviceparser.h"

class genericADC : public hardwareDevice
{
	Q_OBJECT
public:
	genericADC(msa::MSAdevice device, hardwareDevice::HWdevice type, QObject *parent = 0);
	void processNewScan(){}
	bool init();
	void reinit(){}
	bool checkSettings(){return true;}
	hardwareDevice::HWdevice getHardwareType() {return adc_type;}
	// gets the type of CLK this device needs, dedicated or system wide
	clockType getClk_type() const;
	typedef enum {PIN_DATA, PIN_CONVERT, PIN_CLK, PIN_VIRTUAL_CLOCK} pins;
private:
	hardwareDevice::HWdevice adc_type;
};

#endif // GENERICADC_H
