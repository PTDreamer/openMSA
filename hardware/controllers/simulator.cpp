/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2018
 * @brief      simulator.cpp file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   simulator
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
#include "simulator.h"
#include <QDebug>
#include "../deviceparser.h"
#include "../lmx2326.h"
#include "../ad9850.h"
#include "../msa.h"
#include <QTimer>
#include <QRandomGenerator>

simulator::simulator(QObject *parent): interface(parent), autoConnect(true), singleStep(false), writeReadDelay_us(100), isPaused(false),
	pll1data(nullptr),pll1le(nullptr),dds1data(nullptr),dds1fqu(nullptr),pll2data(nullptr),pll2le(nullptr),dds3data(nullptr),dds3fqu(nullptr)
	,pll3data(nullptr),pll3le(nullptr),pll1(nullptr),pll2(nullptr),pll3(nullptr),dds1(nullptr),dds3(nullptr),adcmag(nullptr),adcph(nullptr)
{
	lastCommandedStep = 0;
	usbB2union.command.adcMAG = 0;
	usbB2union.command.adcPhase = 0;
	latchToUSBNumber.insert(1,1);
	latchToUSBNumber.insert(2,3);
	latchToUSBNumber.insert(3,0);
	latchToUSBNumber.insert(4,2);
	latchToUSBNumber.insert(7,7);
	dds1 = dds3 = nullptr;
	pll1 = pll2 = pll3 = nullptr;
	adcmag = adcph = nullptr;
	usbB2union.data[0] = 0xB2;
	usbB2union.data[8] = 0xA4;
	usbB2union.data[9] = 0x14;
	usbB2union.data[10] = 0x00;
	usbB2union.data[11] = 0x00;
}

bool simulator::init(int debugLevel)
{
	setDebugLevel(debugLevel);
	return true;
}

simulator::~simulator()
{
	this->requestInterruption();
	QThread::usleep(500);
}

void simulator::commandStep(quint32 step)
{
	if(msa::getInstance().currentInterface->getDebugLevel() > 1)
		qDebug()<<"step:"<< step;
	quint32 totalSteps = msa::getInstance().getScanConfiguration().gui.steps_number;
	double currentStepPart = double(step) / totalSteps;
	QThread::usleep(writeReadDelay_us);
	emit dataReady(step, quint32(5000 * (QRandomGenerator::global()->generateDouble() + sin(currentStepPart * 2 * M_PI)) + 20000), quint32(5000 * ( QRandomGenerator::global()->generateDouble()+cos(currentStepPart * 2 * M_PI)) + 20000));
}

bool simulator::getIsConnected() const
{
	return true;
}

void simulator::commandNextStep()
{
	commandStep(currentStep);
	if(!msa::getInstance().getIsInverted())
		++currentStep;
	if(currentStep > (numberOfSteps - 1))
		currentStep = 0;
	if(msa::getInstance().getIsInverted()) {
		if(currentStep == 0)
			currentStep = numberOfSteps -1;
		else {
			--currentStep;
		}
	}
}

void simulator::commandPreviousStep()
{
	if(!msa::getInstance().getIsInverted()) {
		if(currentStep == 0)
			currentStep = numberOfSteps - 1;
		else
			--currentStep;
	}
	if(msa::getInstance().getIsInverted())
		++currentStep;
	if(currentStep > (numberOfSteps - 1))
		currentStep = 0;
	commandStep(currentStep);
}

