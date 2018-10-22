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
#include <iostream>
#include <libusb-1.0/libusb.h>
#include <QThread>
#include <QVector>
#include "interface.h"
#include <QHash>
#include "../lmx2326.h"
#include "../ad9850.h"

using namespace std;

class hotplugWorker : public QObject
{
	Q_OBJECT

public slots:
	void doWork(libusb_context *context);
	void quit() {run = false;}
private:
	bool run;
signals:
	void connected();
	void disconnected();
};

class slimusb : public interface
{
	Q_OBJECT
public:
	typedef struct {
		QString name;
		QString serial;
		int deviceNumber;
	} usbDevice;
	slimusb(QObject *parent);
	bool init(int debugLevel);
	~slimusb();
	bool openDevice(int deviceNumber);
	void closeDevice();
	bool isHotPlugCapable();
	QList<usbDevice> getDevices();
	bool getAutoConnect() const;
	void setAutoConnect(bool value);
	bool getIsConnected() const;
	void initScan(bool inverted, double start, double end, double step);
	void hardwareInit(QHash<hardwareDevice::MSAdevice, hardwareDevice::HWdevice> devices);
protected slots:
	void commandNextStep();
	void commandPreviousStep();
	void autoScan();
	void pauseScan();
	void resumeScan();
signals:
	void disconnected();
	void connected();
private:
	void signalConnected();
	void signalDisconnected();
	typedef struct {
		uint8_t latch;
		uint8_t mask;
		uint8_t pin;
	} parallelEqui;
	QHash<hardwareDevice::MSAdevice, QHash<hardwareDevice::HWdevice, parallelEqui>> pinMapping;
	libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
	libusb_device_handle *dev_handle; //a device handle
	libusb_context *ctx = NULL; //a libusb session
	void printdev(libusb_device *dev);
	int enableCallBack(bool enable);
	static int LIBUSB_CALL hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data);
	QThread worker;
	hotplugWorker *w = NULL;
	static libusb_device_handle *deviceHandler;
	bool autoConnect;
	uint8_t data1;
	uint8_t data2;
	uint8_t data3;
	uint8_t data4;
	parallelEqui *pll1data;
	parallelEqui *pll1le;
	parallelEqui *dds1data;
	parallelEqui *dds1fqu;
	genericPLL *pll1;
	genericDDS *dds1;
	QBitArray *pll1array;
	QBitArray *dds1array;

signals:
	void startHotplugCallback(libusb_context *context);
};

#endif // SLIMUSB_H
