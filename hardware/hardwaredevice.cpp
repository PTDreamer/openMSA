/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2018
 * @brief      hardwaredevice.cpp file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   hardwareDevice
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
#include "hardwaredevice.h"
#include "deviceparser.h"
#include <QDebug>

void hardwareDevice::setNewScan(msa::scanStruct scan) {
	msa::getInstance().currentScan = scan;
}

hardwareDevice::hardwareDevice(QObject *parent):QObject(parent), registerSize(0)
{
}

const QHash<int, hardwareDevice::devicePin*>  hardwareDevice::getDevicePins()
{
	return devicePins;
}

hardwareDevice::~hardwareDevice()
{

}

hardwareDevice::HWdevice hardwareDevice::getHardwareType()
{
	return parser->getDeviceType();
}

bool hardwareDevice::setFieldRegister(int field, quint32 value)
{
	field_struct st = fieldlist.value(field);
	value = value << st.offset;
	*st.reg = *st.reg & ~st.mask;
	*st.reg|= value;
	return checkSettings();
}

int hardwareDevice::getFieldRegister(int field)
{
	quint64 *r = fieldsPerRegister.key(field);
	quint64 rval = *r;
	field_struct st = fieldlist.value(field);
	rval &= st.mask;
	return rval >> st.offset;
}

QString hardwareDevice::convertToStr(quint64 *reg)
{
	quint64 value = *reg;
	QString ret = "000000000000000000000";
	quint64 temp = 0;
	int i = 0;
	for(i = 0; i < 20; ++i) {
		temp = (quint64)(value / 2);
		quint64 temp2 = value - 2*temp;
		ret.replace(i,1,QString::number(temp2));
		value = temp;
	};

	ret.replace(i, 1, QString::number(temp));
	QList<int> index;
	foreach (int t, fieldlist.keys()) {
		field_struct st = fieldlist.value(t);
		if(st.reg == reg) {
			index << st.offset + st.bits;
		}
	}
	std::sort(index.begin(), index.end());
	int c = 0;
	foreach (int i, index) {
		if(i < 21) {
			ret.insert(i + c, "---");
			c+= 3;
		}
	}
	return ret;
}

void hardwareDevice::fillField(int field, QList<int> bits, quint64 *reg)
{
	fieldsPerRegister.insert(reg, field);
	std::sort(bits.begin(), bits.end());
	field_struct st;
	st.mask = 0;
	foreach (int bit, bits) {
		st.mask |= (1 << bit);
	}
	st.bits = bits.count();
	st.offset = bits.first();
	st.reg = reg;
	fieldlist.insert(field, st);
}

hardwareDevice::pin_data hardwareDevice::createPinData(int size) {
	pin_data d;
	d.dataArray = new QBitArray(size);
	d.dataMask = new QBitArray(size);
	return d;
}

void hardwareDevice::resizePinData(pin_data *pin, int size) {
	pin->dataArray->resize(size);
	pin->dataMask->resize(size);
}

bool hardwareDevice::convertStringToBitArray(QString string, QBitArray *array)
{
	int val = -1;
	string = string.simplified().replace(" ","");
	array->resize(string.length());
	for(int x = 0; x < string.size(); ++x) {
		if(string.mid(x, 1) == "0") {
			val = 0;
		}
		else if(string.mid(x, 1) == "1")
			val = 1;
		else
			return false;
		(*array)[x] = val;
	}
	return true;
}

void hardwareDevice::registerToBuffer(quint64 *reg, int pin, quint32 step)
{
	if(!devicePins.value(pin)->data.contains(step))
		devicePins.value(pin)->data.insert(step, createPinData(registerSize));
	QBitArray * arrayData = devicePins.value(pin)->data.value(step).dataArray;
	QBitArray * arrayMask = devicePins.value(pin)->data.value(step).dataMask;
	//arrayData->resize(registerSize + 1);
	//arrayMask->resize(registerSize + 1);
	arrayMask->fill(1);
	quint64 r = *reg;
	for(int x = 0; x < registerSize; ++ x) {
		(*arrayData)[registerSize - 1 - x] = r & (quint64)1;
		r = r >> 1;
	}
	//arrayMask->setBit(registerSize);
}

genericPLL::genericPLL(QObject *parent):hardwareDevice(parent)
{
}

genericDDS::genericDDS(QObject *parent):hardwareDevice(parent)
{
}
