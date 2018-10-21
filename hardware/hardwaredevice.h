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

#define INIT_STEP std::numeric_limits<quint32>::max()
class deviceParser;

class hardwareDevice:public QObject
{
	Q_OBJECT
public:
	typedef struct {
		double frequency;
		double LO1;
	} scanStep;
	typedef struct {
		double baseFrequency;
		double LO2; // from configuration
		double finalFrequency; //final filter frequency;
		double appxdds1; // center freq. of DDS1 xtal filter; exact value determined in calibration
		double PLL1phasefreq; // from configuration default=0.974
		double masterOscilatorFrequency;
	} scanConfig;

	typedef struct {
		scanConfig configuration;
		QHash<quint32, scanStep> steps;
	} scanStruct;

	hardwareDevice(QObject *parent);
	typedef enum {INPUT, OUTPUT, INPUT_OUTPUT, CLK}pinType;
	typedef enum {CLOCK_RISING_EDGE, CLOCK_FALLING_EDGE}clockType;
	typedef struct {
		QString name;
		pinType type;
		QHash<quint32, QBitArray *> dataArray;//dataarray containing the serialized bit values for each scan step
	} devicePin;
	virtual void processNewScan(scanStruct scan)=0;
	virtual void init()=0;
	virtual void reinit()=0;
	// gets the type of CLK this device needs, dedicated or system wide
	virtual clockType getClk_type() const=0;
	// gets the pins list of the device, containing the name (string), type (input, output or both) and dataarray
	virtual QList<devicePin*> getDevicePins();
	~hardwareDevice();
protected:
	typedef struct {
		quint64 mask;
		quint8 offset;
		quint8 bits;
		quint64 *reg;
	} field_struct;
	QHash<int, devicePin*> devicePins;
	deviceParser *parser;
	// contains a structure describing each field, where it is, how bit it is, etc...
	QHash<int, field_struct> fieldlist;
	// contains all fields for a given device register
	QMultiHash<quint64 *, int> fieldsPerRegister;
	bool setFieldRegister(int field, int value);
	int getFieldRegister(int field);
	virtual bool checkSettings() = 0;

	QString convertToStr(quint64 *reg);
	// automagically fills fieldlist by passing the field mask and destination register
	void fillField(int field, QList<int> bits, quint64 *reg);
	void registerToBuffer(quint64 *reg, int pin, quint32 step);
	int registerSize;
protected slots:
};

class genericPLL: public hardwareDevice
{
	Q_OBJECT
public:
	genericPLL(QObject *parent);
	virtual double getPFD(int step) {return pfd.value(step);}
	void setPFD(double value, int step){pfd.insert(step, value);}
	virtual int getRCounter() = 0;
protected:
	QHash<int, double> pfd;
};

class genericDDS: public hardwareDevice
{
	Q_OBJECT
public:
	genericDDS(QObject *parent);
	double getDDSOutput(int step) {return ddsout.value(step);}
	void setDDSOutput(double value, int step) {ddsout.insert(step, value);}
protected:
	QHash<int, double> ddsout;
};

#endif // HARDWAREDEVICE_H
