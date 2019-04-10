/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2018
 * @brief      hardwaredevice.h file
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
#ifndef HARDWAREDEVICE_H
#define HARDWAREDEVICE_H

#include <QList>
#include <QBitArray>
#include <limits>
#include <QObject>
#include "msa.h"

#define HW_INIT_STEP std::numeric_limits<quint32>::max()
#define SCAN_INIT_STEP HW_INIT_STEP-1
class deviceParser;

class hardwareDevice:public QObject
{
	Q_OBJECT
public:
	hardwareDevice(QObject *parent);
	typedef enum {LMX2326, AD9850, AD7685, LT1865, NONE} HWdevice;
	typedef enum {MAIN_DATA, GEN_INPUT, GEN_OUTPUT, INPUT_OUTPUT, CLK, VIRTUAL_CLK}pinType;
	typedef enum {CLOCK_RISING_EDGE, CLOCK_FALLING_EDGE}clockType;
	typedef struct {
		QBitArray *dataArray;
		QBitArray *dataMask;
	} pin_data;
	typedef struct {
		QString name;
		pinType IOtype;
		void *hwconfig;
		QHash<quint32, pin_data> data;//dataarray containing the serialized bit values for each scan step
	} devicePin;
	virtual bool processNewScan()=0;
	virtual bool init()=0;
	virtual void reinit()=0;
	// gets the type of CLK this device needs, dedicated or system wide
	virtual clockType getClk_type() const=0;
	// gets the pins list of the device, containing the name (string), type (input, output or both) and dataarray
	virtual const QHash<int, devicePin *> getDevicePins();
	~hardwareDevice();
	QHash<int, devicePin*> devicePins;
	HWdevice getHardwareType();
	static void setNewScan(msa::scanStruct scan);
	QList<quint32> getInitIndexes(){return initIndexes;}
protected:
	typedef struct {
		quint64 mask;
		quint8 offset;
		quint8 bits;
		quint64 *reg;
	} field_struct;
	QList<quint32> initIndexes;
	deviceParser *parser;
	// contains a structure describing each field, where it is, how bit it is, etc...
	QHash<int, field_struct> fieldlist;
	// contains all fields for a given device register
	QMultiHash<quint64 *, int> fieldsPerRegister;
	bool setFieldRegister(int field, quint32 value);
	int getFieldRegister(int field);
	virtual bool checkSettings() = 0;

	QString convertToStr(quint64 *reg);
	// automagically fills fieldlist by passing the field mask and destination register
	void fillField(int field, QList<int> bits, quint64 *reg);
	void registerToBuffer(quint64 *reg, int pin, quint32 step);
	int registerSize;
	hardwareDevice::pin_data createPinData(int size);
	bool convertStringToBitArray(QString string, QBitArray *array);
	void resizePinData(pin_data *pin, int size);
protected slots:
};

class genericPLL: public hardwareDevice
{
	Q_OBJECT
public:
	genericPLL(QObject *parent);
	virtual double getPFD(quint32 step) {return pfd.value(step);}
	void setPFD(double value, quint32 step){pfd.insert(step, value);}
	virtual int getRCounter() = 0;
protected:
	QHash<quint32, double> pfd;
};

class genericDDS: public hardwareDevice
{
	Q_OBJECT
public:
	genericDDS(QObject *parent);
	double getDDSOutput(quint32 step) {return ddsout.value(step);}
	void setDDSOutput(double value, quint32 step) {ddsout.insert(step, value);}
protected:
	QHash<quint32, double> ddsout;
};

#endif // HARDWAREDEVICE_H
