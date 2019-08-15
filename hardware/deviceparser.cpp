/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2018
 * @brief      deviceparser.cpp file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   deviceParser
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
#include "deviceparser.h"
#include <QDebug>
#include "lmx2326.h"
#include "ad9850.h"
#include "genericadc.h"
#include "../hardware/controllers/interface.h"
#include <cmath>

deviceParser::~deviceParser()
{
}

deviceParser::deviceParser(msa::MSAdevice dev, hardwareDevice *parent) : QObject(parent),
	msadev(dev),
	device(parent)
{
	lmx2326 *l = dynamic_cast<lmx2326 *>(parent);
	ad9850 *a = dynamic_cast<ad9850 *>(parent);
	genericADC *adc = dynamic_cast<genericADC *>(parent);
	if (l)
		hwdev = hardwareDevice::LMX2326;
	else if (a)
		hwdev = hardwareDevice::AD9850;
	else if (adc)
		hwdev = adc->getHardwareType();
}

double deviceParser::parsePLLRCounter(msa::scanConfig config)
{
	double ret = -1;
	switch (msadev) {
	case msa::PLL1:
		switch (hwdev) {
		case hardwareDevice::LMX2326:
			ret = std::round(config.appxdds1/config.PLL1phasefreq);
			break;
		default:
			break;
		}
		break;
	case msa::PLL2:
		switch (hwdev) {
		case hardwareDevice::LMX2326:
			ret = std::round(config.masterOscilatorFrequency / config.PLL2phasefreq);
			break;
		default:
			break;
		}
		break;
	case msa::PLL3:
		switch (hwdev) {
		case hardwareDevice::LMX2326:
			ret = std::round(config.appxdds3/config.PLL3phasefreq);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return ret;
}

#define myDebug() qDebug() << fixed << qSetRealNumberPrecision(12)

double deviceParser::parsePLLNCounter(msa::scanConfig configuration, msa::scanStep &step, quint32 stepNumber, bool &error, bool &fatalError)
{
	error = false;
	fatalError = false;
	double ncounter = 0;
	double ncount = 0;
	genericPLL *lmx = nullptr;
	switch (msadev) {
	case msa::PLL1:
		if (stepNumber == quint32(HW_INIT_STEP))
			return -1;
		switch (hwdev) {
		case hardwareDevice::LMX2326:
			lmx = dynamic_cast<genericPLL *>(msa::getInstance().currentHardwareDevices.value(msadev));
			step.LO1 = configuration.baseFrequency + step.translatedFrequency + configuration.LO2 - configuration.pathCalibration.centerFreq_MHZ;
			if (step.LO1 > 2200) {
				msa::getInstance().currentInterface->errorOcurred(msadev, QString("LO1 will be above 2200MHz for step %1").arg(stepNumber), true, false);
				error = true;
				fatalError = true;
			} else if (step.LO1 < 950) {
				msa::getInstance().currentInterface->errorOcurred(msadev, QString("LO1 will be below 950MHz for step %1").arg(stepNumber), true, false);
				error = true;
				fatalError = true;
			}
			ncount = step.LO1/(configuration.appxdds1/ lmx->getRCounter()); // approximates the Ncounter for PLL
			ncounter = int(std::round(ncount)); // approximates the ncounter for PLL
			lmx->setPFD(step.LO1/ncounter, stepNumber);// approx phase freq of PLL
			if (msa::getInstance().currentInterface->getDebugLevel() > 2) {
				myDebug() << "LO1 step:"<< stepNumber << step.LO1 <<"="<< configuration.baseFrequency <<"+"<< step.translatedFrequency <<"+"<< configuration.LO2 <<"-"
						  << configuration.pathCalibration.centerFreq_MHZ;
				myDebug()<< "PLL1 "<< "step:"<<stepNumber << "PFD:"<<step.LO1/ncounter;
			}
			break;
		default:
			break;
		}
		break;
	case msa::PLL2:
		switch (hwdev) {
		case hardwareDevice::LMX2326:
			if (stepNumber == quint32(HW_INIT_STEP)) {
				lmx = dynamic_cast<genericPLL *>(msa::getInstance().currentHardwareDevices.value(msadev));
				ncount = configuration.LO2/(configuration.masterOscilatorFrequency / lmx->getRCounter()); // approximates the Ncounter for PLL
				ncounter = int(std::round(ncount)); // approximates the ncounter for PLL
			lmx->setPFD(configuration.LO2/ncounter, stepNumber);// approx phase freq of PLL
			}
			else if(configuration.cavityTestRunning){
				lmx = dynamic_cast<genericPLL *>(msa::getInstance().currentHardwareDevices.value(msadev));
				ncount = (step.LO1 + configuration.pathCalibration.centerFreq_MHZ)/(configuration.masterOscilatorFrequency/ lmx->getRCounter()); // approximates the Ncounter for PLL
				ncounter = int(std::round(ncount)); // approximates the ncounter for PLL
				lmx->setPFD(step.LO1 + configuration.pathCalibration.centerFreq_MHZ, stepNumber);// approx phase freq of PLL
			}
			break;
		default:
			break;
		}
		break;
	case msa::PLL3:
		step.LO3 = 0;
		if (stepNumber == quint32(HW_INIT_STEP))
			return -1;
		switch (hwdev) {
		case hardwareDevice::LMX2326:
			lmx = dynamic_cast<genericPLL *>(msa::getInstance().currentHardwareDevices.value(msadev));
			if (configuration.scanType == ComProtocol::SA_TG) {
				if (!configuration.gui.TGreversed) {
					if (step.band == 3)
						step.LO3 = step.realFrequency + configuration.gui.TGoffset - configuration.LO2;
					else
						step.LO3 = configuration.LO2 + step.translatedFrequency + configuration.gui.TGoffset;
				} else {
					double reversedFrequency;
					quint32 reversedIndex = uint(msa::getInstance().currentScan.steps->size()) - stepNumber - 1;
					if (step.band == 1)
						reversedFrequency = msa::getInstance().currentScan.steps->value(reversedIndex).realFrequency;
					else
						reversedFrequency = msa::getInstance().currentScan.steps->value(reversedIndex).translatedFrequency;
					if (step.band == 3)
						step.LO3 = msa::getInstance().currentScan.steps->value(reversedIndex).realFrequency + configuration.gui.TGoffset - configuration.LO2;
					else
						step.LO3 = configuration.LO2 + reversedFrequency + configuration.gui.TGoffset;
				}
				step.LO3 = configuration.LO2 - configuration.pathCalibration.centerFreq_MHZ - configuration.gui.TGoffset;
			} else if (configuration.scanType == ComProtocol::SA_SG) {
				if (configuration.gui.SGout <= configuration.LO2)
					step.LO3 = configuration.gui.SGout + configuration.LO2;
				else if (configuration.gui.SGout > (2*configuration.LO2))
					step.LO3 = configuration.gui.SGout - configuration.LO2;
				else
					step.LO3 = configuration.gui.SGout;
			}
			if (lmx) {
				ncount = step.LO3/(configuration.appxdds3/ lmx->getRCounter()); // approximates the Ncounter for PLL
				ncounter = int(std::round(ncount)); // approximates the ncounter for PLL
				lmx->setPFD(step.LO3/ncounter, stepNumber);// approx phase freq of PLL
			}
			if (step.LO3 > 2200) {
				msa::getInstance().currentInterface->errorOcurred(msadev, QString("LO3 will be above 2200MHz for step %1").arg(stepNumber), true, false);
				error = true;
				fatalError = true;
			} else if (step.LO3 < 950) {
				msa::getInstance().currentInterface->errorOcurred(msadev, QString("LO3 will be below 950MHz for step %1").arg(stepNumber), true, false);
				error = true;
				fatalError = true;
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return ncounter;
}

bool deviceParser::getPLLinverted(msa::scanConfig config)
{
	switch (msadev) {
	case msa::PLL1:
		return config.PLL1phasepolarity_inverted;
	case msa::PLL2:
		return config.PLL2phasepolarity_inverted;
	case msa::PLL3:
		return config.PLL3phasepolarity_inverted;
	default:
		return false;
	}
}

quint32 deviceParser::parseDDSOutput(msa::scanConfig configuration, quint32 stepNumber, bool &error, bool &fatalError)
{
	error = false;
	fatalError = false;
	double ddsoutput = 0;
	double fullbase = 0;
	double base = 0;
	switch (msadev) {
	case msa::DDS1:
		switch (hwdev) {
		case hardwareDevice::AD9850:
			if (configuration.forcedDDS1.isForced) {
				fullbase = (configuration.forcedDDS1.outputFreq * std::pow(2, 32) / configuration.forcedDDS1.oscFreq);
				base = std::round(fullbase);
				ddsoutput = base*configuration.forcedDDS1.oscFreq/std::pow(2, 32);
				(dynamic_cast<genericDDS *>(msa::getInstance().currentHardwareDevices.value(msa::DDS1)))->setDDSOutput(ddsoutput, stepNumber);
				if(stepNumber == 1)
					myDebug() << "Base for DDS1" << base;
			} else {
				ddsoutput = ((genericPLL *)msa::getInstance().currentHardwareDevices.value(msa::PLL1))->getPFD(stepNumber)
							* ((genericPLL *)msa::getInstance().currentHardwareDevices.value(msa::PLL1))->getRCounter();
				fullbase = (ddsoutput*std::pow(2, 32)/configuration.masterOscilatorFrequency);
				base = std::round(fullbase);
				ddsoutput = base*msa::getInstance().currentScan.configuration.masterOscilatorFrequency/std::pow(2, 32);
				(dynamic_cast<genericDDS *>(msa::getInstance().currentHardwareDevices.value(msa::DDS1)))->setDDSOutput(ddsoutput, stepNumber);
				////qDebug() << qSetRealNumberPrecision( 10 ) << "target freq"<< scan.steps[step].frequency << "ddsoutput" << ddsoutput << "VCO" <<getVcoFrequency(ddsoutput) << "DIFF" <<10.7 - (1024 - getVcoFrequency(ddsoutput));
				if (qAbs(ddsoutput - configuration.appxdds1) > (configuration.dds1Filterbandwidth / 2)) {
					error = true;
					fatalError = true;
					msa::getInstance().currentInterface->errorOcurred(msadev, QString(
																		  "DDS1 output would be outside its output filter bandwidth for step %1").arg(stepNumber), true, false);
				}
				if (msa::getInstance().currentInterface->getDebugLevel() > 2)
					myDebug() << "DD1 step" << stepNumber << " ddsoutput=" << ddsoutput << " base:" << base << " fullbase:"<<fullbase;
			}
			break;
		default:
			break;
		}
		break;
	case msa::DDS3:
		switch (hwdev) {
		case hardwareDevice::AD9850:
			if (configuration.forcedDDS3.isForced) {
				fullbase = (configuration.forcedDDS3.outputFreq * std::pow(2, 32) / configuration.forcedDDS3.oscFreq);
				base = std::round(fullbase);
				ddsoutput = base*configuration.forcedDDS3.oscFreq/std::pow(2, 32);     // 117c19
				((genericDDS *)msa::getInstance().currentHardwareDevices.value(msa::DDS3))->setDDSOutput(ddsoutput, stepNumber);
			} else {
				ddsoutput = ((genericPLL *)msa::getInstance().currentHardwareDevices.value(msa::PLL3))->getPFD(stepNumber)
							* ((genericPLL *)msa::getInstance().currentHardwareDevices.value(msa::PLL3))->getRCounter();
				fullbase = (ddsoutput*std::pow(2, 32)/configuration.masterOscilatorFrequency); // decimal number, including fraction
				base = std::round(fullbase);
				ddsoutput = base*msa::getInstance().currentScan.configuration.masterOscilatorFrequency/std::pow(2, 32); // 117c19
				((genericDDS *)msa::getInstance().currentHardwareDevices.value(msa::DDS3))->setDDSOutput(ddsoutput, stepNumber);
				////qDebug() << qSetRealNumberPrecision( 10 ) << "target freq"<< scan.steps[step].frequency << "ddsoutput" << ddsoutput << "VCO" <<getVcoFrequency(ddsoutput) << "DIFF" <<10.7 - (1024 - getVcoFrequency(ddsoutput));
				if ((ddsoutput- configuration.appxdds3) > (configuration.dds3Filterbandwidth / 2)) {
					error = true;
					fatalError = true;
					msa::getInstance().currentInterface->errorOcurred(msadev, QString(
																		  "DDS3 output would be outside its output filter bandwidth for step %1").arg(stepNumber), true, false);
				}
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return static_cast<quint32>(base);
}
