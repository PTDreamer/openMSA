/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2018
 * @brief      slimusb.h file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   slimusb
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
#ifndef SLIMUSB_H
#define SLIMUSB_H

#include <QObject>
#include <QThread>
#include <QVector>
#include "interface.h"
#include <QHash>
#include <QVector>
#include "../lmx2326.h"
#include "../ad9850.h"
#include "../genericadc.h"

#include "usbdevice.h"

class slimusb : public interface
{
	Q_OBJECT
public:
	slimusb(QObject *parent);
	bool init(int debugLevel);
	~slimusb();
	QList<usbdevice::usbDevice_t> getDevices();
	bool openDevice(int deviceNumber);
	void closeDevice();
	bool getAutoConnect() const;
	void setAutoConnect(bool value);
	bool getIsConnected() const;
	bool initScan();
	void hardwareInit();
	QByteArray convertStringToByteArray(QString str);
	bool sendArrayForDebug(QByteArray);
	interface_types type() {return USB;}
protected:
	void run();
	void on_autoscan();
	void on_pausescan();
	void on_resumescan();
	void on_cancelscan();
	void on_commandNextStep();
	void on_commandPreviousStep();
	void on_setWriteReadDelay_us(unsigned long value);
public slots:
signals:

private:
	typedef struct {
		char last_command_processed;
		char flags; //No meaningful data yet (not coded anything so far here)
		char port_A;
		char port_B;
		char port_C;
		char port_D;
		char port_E;
		char number_of_ADC_results;
		quint32 adcMAG;
		quint32 adcPhase;
	} usbB2command;
	union {
		usbB2command command;
		unsigned char data[16];
	} usbB2union;
	usbdevice usb;
	typedef struct {
		uint8_t latch;
		uint8_t mask;
		uint8_t pin;
	} parallelEqui;
	QHash<msa::MSAdevice, QHash<hardwareDevice::HWdevice, parallelEqui>> pinMapping;
	bool autoConnect;
	bool singleStep;
	parallelEqui *pll1data;
	parallelEqui *pll1le;
	parallelEqui *dds1data;
	parallelEqui *dds1fqu;

	parallelEqui *pll2data;
	parallelEqui *pll2le;
	parallelEqui *dds3data;
	parallelEqui *dds3fqu;

	parallelEqui *pll3data;
	parallelEqui *pll3le;
	genericPLL *pll1;
	genericPLL *pll2;
	genericPLL *pll3;

	genericDDS *dds1;
	genericDDS *dds3;

	genericADC *adcmag;
	genericADC *adcph;
	QHash<uint8_t, uint8_t> latchToUSBNumber;
	void commandStep(quint32 step);
	void commandInitStep(hardwareDevice *dev, quint32 step);
	void sendUSB(QByteArray data, uint8_t latch, bool autoClock, bool isADC = false);
	QString byteToString(uint8_t byte);
	QString constructString(uint8_t latch1, uint8_t latch2, uint8_t latch3, uint8_t latch4, QString clock);
	void usbToString(QByteArray array, bool print, int temp);
	QHash<quint32, QByteArray *> usbData;
	void printUSBData(quint32 step);
	QByteArray adcSend;
	int expectedAdcSize;
};

#endif // SLIMUSB_H
