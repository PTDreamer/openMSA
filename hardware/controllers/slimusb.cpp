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

slimusb::slimusb(QObject *parent): interface(parent), usb(parent),autoConnect(true)
{
	currentLatchValue.resize(5);
	latchToUSBNumber.insert(1,1);
	latchToUSBNumber.insert(2,3);
	latchToUSBNumber.insert(3,0);
	latchToUSBNumber.insert(4,2);
	latchToUSBNumber.insert(7,7);
}

bool slimusb::init(int debugLevel)
{
	if(!usb.init((debugLevel)))
		return false;
	if(autoConnect)
		usb.enableCallBack(true);
	return true;
}

slimusb::~slimusb()
{
	qDeleteAll(usbData.values());
}

void slimusb::commandStep(int step)
{
	sendUSB(*usbData.value(step), 7, false);
}

bool slimusb::getIsConnected() const
{
	return (usbdevice::deviceHandler != NULL);
}

void slimusb::commandNextStep()
{

}

void slimusb::commandPreviousStep()
{

}

void slimusb::initScan(bool inverted, double start, double end, double step)
{
	QList<hardwareDevice::devicePin*> dataPins;
	int maxSize = 0;
	interface::initScan(inverted, start, end, step);
	pll1->processNewScan();
	dds1->processNewScan();
	qDeleteAll(usbData.values());
	usbData.clear();
	foreach (hardwareDevice *dev, getCurrentHardwareDevices().values()) {
		foreach (hardwareDevice::devicePin *pin, dev->getDevicePins().values()) {
			if(pin->IOtype == hardwareDevice::MAIN_DATA) {
				dataPins.append(pin);
				if(pin->data.contains(0) && pin->data.value(0).dataArray->size() > maxSize)
					maxSize = pin->data.value(0).dataArray->size();
			}
		}
	}
	int delta;
	for(int step = 0; step < hardwareDevice::currentScan.steps.size(); ++step) {
		QByteArray *arr = new QByteArray;
		usbData.insert(step, arr);
		for(int b = 0; b < maxSize; ++b) {
			uint8_t byte = currentLatchValue.at(1);
			foreach (hardwareDevice::devicePin *pin, dataPins) {
				if(pin->data.contains(step)) {
					delta = maxSize - pin->data.value(step).dataArray->size() + pin->data.value(step).dataMask->count(0);
				}
				if(b >= delta) {
					byte = (byte & ((parallelEqui*)(pin->hwconfig))->mask) | pin->data.value(step).dataArray->at(b - delta) << ((parallelEqui*)(pin->hwconfig))->pin;

				}
			}
			arr->append(byte);
		}
	}
	commandStep(0);
}