bool simulator::initScan()
{
	QList<hardwareDevice::devicePin*> dataPins;
	int maxSize = 0;
	bool error = false;
	interface::initScan();
	if(pll1)
		error |= pll1->processNewScan();
	if(dds1)
		error |= dds1->processNewScan();
	if(pll3)
		error |= pll3->processNewScan();
	if(dds3)
		error |= dds3->processNewScan();
	if(error)
		msa::getInstance().currentInterface->errorOcurred(msa::MSA, "Error ocurred processing new scan", true, true);
	qDeleteAll(usbData.values());
	usbData.clear();
	foreach (hardwareDevice *dev, msa::getInstance().currentHardwareDevices.values()) {
		foreach (hardwareDevice::devicePin *pin, dev->getDevicePins().values()) {
			if(pin->IOtype == hardwareDevice::MAIN_DATA) {
				dataPins.append(pin);
				if(pin->data.contains(0) && pin->data.value(0).dataArray->size() > maxSize)
					maxSize = pin->data.value(0).dataArray->size();
			}
		}
	}
	int delta;
	for(quint32 step = 0; step < quint32(msa::getInstance().currentScan.steps->size()); ++step) {
		QByteArray *arr = new QByteArray;
		usbData.insert(step, arr);
		for(int b = 0; b < maxSize; ++b) {
			uint8_t byte = 0;
			foreach (hardwareDevice::devicePin *pin, dataPins) {
				if(!pin->hwconfig)
					continue;
				if(pin->data.contains(step)) {
					delta = maxSize - pin->data.value(step).dataArray->size() + pin->data.value(step).dataMask->count(0);

					if(b >= delta) {
						byte = uint8_t((byte & (static_cast<parallelEqui*>(pin->hwconfig))->mask) | pin->data.value(step).dataArray->at(b - delta) << (static_cast<parallelEqui*>(pin->hwconfig))->pin);
					}
				}
			}
			arr->append(char(byte | msa::getInstance().getResolution_filter_bank()));
		}
	}
	adcSend.clear();
	if((msa::getInstance().currentScan.configuration.scanType != ComProtocol::VNA_Rec) && (msa::getInstance().currentScan.configuration.scanType != ComProtocol::VNA_Trans)) {
		adcSend.append(char(0xB2));//TODO
	}
	else
		adcSend.append(char(0xB2));
	if(adcmag && (adcmag->getHardwareType() == hardwareDevice::AD7685)) {
		expectedAdcSize = 16;
		adcSend.append(char(0x00));
		adcSend.append(0x01); adcSend.append(0x10);
	}
	else {
		adcSend.append(0x01);adcSend.append(0x03);adcSend.append(0x0C);
	}
	adcSend.append(char(msa::getInstance().currentScan.configuration.adcAveraging));
	return !error;
}
//first load the parallelEui struct with the configuration of each device (latch,pin, etc...)

void simulator::hardwareInit()
{
	interface::hardwareInit();
	QHash<msa::MSAdevice, hardwareDevice *> loadedDevices = msa::getInstance().currentHardwareDevices;
	foreach(hardwareDevice* dev, loadedDevices.values()) {
		if((loadedDevices.key(dev) == msa::PLL1) || (loadedDevices.key(dev) == msa::PLL2) || (loadedDevices.key(dev) == msa::PLL3)) {
			genericPLL *pll = qobject_cast<genericPLL*>(dev);
			if(loadedDevices.key(dev) == msa::PLL1)
				pll1 = pll;
			if(loadedDevices.key(dev) == msa::PLL2)
				pll2 = pll;
			if(loadedDevices.key(dev) == msa::PLL3)
				pll3 = pll;
			QHash<int, hardwareDevice::devicePin*> pllpins = pll->getDevicePins();
			hardwareDevice::devicePin *pin;
			parallelEqui *p;
			foreach (int type, pllpins.keys()) {
				pin = pllpins.value(type);
				p = new parallelEqui;
				switch (type) {
				case lmx2326::PIN_DATA:
					switch (loadedDevices.key(dev)) {
					case msa::PLL1:
						p->latch = 1;
						p->pin = 1;
						pll1data = p;
						break;
					case msa::PLL2:
						p->latch = 1;
						p->pin = 4;
						pll2data = p;
						break;
					case msa::PLL3:
						p->latch = 1;
						p->pin = 3;
						pll3data = p;
						break;
					default:
						break;
					}
					break;
				case lmx2326::PIN_CLK:
					p->latch = 1;
					p->pin = 0;
					break;
				case lmx2326::PIN_LE:
					p->latch = 2;
					switch (loadedDevices.key(dev)) {
					case msa::PLL1:
						p->pin = 0;
						pll1le = p;
						break;
					case msa::PLL2:
						p->pin = 4;
						pll2le = p;
						break;
					case msa::PLL3:
						p->pin = 2;
						pll3le = p;
						break;
					default:
						break;
					}
					break;
				default:
					pin = pllpins.value(type);
					delete p;
					p = nullptr;
					break;
				}
				if(p)
					p->mask = uint8_t(~(1 << p->pin));
				pin->hwconfig = p;
			}

		}
		else if((loadedDevices.key(dev) == msa::DDS1) || (loadedDevices.key(dev) == msa::DDS3)) {
			genericDDS *dds = qobject_cast<genericDDS*>(dev);
			if(loadedDevices.key(dev) == msa::DDS1)
				dds1 = dds;
			else if(loadedDevices.key(dev) == msa::DDS3)
				dds3 = dds;
			QHash<int, hardwareDevice::devicePin*> ddspins = dds->getDevicePins();
			hardwareDevice::devicePin *pin;
			parallelEqui *p;
			foreach (int type, ddspins.keys()) {
				pin = ddspins.value(type);
				p = new parallelEqui;
				switch (type) {
				case ad9850::PIN_DATA:
					switch (loadedDevices.key(dev)) {
					case msa::DDS1:
						p->latch = 1;
						p->pin = 2;
						dds1data = p;
						break;
					case msa::DDS3:
						p->latch = 1;
						p->pin = 4;
						dds3data = p;
						break;
					default:
						break;
					}
				break;
				case ad9850::PIN_WCLK:
					switch (loadedDevices.key(dev)) {
					case msa::DDS1:
					case msa::DDS3:
						p->latch = 1;
						p->pin = 0;
						break;
					default:
						break;
					}
				break;
				case ad9850::PIN_FQUD:
					switch (loadedDevices.key(dev)) {
					case msa::DDS1:
						p->latch = 2;
						p->pin = 1;
						dds1fqu = p;
						break;
					case msa::DDS3:
						p->latch = 2;
						p->pin = 3;
						dds3fqu = p;
						break;
					default:
						break;
					}
				break;
				default:
					pin = ddspins.value(type);
					delete p;
					p = nullptr;
					break;

				}
				if(p)
					p->mask = uint8_t(~(1 << p->pin));
				pin->hwconfig = p;
			}
		}
		else if((loadedDevices.key(dev) == msa::ADC_MAG) || (loadedDevices.key(dev) == msa::ADC_PH)) {
			genericADC *adc = qobject_cast<genericADC*>(dev);
			if(loadedDevices.key(dev) == msa::ADC_MAG)
				adcmag = adc;
			else if(loadedDevices.key(dev) == msa::ADC_PH)
				adcph = adc;
			QHash<int, hardwareDevice::devicePin*> adcpins = adc->getDevicePins();
			hardwareDevice::devicePin *pin;
			parallelEqui *p;
			foreach (int type, adcpins.keys()) {
				pin = adcpins.value(type);
				p = new parallelEqui;
				switch (type) {
				case genericADC::PIN_DATA:
					switch (loadedDevices.key(dev)) {
					case msa::ADC_MAG:
						p->latch = 0;
						p->pin = 4;
						break;
					case msa::ADC_PH:
						p->latch = 0;
						p->pin = 5;
						break;
					default:
						break;
					}
					break;
				case genericADC::PIN_CLK:
					switch (loadedDevices.key(dev)) {
					case msa::ADC_MAG:
					case msa::ADC_PH:
						p->latch = 3;
						p->pin = 6;
						break;
					default:
						break;
					}
					break;
				case genericADC::PIN_CONVERT:
					switch (loadedDevices.key(dev)) {
					case msa::ADC_MAG:
					case msa::ADC_PH:
						p->latch = 3;
						p->pin = 7;
						break;
					default:
						break;
					}
					break;
				default:
					pin = adcpins.value(type);
					delete p;
					p = nullptr;
					break;
				}
				if(p)
					p->mask = uint8_t(~(1 << p->pin));
				pin->hwconfig = p;
			}
		}
	}
}

