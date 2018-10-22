/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2018
 * @brief      slimusb.cpp file
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
#include "slimusb.h"
#include <QDebug>
#include "../deviceparser.h"
#include "../lmx2326.h"
#include "../ad9850.h"

#define G8_VID 0x0547
#define G8_PID 0x1015

libusb_device_handle *slimusb::deviceHandler = NULL;

slimusb::slimusb(QObject *parent): interface(parent), devs(NULL), autoConnect(false)
{
}

bool slimusb::init(int debugLevel)
{
	int r; //for return values
	r = libusb_init(&ctx); //initialize the library for the session we just declared
	if(r < 0) {
		cout<<"Init Error "<<r<<endl; //there was an error
		return false;
	}
	libusb_set_debug(ctx, debugLevel); //set verbosity level to 3, as suggested in the documentation

	if(autoConnect)
		enableCallBack(true);
	return true;
}

slimusb::~slimusb()
{
	if(deviceHandler)
		libusb_close(deviceHandler);
	libusb_free_device_list(devs, 1); //free the list, unref the devices in it
	enableCallBack(false);
	worker.wait(10000);
}

bool slimusb::openDevice(int deviceNumber)
{
	libusb_open(devs[deviceNumber], &deviceHandler);
	if(deviceHandler) {
		qDebug() << "USB device connected";
		return true;
	}
	else
		return false;
}

bool slimusb::isHotPlugCapable()
{
	if (!libusb_has_capability (LIBUSB_CAP_HAS_HOTPLUG)) {
			qDebug()<< ("Hotplug capabilites are not supported on this platform\n");
			libusb_exit (NULL);
			return false;
		}
	else {
		qDebug()<< ("Hotplug capabilites are SUPPORTED\n");
		return true;
	}
}

QList<slimusb::usbDevice> slimusb::getDevices()
{
	if(devs)
		libusb_free_device_list(devs, 1); //free the list, unref the devices in it
	QList<usbDevice> ret;
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
				usbDevice d;
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

void slimusb::printdev(libusb_device *dev) {
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

int slimusb::enableCallBack(bool enable)
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
		libusb_init(NULL);
		rc = libusb_hotplug_register_callback(NULL, (libusb_hotplug_event)(LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
											  LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT), (libusb_hotplug_flag)LIBUSB_HOTPLUG_ENUMERATE, G8_VID, G8_PID,
											  LIBUSB_HOTPLUG_MATCH_ANY, (libusb_hotplug_callback_fn)slimusb::hotplug_callback, NULL,
											  &handle);
		if (LIBUSB_SUCCESS != rc) {
		  printf("Error creating a hotplug callback\n");
		  libusb_exit(NULL);
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
		libusb_hotplug_deregister_callback(NULL, handle);
		libusb_exit(NULL);
		callbackEnabled = false;
		return true;
	}
	return true;
}

int LIBUSB_CALL slimusb::hotplug_callback(struct libusb_context *ctx, struct libusb_device *dev, libusb_hotplug_event event, void *user_data) {
	Q_UNUSED(ctx)
	Q_UNUSED(user_data)
	struct libusb_device_descriptor desc;
	int rc;
	(void)libusb_get_device_descriptor(dev, &desc);
	if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event) {
		rc = libusb_open(dev, &deviceHandler);
		if (LIBUSB_SUCCESS != rc) {
			qDebug()<<("Could not open USB device\n");
		}
		else {
			qDebug() << "Device connected";
		}
	} else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
		if (deviceHandler) {
			qDebug() << "Device disconnected";
			libusb_close(deviceHandler);
			deviceHandler = NULL;
		}
	} else {
		qDebug()<< QString("Unhandled event %d\n").arg(event);
	}
	return 0;
}

bool slimusb::getIsConnected() const
{
	return (deviceHandler != NULL);
}

void slimusb::commandNextStep()
{

}

void slimusb::commandPreviousStep()
{

}

void slimusb::initScan(bool inverted, double start, double end, double step)
{
	QBitArray *pll1array = NULL;
	QBitArray *dds1array = NULL;
	interface::initScan(inverted, start, end, step);
	genericPLL *pll1dev = (genericPLL*)deviceParser::getDeviceList().value(hardwareDevice::PLL1);
	genericDDS *dds1dev = (genericDDS*)deviceParser::getDeviceList().value(hardwareDevice::DDS1);
	hardwareDevice::setNewScan(currentScan);
	pll1dev->processNewScan();
	dds1dev->processNewScan();
	switch (pll1dev->getHardwareType()) {
	case hardwareDevice::LMX2326:
		pll1array = deviceParser::getDeviceList().value(hardwareDevice::PLL1)->devicePins.value(lmx2326::PIN_DATA)->dataArray.value(INIT_STEP);
		break;
	default:
		break;
	}
	switch (dds1dev->getHardwareType()) {
	case hardwareDevice::AD9850:
		dds1array = deviceParser::getDeviceList().value(hardwareDevice::DDS1)->devicePins.value(ad9850::PIN_DATA)->dataArray.value(INIT_STEP);
		break;
	default:
		break;
	}
}

void slimusb::autoScan()
{

}

void slimusb::pauseScan()
{

}

void slimusb::resumeScan()
{

}

bool slimusb::getAutoConnect() const
{
	return autoConnect;
}

void slimusb::setAutoConnect(bool value)
{
	autoConnect = value;
	enableCallBack(autoConnect);
}

void hotplugWorker::doWork(libusb_context *context)
{
	run = true;
	while (run) {
		QThread::sleep(1);
		libusb_handle_events_completed(context, NULL);
	}
	qDebug()<<("quit hotplugWorker");
}
