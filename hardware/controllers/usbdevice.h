/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2018
 * @brief      usbdevice.h file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   usbdevice
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
#ifndef USBDEVICE_H
#define USBDEVICE_H

#include <QObject>
#include <iostream>
#include <libusb-1.0/libusb.h>
#include <QThread>
#include <QDebug>

#define G8_VID 0x0547
#define G8_PID 0x1015

using namespace std;

class hotplugWorker : public QObject
{
	Q_OBJECT

public slots:
	void doWork(libusb_context *context);
	void quit();
private:
	bool run;
signals:
	void connected();
	void disconnected();
};


class usbdevice : public QObject
{
	Q_OBJECT
public:
	typedef struct {
		QString name;
		QString serial;
		int deviceNumber;
	} usbDevice_t;
	explicit usbdevice(QObject *parent = 0);
	bool init(int debugLevel);
	bool openDevice(int deviceNumber);
	void closeDevice();
	bool isHotPlugCapable();
	QList<usbDevice_t> getDevices();
	~usbdevice();
	int enableCallBack(bool enable);
	static libusb_device_handle *deviceHandler;
	bool isConnected() {return usbdevice::deviceHandler != NULL;}
	bool sendArray(QByteArray data);
protected:

private:
	libusb_device_handle *dev_handle; //a device handle
	libusb_device **devs; //pointer to pointer of device, used to retrieve a list of devices
	libusb_context *ctx = NULL; //a libusb session
	void printdev(libusb_device *dev);
	static int LIBUSB_CALL hotplug_callback(libusb_context *ctx, libusb_device *dev, libusb_hotplug_event event, void *user_data);
	QThread worker;
	hotplugWorker *w = NULL;
	void signalConnected();
	void signalDisconnected();
signals:
	void disconnected();
	void connected();
	void startHotplugCallback(libusb_context*);
public slots:
};

#endif // USBDEVICE_H