QByteArray simulator::convertStringToByteArray(QString str) {
	QByteArray ret;
	bool ok;
	for(int x = 0; x < str.length(); x = x + 2) {
		ret.append(char(str.mid(x, 2).toInt(&ok,16)));
	}
	return ret;
}

void simulator::run()
{
	forever {
		if ( QThread::currentThread()->isInterruptionRequested() ) {
			return;
		}
		commandStep(currentStep);
		if(!msa::getInstance().getIsInverted())
			++currentStep;
		if(currentStep > (numberOfSteps - 1))//TODO was >=
			currentStep = 0;
		if(msa::getInstance().getIsInverted()) {
			if(currentStep == 0)
				currentStep = numberOfSteps - 1;
			else
				--currentStep;
		}
	}
}

void simulator::commandInitStep(hardwareDevice *dev, quint32 step) {
	Q_UNUSED(dev);
	Q_UNUSED(step);
}


QString simulator::byteToString(uint8_t byte) {
	QString str;
	for(int x = 0; x < 8; ++x) {
		uint8_t v = ((byte) >> x) & 1;
		str.insert(0, QString::number(v));
	}
	return str;
}

void simulator::autoScan()
{
	this->start();
}

void simulator::pauseScan()
{
	this->requestInterruption();
	this->wait(1000);
	isPaused = true;
}

void simulator::resumeScan()
{
	this->start();
	isPaused = false;
}

void simulator::cancelScan()
{
	this->requestInterruption();
}

bool simulator::getAutoConnect() const
{
	return autoConnect;
}

void simulator::setAutoConnect(bool value)
{
	autoConnect = value;
}

unsigned long simulator::getWriteReadDelay_us() const
{
	return writeReadDelay_us;
}

void simulator::setWriteReadDelay_us(unsigned long value)
{
	writeReadDelay_us = value;
}

bool simulator::isScanning()
{
	return this->isRunning();
}