void slimusb::hardwareInit(QHash<hardwareDevice::MSAdevice, hardwareDevice::HWdevice> devices)
{
	interface::hardwareInit(devices);
	QHash<hardwareDevice::MSAdevice, hardwareDevice *> loadedDevices = getCurrentHardwareDevices();
	if(loadedDevices.contains(hardwareDevice::PLL1) && (loadedDevices.value(hardwareDevice::PLL1)->getHardwareType() == hardwareDevice::LMX2326)) {
		pll1 = qobject_cast<genericPLL*>(loadedDevices.value(hardwareDevice::PLL1));
		QHash<int, hardwareDevice::devicePin*> pll1pins = pll1->getDevicePins();
		hardwareDevice::devicePin *pin;
		parallelEqui *p;
		foreach (int type, pll1pins.keys()) {
			switch (type) {
			case lmx2326::PIN_DATA:
				pin = pll1pins.value(type);
				pll1data = new parallelEqui;
				pll1data->latch = 1;
				pll1data->pin = 1;
				pll1data->mask = ~(1 << pll1data->pin);
				pin->hwconfig = pll1data;
				break;
			case lmx2326::PIN_CLK:
				pin = pll1pins.value(type);
				p = new parallelEqui;
				p->latch = 1;
				p->pin = 0;
				p->mask = ~(1 << p->pin);
				pin->hwconfig = p;
				break;
			case lmx2326::PIN_LE:
				pin = pll1pins.value(type);
				pll1le = new parallelEqui;
				pll1le->latch = 2;
				pll1le->pin = 0;
				pll1le->mask = ~(1 << pll1le->pin);
				pin->hwconfig = pll1le;
				break;
			default:
				pin = pll1pins.value(type);
				pin->hwconfig = NULL;
				break;
				break;
			}
		}
	}
	if(loadedDevices.contains(hardwareDevice::DDS1) && (loadedDevices.value(hardwareDevice::DDS1)->getHardwareType() == hardwareDevice::AD9850)) {
		dds1 = qobject_cast<genericDDS*>(loadedDevices.value(hardwareDevice::DDS1));
		QHash<int, hardwareDevice::devicePin*> dds1pins = dds1->getDevicePins();
		hardwareDevice::devicePin *pin;
		parallelEqui *p;
		foreach (int type, dds1pins.keys()) {
			switch (type) {
			case ad9850::PIN_DATA:
				pin = dds1pins.value(type);
				dds1data = new parallelEqui;
				dds1data->latch = 1;
				dds1data->pin = 2;
				dds1data->mask = ~(1 << dds1data->pin);
				pin->hwconfig = dds1data;
				break;
			case ad9850::PIN_WCLK:
				pin = dds1pins.value(type);
				p = new parallelEqui;
				p->latch = 1;
				p->pin = 0;
				p->mask = ~(1 << p->pin);
				pin->hwconfig = p;
				break;
			case ad9850::PIN_FQUD:
				pin = dds1pins.value(type);
				dds1fqu = new parallelEqui;
				dds1fqu->latch = 2;
				dds1fqu->pin = 1;
				dds1fqu->mask = ~(1 << dds1fqu->pin);
				pin->hwconfig = dds1fqu;
				break;
			default:
				pin = dds1pins.value(type);
				pin->hwconfig = NULL;
				break;
			}
		}
	}

	switch (pll1->getHardwareType()) {
	case hardwareDevice::LMX2326:
		pll1array = deviceParser::getDeviceList().value(hardwareDevice::PLL1)->devicePins.value(lmx2326::PIN_DATA)->data.value(HW_INIT_STEP).dataArray;
		break;
	default:
		break;
	}
	switch (dds1->getHardwareType()) {
	case hardwareDevice::AD9850:
		dds1array = deviceParser::getDeviceList().value(hardwareDevice::DDS1)->devicePins.value(ad9850::PIN_DATA)->data.value(HW_INIT_STEP).dataArray;
		break;
	default:
		break;
	}
	qDebug() << "PLL";
	foreach (int step, pll1->getInitIndexes()) {
		commandInitStep(pll1, step);
		usbToString(QByteArray(), true, 0);
	}
	qDebug() << "DDS";
	foreach (int step, dds1->getInitIndexes()) {
		commandInitStep(dds1, step);
		usbToString(QByteArray(), true, 0);
	}
}

void slimusb::commandInitStep(hardwareDevice *dev, int step) {

	int maxSize = 0;
	foreach (hardwareDevice::devicePin *pin, dev->getDevicePins()) {
		if(pin->data.contains(step)) {
			if(pin->data.value(step).dataArray->size() > maxSize)
				maxSize = pin->data.value(step).dataArray->size();
			if(pin->data.value(step).dataMask->size() > maxSize)
				maxSize = pin->data.value(step).dataMask->size();
		}
	};

	int previousLatch = -1;
	int previousVirtualCLK = -1;
	int currentLatch = -1;
	int currentVirtualCLK = -1;
	QByteArray sendArray;
	for(int x = 0; x < maxSize; ++x) {
		currentLatch = -1;
		currentVirtualCLK = -1;
		foreach (hardwareDevice::devicePin *pin, dev->getDevicePins()) {
			if(pin->data.contains(step) && pin->data.value(step).dataMask && (x < pin->data.value(step).dataMask->size()) && pin->data.value(step).dataMask->at(x)) {
				if(currentLatch == -1 && pin->hwconfig) {
					currentLatch = ((parallelEqui*)(pin->hwconfig))->latch;
					if(previousLatch == -1)
						previousLatch = currentLatch;
				} else if(pin->hwconfig && (currentLatch != ((parallelEqui*)(pin->hwconfig))->latch)){
					Q_ASSERT(false);
				}
			}
		}
		Q_ASSERT(currentLatch != -1);
		foreach (hardwareDevice::devicePin *pin, dev->getDevicePins()) {
			if(pin->data.contains(step) && pin->data.value(step).dataMask && (x < pin->data.value(step).dataMask->size()) && pin->data.value(step).dataMask->at(x)) {
				if(pin->IOtype == hardwareDevice::VIRTUAL_CLK) {
					currentVirtualCLK = pin->data.value(step).dataArray->at(x);
					if(previousVirtualCLK == -1)
						previousVirtualCLK = currentVirtualCLK;
					//qDebug() << "setting virtual clock to:" << currentVirtualCLK << "bit:" << x;
				}
			}
		}
		if(previousVirtualCLK == -1)
			previousVirtualCLK = 0;
		if((currentLatch != previousLatch) || ((currentVirtualCLK != previousVirtualCLK) && currentVirtualCLK !=-1)) {
			qDebug() << "currentLatch"<<currentLatch<< "previousLatch"<<previousLatch<<"currentVirtualCLK"<<currentVirtualCLK<<"previousVirtualCLK"<<previousVirtualCLK;
			if(sendArray.size() > 0) {
				sendUSB(sendArray, previousLatch, previousVirtualCLK == 1);
				sendArray.clear();
				previousLatch = currentLatch;
				previousVirtualCLK = currentVirtualCLK;
			}
		}

		uint8_t currentByte = 0;
		foreach (hardwareDevice::devicePin *pin, dev->getDevicePins()) {
			if(pin->data.contains(step) && pin->data.value(step).dataMask && (x < pin->data.value(step).dataMask->size()) && pin->data.value(step).dataMask->at(x)) {
				if(pin->hwconfig) {
					currentByte	= ((currentByte & ((parallelEqui*)(pin->hwconfig))->mask)) | (pin->data.value(step).dataArray->at(x) << ((parallelEqui*)(pin->hwconfig))->pin);
				}
			}
		}
		sendArray.append(currentByte);
	}
	if(sendArray.size() > 0) {
		sendUSB(sendArray, currentLatch, currentVirtualCLK == 1);
	}
}

