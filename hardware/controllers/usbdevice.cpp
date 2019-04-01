/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2018
 * @brief      usbdevice.cpp file
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
#include "usbdevice.h"

libusb_device_handle *usbdevice::deviceHandler = NULL;

usbdevice::usbdevice(QObject *parent) : QObject(parent),devs(NULL)
{
	connect(this, SIGNAL(closeWorker()), &worker, SLOT(quit()));
}

bool usbdevice::init(int debugLevel)
{
	int r; //for return values
	r = libusb_init(&ctx); //initialize the library for the session we just declared
	if(r < 0) {
		cout<<"Init Error "<<r<<endl; //there was an error
		return false;
	}
	libusb_set_debug(ctx, debugLevel); //set verbosity level to 3, as suggested in the documentation
	return true;
}

usbdevice::~usbdevice()
{
	enableCallBack(false);
	worker.wait(1000);
	if(deviceHandler) {
		libusb_close(deviceHandler);
		//qDebug() << "closing device";
	}
	libusb_free_device_list(devs, 1); //free the list, unref the devices in it
	libusb_exit(ctx);
	//qDebug()<< "end destructor";
	delete w;
}

bool usbdevice::openDevice(int deviceNumber)
{
	libusb_open(devs[deviceNumber], &deviceHandler);
	int r = libusb_claim_interface(deviceHandler, 0); //claim interface 0 (the first) of device (mine had jsut 1)
	if(r < 0) {
		cout<<"Cannot Claim Interface"<<endl;
		return false;
	}
	if(deviceHandler) {
		//qDebug() << "USB device connected";
		emit connected();
		return true;
	}
	else
		return false;
}

void usbdevice::closeDevice()
{
	libusb_close(deviceHandler);
	emit disconnected();
}

bool usbdevice::isHotPlugCapable()
{
	if (!libusb_has_capability (LIBUSB_CAP_HAS_HOTPLUG)) {
		//qDebug()<< ("Hotplug capabilites are not supported on this platform\n");
		libusb_exit (NULL);
		return false;
	}
	else {
		//qDebug()<< ("Hotplug capabilites are SUPPORTED\n");
		return true;
	}
}

QList<usbdevice::usbDevice_t> usbdevice::getDevices()
{
	if(devs)
		libusb_free_device_list(devs, 1); //free the list, unref the devices in it
	QList<usbDevice_t> ret;
	ssize_t cnt; //holding number of devices in list
	cnt = libusb_get_device_list(ctx, &devs); //get the list of devices
	if(cnt < 0) {
		cout<<"Get Device Error"<<endl; //there was an error
		return ret;
	}
	for(int i = 0; i < cnt; i++) {
		libusb_device_descriptor desc;
		int r = libusb_get_device_descriptor(devs[i], &desc);
		if((r >=0)  && (desc.idVendor == G8_VID) && (desc.idProduct == G8_PID)) {
			//printdev(devs[i]); //print specs of this device
			libusb_device_handle *m_handle;
			libusb_open(devs[i], &m_handle);// libusb_open_device_with_vid_pid(ctx, 0x0547, 0x1015); //these are vendorID and productID I found for my usb device
			if(m_handle == NULL) {
				cout<<"Cannot open device"<<endl;
				continue;
			}
			else {
				libusb_device_descriptor desc;
				int r = libusb_get_device_descriptor(devs[i], &desc);
				if (r < 0) {
					libusb_close(m_handle);
					continue;
				}
				unsigned char productStr[256];
				unsigned char serialStr[256];

				libusb_get_string_descriptor_ascii(m_handle, desc.iProduct, productStr, sizeof(productStr));
				usbDevice_t d;
				d.name = QString::fromLatin1((char*)productStr);
				libusb_get_string_descriptor_ascii(m_handle, desc.iSerialNumber, serialStr, sizeof(serialStr));
				d.serial = QString::fromLatin1((char*)serialStr);
				d.deviceNumber = i;
				ret.append(d);
				libusb_close(m_handle);
			}
		}
	}
	return ret;
}

void usbdevice::printdev(libusb_device *dev) {
	libusb_device_descriptor desc;

	int r = libusb_get_device_descriptor(dev, &desc);
	if (r < 0) {
		cout<<"failed to get device descriptor"<<endl;
		return;
	}
	cout<<"Number of possible configurations: "<<(int)desc.bNumConfigurations<<"  ";
	cout<<"Device Class: "<<(int)desc.bDeviceClass<<"  ";
	cout<<"VendorID: "<<desc.idVendor<<"  ";
	cout<<"ProductID: "<<desc.idProduct<<endl;
	libusb_config_descriptor *config;
	libusb_get_config_descriptor(dev, 0, &config);
	cout<<"Interfaces: "<<(int)config->bNumInterfaces<<" ||| ";
	const libusb_interface *inter;
	const libusb_interface_descriptor *interdesc;
	const libusb_endpoint_descriptor *epdesc;
	for(int i=0; i<(int)config->bNumInterfaces; i++) {
		inter = &config->interface[i];
		cout<<"Number of alternate settings: "<<inter->num_altsetting<<" | ";
		for(int j=0; j<inter->num_altsetting; j++) {
			interdesc = &inter->altsetting[j];
			cout<<"Interface Number: "<<(int)interdesc->bInterfaceNumber<<" | ";
			cout<<"Number of endpoints: "<<(int)interdesc->bNumEndpoints<<" | ";
			for(int k=0; k<(int)interdesc->bNumEndpoints; k++) {
				epdesc = &interdesc->endpoint[k];
				cout<<"Descriptor Type: "<<(int)epdesc->bDescriptorType<<" | ";
				cout<<"EP Address: "<<(int)epdesc->bEndpointAddress<<" | ";
			}
		}
	}
	cout<<endl<<endl<<endl;
	libusb_free_config_descriptor(config);
}

