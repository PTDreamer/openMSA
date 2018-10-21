/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2018
 * @brief      ad9850.cpp file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   ad9850
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
#include "ad9850.h"
#include <QDebug>

ad9850::ad9850(deviceParser::MSAdevice device, QObject *parent) : genericDDS(parent)
{
	registerSize = 40;
	parser = new deviceParser(device, this);
	//(Serial load Power-Down Sequence)WCLK up,WCLK up and FQUD up,WCLK up and FQUD down,WCLK down
	//(Serial load enable Sequence)WCLK up, WCLK down, FQUD up, FQUD down
	//(flush and command DDS1)D7,WCLK up,WCLK down,(repeat39more),FQUD up,FQUD down
	QList<int> bits;
	//bit numbering follows ADF4118 datasheet, LMX2326 is inverted
	for(int x = 0; x < 32; ++x)
		bits << x;
	fillField(FIELD_FREQUENCY, bits, &deviceRegister);
	bits.clear();
	bits << 32 << 33;
	fillField(FIELD_CONTROL, bits, &deviceRegister);
	bits.clear();
	bits << 34;
	fillField(FIELD_POWER, bits, &deviceRegister);
	bits.clear();
	bits << 35 << 36 << 37 << 38 << 39;
	fillField(FIELD_PHASE, bits, &deviceRegister);
	devicePin *pin = new devicePin;
	pin->dataArray = QHash<quint32, QBitArray *>();
	pin->name = "Data";
	pin->type = hardwareDevice::INPUT;
	devicePins.insert(PIN_DATA, pin);
	pin = new devicePin;
	pin->dataArray = QHash<quint32, QBitArray *>();
	pin->name = "Frequency Update";
	pin->type = hardwareDevice::INPUT;
	devicePins.insert(PIN_FQUD, pin);
	pin = new devicePin;
	pin->dataArray = QHash<quint32, QBitArray *>();
	pin->name = "Clock";
	pin->type = hardwareDevice::CLK;
	devicePins.insert(PIN_WCLK, pin);
}

void ad9850::processNewScan(hardwareDevice::scanStruct scan)
{
	foreach (int step, scan.steps.keys()) {
		quint32 base = parser->parseDDSOutput(scan.configuration, step);
		setFieldRegister(FIELD_FREQUENCY, base);
		registerToBuffer(&deviceRegister, PIN_DATA, step);
	}
	foreach(int x, devicePins.value(PIN_DATA)->dataArray.keys()) {
		QBitArray *arr = devicePins.value(PIN_DATA)->dataArray.value(x);
		qDebug() << getDDSOutput(x) << *arr;
	}
}

void ad9850::init()
{
	if(!devicePins.value(PIN_DATA)->dataArray.contains(INIT_STEP))
		devicePins.value(PIN_DATA)->dataArray.insert(INIT_STEP, new QBitArray(registerSize + 2));

	if(!devicePins.value(PIN_WCLK)->dataArray.contains(INIT_STEP))
		devicePins.value(PIN_WCLK)->dataArray.insert(INIT_STEP, new QBitArray(registerSize + 2));

	if(!devicePins.value(PIN_FQUD)->dataArray.contains(INIT_STEP))
		devicePins.value(PIN_FQUD)->dataArray.insert(INIT_STEP, new QBitArray(registerSize + 2));

	devicePins.value(PIN_DATA)->dataArray.value(INIT_STEP)->fill(false);
	devicePins.value(PIN_FQUD)->dataArray.value(INIT_STEP)->fill(false);
	devicePins.value(PIN_WCLK)->dataArray.value(INIT_STEP)->fill(true);

	devicePins.value(PIN_WCLK)->dataArray.value(INIT_STEP)->setBit(registerSize, false);
	devicePins.value(PIN_FQUD)->dataArray.value(INIT_STEP)->setBit(registerSize, true);
	devicePins.value(PIN_WCLK)->dataArray.value(INIT_STEP)->setBit(registerSize + 1, false);

	qDebug() << "LLLLL";
	qDebug() << *devicePins.value(PIN_WCLK)->dataArray.value(INIT_STEP);
	qDebug() << *devicePins.value(PIN_FQUD)->dataArray.value(INIT_STEP);
	qDebug() << *devicePins.value(PIN_DATA)->dataArray.value(INIT_STEP);

}

void ad9850::reinit()
{

}

bool ad9850::checkSettings()
{
	return true;
}

void ad9850::registerToBuffer(quint64 *reg, int pin, quint32 step)
{
	if(!devicePins.value(pin)->dataArray.contains(step))
		devicePins.value(pin)->dataArray.insert(step, new QBitArray());
	QBitArray * arr = devicePins.value(pin)->dataArray.value(step);
	arr->clear();
	arr->resize(registerSize);
	quint64 r = *reg;
	for(int x = 0; x < registerSize; ++ x) {
		(*arr)[x] = r & (quint64)1;
		r = r >> 1;
	}
	//	qDebug() << *arr;
}

hardwareDevice::clockType ad9850::getClk_type() const
{
	return hardwareDevice::CLOCK_RISING_EDGE;
}