void slimusb::sendUSB(QByteArray data, uint8_t latch, bool autoClock) {
	static int temp = 0;
	qDebug() << temp << "send latch:" << latch << "autoclk:"<< autoClock << data;
	static int z = 0;
	if(z == 2) {
		//while(true);
	}
	++z;
	int s =data.size();
	data.insert(0, (int)autoClock);
	data.insert(0, s);
	data.insert(0, 0xA0 + latchToUSBNumber.value(latch));
	if(usb.isConnected()) {
		usb.sendArray(data);
	}
	else
		usbToString(data, false, temp);
	++temp;
}
void slimusb::usbToString(QByteArray array, bool print, int temp) {
	QString str;
	unsigned char *data = reinterpret_cast<unsigned char*>(array.data());
	for(int i = 0; i < array.length(); ++i) {
		QString d = QString::number(data[i], 16);
		if(d.length() == 1)
			d.insert(0,"0");
		str.append(d);
	}
	qDebug() << "SENT";
	qDebug() << str;
	//	latchToUSBNumber.insert(1,1);
//	latchToUSBNumber.insert(2,3);
//	latchToUSBNumber.insert(3,0);
//	latchToUSBNumber.insert(4,2);
	static uint8_t latch1 = 0;
	static uint8_t latch2 = 0;
	static uint8_t latch3 = 0;
	static uint8_t latch4 = 0;
	QString autoClk;
	uint8_t *latch;
	static QStringList list;
	if(array.length() > 0) {
	if((array.at(0) & 0xA0) == 0xA0) {
		if((array.at(0) & 0x0F) == 0x00) {
			latch = &latch3;
		}
		else if((array.at(0) & 0x0F) == 0x01) {
			latch = &latch1;
		}
		else if((array.at(0) & 0x0F) == 0x02) {
			latch = &latch4;
		}
		else if((array.at(0) & 0x0F) == 0x03) {
			latch = &latch2;
		}
	}
	if((array.at(2)) == 0x01) {
		autoClk = "A";
	}
	else
		autoClk = "X";
	for(int x = 3; x < array.length(); ++ x) {
		*latch = array.at(x);
		list.append(QString::number(temp) + " " + constructString(latch1, latch2, latch3, latch4, autoClk));
	}
	}
	if(print) {
		list.append("");
		foreach (QString s, list) {
			qDebug() << s;
		}
		list.clear();
	}
}
QString slimusb::constructString(uint8_t latch1, uint8_t latch2, uint8_t latch3, uint8_t latch4, QString clock) {
	QString str;
	str = QString("%0 %1 %2 %3 %4").arg(clock).arg(byteToString(latch1)).arg(byteToString(latch2)).arg(byteToString(latch3)).arg(byteToString(latch4));
	return str;
}

QString slimusb::byteToString(uint8_t byte) {
	QString str;
	for(int x = 0; x < 8; ++x) {
		uint8_t v = ((byte) >> x) & 1;
		str.insert(0, QString::number(v));
	}
	return str;
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
	usb.enableCallBack(autoConnect);
}

void slimusb::printUSBData(int step) {
	QString str;
	unsigned char *data = reinterpret_cast<unsigned char*>(usbData.value(step)->data());
	for(int i = 0; i < usbData.value(step)->length(); ++i) {
		QString d = QString::number(data[i], 16);
		if(d.length() == 1)
			d.insert(0,"0");
		str.append(d);
	}
	qDebug() << str;
}

QList<usbdevice::usbDevice_t> slimusb::getDevices()
{
	return usb.getDevices();
}

bool slimusb::openDevice(int deviceNumber)
{
	return usb.openDevice(deviceNumber);
}

void slimusb::closeDevice()
{
	usb.closeDevice();
}