int usbdevice::enableCallBack(bool enable)
{
	static libusb_hotplug_callback_handle handle;
	static bool callbackEnabled = false;
	static bool once = true;
	if(once) {
		if(!isHotPlugCapable())
			return false;
	}
	once = false;
	if(enable && !callbackEnabled) {
		int rc;
		libusb_init(nullptr);
		rc = libusb_hotplug_register_callback(nullptr, libusb_hotplug_event(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
																		   LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), libusb_hotplug_flag(LIBUSB_HOTPLUG_ENUMERATE), G8_VID, G8_PID,
											  LIBUSB_HOTPLUG_MATCH_ANY, libusb_hotplug_callback_fn(usbdevice::hotplug_callback), this,
											  &handle);
		if (LIBUSB_SUCCESS != rc) {
			printf("Error creating a hotplug callback\n");
			libusb_exit(nullptr);
			return false;
		}
		w = new hotplugWorker;
		w->moveToThread(&worker);
		worker.start();
		worker.setPriority(QThread::LowestPriority);
		connect(this, SIGNAL(startHotplugCallback(libusb_context*)), w, SLOT(doWork(libusb_context*)));
		emit startHotplugCallback(ctx);
		callbackEnabled = true;
		return true;
	}
	else if(callbackEnabled){
		if(w)
			w->quit();
		worker.quit();
		libusb_hotplug_deregister_callback(nullptr, handle);
		libusb_exit(nullptr);
		callbackEnabled = false;
		return true;
	}
	return true;
}

bool usbdevice::sendArray(QByteArray data, unsigned char* receivedData, int expectedSize) {
	if(!usbdevice::deviceHandler) {
		return false;
	}
	int actual;
	int r = libusb_bulk_transfer(deviceHandler, (2 | LIBUSB_ENDPOINT_OUT), reinterpret_cast<unsigned char*>(data.data()), data.size(), &actual, 0);
	if(r != 0)
	{
		if(usbdevice::deviceHandler)
			libusb_close(usbdevice::deviceHandler);
		usbdevice::deviceHandler = NULL;
		//qDebug() << "NOT reveived ADC2";
		emit disconnected();
		return false;
	}
	for(int x = 0; x < 10; ++x) {
		r = libusb_bulk_transfer(deviceHandler, (6 | LIBUSB_ENDPOINT_IN), receivedData, expectedSize, &actual, 0);
		if(r != 0)
		{
			if(usbdevice::deviceHandler)
				libusb_close(usbdevice::deviceHandler);
			usbdevice::deviceHandler = NULL;
			emit disconnected();
			//qDebug() << "NOT reveived ADC2" << r;
			Q_ASSERT(false);
			return false;
		}
		else if(actual == expectedSize) {
			return true;
		}
		else if(x > 5)
			qDebug() << "actual" << actual << "expected" << expectedSize;
	}
	qDebug() << "NOT reveived ADC3";
	return false;
}

bool usbdevice::sendArray(QByteArray data)
{
	if(!usbdevice::deviceHandler)
		return false;
	int actual;
	int r = libusb_bulk_transfer(deviceHandler, (2 | LIBUSB_ENDPOINT_OUT), reinterpret_cast<unsigned char*>(data.data()), data.size(), &actual, 0);
	if(r != 0)
	{
		if(usbdevice::deviceHandler)
			libusb_close(usbdevice::deviceHandler);
		usbdevice::deviceHandler = nullptr;
		emit disconnected();
		return false;
	}
	return true;
}

int LIBUSB_CALL usbdevice::hotplug_callback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data) {
	Q_UNUSED(ctx)
	Q_UNUSED(user_data)
	usbdevice *th = static_cast<usbdevice*>(user_data);
	struct libusb_device_descriptor desc;
	int rc;
	(void)libusb_get_device_descriptor(dev, &desc);
	if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event) {
		rc = libusb_open(dev, &deviceHandler);
		int r;
		r = libusb_claim_interface(deviceHandler, 0); //claim interface 0 (the first) of device (mine had jsut 1)
		if(r < 0) {
			cout<<"Cannot Claim Interface"<<endl;
			return 1;
		}
		cout<<"Claimed Interface"<<endl;
		if (LIBUSB_SUCCESS != rc) {
			//qDebug()<<("Could not open USB device\n");
		}
		else {
			if(th)
				th->signalConnected();
			//qDebug() << "Device connected";
		}
	} else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
		if (deviceHandler) {
			if(th)
				th->signalDisconnected();
			//qDebug() << "Device disconnected";
			libusb_close(deviceHandler);
			deviceHandler = NULL;
		}
	} else {
		//qDebug()<< QString("Unhandled event %d\n").arg(event);
	}
	return 0;
}

void usbdevice::signalConnected()
{
	emit connected();
}

void usbdevice::signalDisconnected()
{
	emit disconnected();
}

void hotplugWorker::doWork(libusb_context *context)
{
	run = true;
	timeval t;
	t.tv_sec = 2;
	t.tv_usec = 0;
	while (run) {
		libusb_handle_events_timeout_completed(context, &t, NULL);
	}
	//qDebug()<<("quit hotplugWorker");
}

void hotplugWorker::quit()
{
	 run = false;
	 //qDebug() << "QUIT";
}
