/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2018
 * @brief      genericadc.cpp file
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
#include "genericadc.h"

genericADC::genericADC(hardwareDevice::MSAdevice device, hardwareDevice::HWdevice type, QObject *parent): hardwareDevice(parent), adc_type(type)
{
	registerSize = 40;
	parser = new deviceParser(device, this);
	devicePin *pin = new devicePin;
	pin->name = "Data";
	pin->IOtype = hardwareDevice::MAIN_DATA;
	devicePins.insert(PIN_DATA, pin);
	pin = new devicePin;
	pin->name = "Convert";
	pin->IOtype = hardwareDevice::GEN_INPUT;
	devicePins.insert(PIN_CONVERT, pin);
	pin = new devicePin;
	pin->name = "Clock";
	pin->IOtype = hardwareDevice::CLK;
	devicePins.insert(PIN_CLK, pin);
	pin = new devicePin;
	pin->IOtype = hardwareDevice::VIRTUAL_CLK;
	devicePins.insert(PIN_VIRTUAL_CLOCK, pin);
}

bool genericADC::init()
{
	return true;
}

hardwareDevice::clockType genericADC::getClk_type() const
{
	return hardwareDevice::CLOCK_RISING_EDGE;
}
