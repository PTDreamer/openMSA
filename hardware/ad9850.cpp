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

ad9850::ad9850(hardwareDevice::MSAdevice device, QObject *parent) : genericDDS(parent)
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
	pin->name = "Data";
	pin->IOtype = hardwareDevice::MAIN_DATA;
	devicePins.insert(PIN_DATA, pin);
	pin = new devicePin;
	pin->name = "Frequency Update";
	pin->IOtype = hardwareDevice::GEN_INPUT;
	devicePins.insert(PIN_FQUD, pin);
	pin = new devicePin;
	pin->name = "Clock";
	pin->IOtype = hardwareDevice::CLK;
	devicePins.insert(PIN_WCLK, pin);
	pin = new devicePin;
	pin->IOtype = hardwareDevice::VIRTUAL_CLK;
	devicePins.insert(PIN_VIRTUAL_CLOCK, pin);
}

void ad9850::processNewScan()
{
	bool error; //TODO CHECK ERRORS
	foreach (int step, currentScan.steps.keys()) {
		quint32 base = parser->parseDDSOutput(currentScan.configuration, step, error);
		if(!error) {
			setFieldRegister(FIELD_FREQUENCY, base);
			registerToBuffer(&deviceRegister, PIN_DATA, step);
		}
	}
//	foreach(int x, devicePins.value(PIN_DATA)->data.keys()) {
//		QBitArray *arr = devicePins.value(PIN_DATA)->data.value(x).dataArray;
//		//qDebug() << "ad9850" << getDDSOutput(x) << *arr;
//	}
}

bool ad9850::init()
{
	foreach (int key, devicePins.keys()) {
		devicePins.value(key)->data.insert(HW_INIT_STEP, createPinData(0));
	}
	QString clkd = "111010 00 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 00";//1
	QString clkm = "100111 00 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 00";
	QString fqud = "010000 10 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 10";//2
	QString fqum = "011000 11 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 11";
	QString datd = "000000 00 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 00";
	QString datm = "000000 00 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 00";
	QString vcld = "000000 00 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 00";
	QString vclm = "000000 00 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 10";

	convertStringToBitArray(clkd, devicePins.value(PIN_WCLK)->data.value(HW_INIT_STEP).dataArray);
	convertStringToBitArray(clkm, devicePins.value(PIN_WCLK)->data.value(HW_INIT_STEP).dataMask);
	convertStringToBitArray(fqud, devicePins.value(PIN_FQUD)->data.value(HW_INIT_STEP).dataArray);
	convertStringToBitArray(fqum, devicePins.value(PIN_FQUD)->data.value(HW_INIT_STEP).dataMask);
	convertStringToBitArray(datd, devicePins.value(PIN_DATA)->data.value(HW_INIT_STEP).dataArray);
	convertStringToBitArray(datm, devicePins.value(PIN_DATA)->data.value(HW_INIT_STEP).dataMask);
	convertStringToBitArray(vcld, devicePins.value(PIN_VIRTUAL_CLOCK)->data.value(HW_INIT_STEP).dataArray);
	convertStringToBitArray(vclm, devicePins.value(PIN_VIRTUAL_CLOCK)->data.value(HW_INIT_STEP).dataMask);
	initIndexes.clear();
	initIndexes.append(HW_INIT_STEP);
	return true;
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
	if(!devicePins.value(pin)->data.contains(step))
		devicePins.value(pin)->data.insert(step, createPinData(registerSize));
	QBitArray * arrayData = devicePins.value(pin)->data.value(step).dataArray;
	QBitArray * arrayMask = devicePins.value(pin)->data.value(step).dataMask;
	arrayData->clear();
	arrayData->resize(registerSize);
	arrayMask->clear();
	arrayMask->resize(registerSize);
	quint64 r = *reg;
	for(int x = 0; x < registerSize; ++ x) {
		(*arrayMask)[x] = 1;
		(*arrayData)[x] = r & (quint64)1;
		r = r >> 1;
	}
}

hardwareDevice::clockType ad9850::getClk_type() const
{
	return hardwareDevice::CLOCK_RISING_EDGE;
}

