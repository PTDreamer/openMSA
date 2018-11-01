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

QHash<hardwareDevice::MSAdevice, hardwareDevice*> deviceParser::deviceList;

const QHash<hardwareDevice::MSAdevice, hardwareDevice*> deviceParser::getDeviceList() {
	return deviceParser::deviceList;
}

deviceParser::~deviceParser()
{
	deviceList.remove(msadev);
}

deviceParser::deviceParser(hardwareDevice::MSAdevice dev, hardwareDevice *parent):QObject(parent),msadev(dev), device(parent)
{
	lmx2326 *l = dynamic_cast<lmx2326*>(parent);
	ad9850 *a = dynamic_cast<ad9850*>(parent);
	genericADC *adc = dynamic_cast<genericADC*>(parent);
	if(l) {
		hwdev = hardwareDevice::LMX2326;
		deviceList.insert(dev, dynamic_cast<genericPLL*>(parent));
	}
	else if(a) {
		hwdev = hardwareDevice::AD9850;
		deviceList.insert(dev, a);
	}
	else if(adc) {
		hwdev = adc->getHardwareType();
	}
}

double deviceParser::parsePLLRCounter(hardwareDevice::scanConfig config)
{
	double ret = -1;
	switch (msadev) {
	case hardwareDevice::PLL1:
		switch (hwdev) {
		case hardwareDevice::LMX2326:
			ret = round(config.appxdds1/config.PLL1phasefreq);
			break;
		default:
			break;
		}
		break;
	case hardwareDevice::PLL2:
		switch (hwdev) {
		case hardwareDevice::LMX2326:
			ret = round(config.masterOscilatorFrequency / config.PLL2phasefreq);
			break;
		default:
			break;
		}
		break;
	case hardwareDevice::PLL3:
		switch (hwdev) {
		case hardwareDevice::LMX2326:
			ret = round(config.appxdds3/config.PLL3phasefreq);
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

double deviceParser::parsePLLNCounter(hardwareDevice::scanConfig configuration, hardwareDevice::scanStep &step, int stepNumber)
{
	double ncounter = 0;
	double ncount = 0;
	genericPLL *lmx = NULL;
	switch (msadev) {
	case hardwareDevice::PLL1:
		if(stepNumber == (int)HW_INIT_STEP)
			return -1;
		switch (hwdev) {
		case hardwareDevice::LMX2326:
			lmx = (genericPLL*)deviceList.value(msadev);
			step.LO1 = configuration.baseFrequency + step.translatedFrequency + configuration.LO2 - configuration.finalFilterFrequency;
			ncount = step.LO1/(configuration.appxdds1/ lmx->getRCounter()); //approximates the Ncounter for PLL
			ncounter = (int)round(ncount); //approximates the ncounter for PLL
			lmx->setPFD(step.LO1/ncounter, stepNumber);//approx phase freq of PLL
			break;
		default:
			break;
		}
		break;
	case hardwareDevice::PLL2:
		switch (hwdev) {
		case hardwareDevice::LMX2326:
			lmx = (genericPLL*)deviceList.value(msadev);
			ncount = configuration.LO2/(configuration.masterOscilatorFrequency / lmx->getRCounter()); //approximates the Ncounter for PLL
			ncounter = (int)round(ncount); //approximates the ncounter for PLL
			lmx->setPFD(configuration.LO2/ncounter, stepNumber);//approx phase freq of PLL
			break;
		default:
			break;
		}
		break;
	case hardwareDevice::PLL3:
		step.LO3 = 0;
		if(stepNumber == (int)HW_INIT_STEP)
			return -1;
		switch (hwdev) {
		case hardwareDevice::LMX2326:
			lmx = (genericPLL*)deviceList.value(msadev);
			if(configuration.scanType == hardwareDevice::SA_TG) {
				if(!configuration.TGreversed) {
					if(step.band == 3) {
						step.LO3 = step.realFrequency + configuration.TGoffset - configuration.LO2;
					}
					else {
						step.LO3 = configuration.LO2 + step.translatedFrequency + configuration.TGoffset;
					}
				}
				else {
					double reversedFrequency;
					int reversedIndex = hardwareDevice::currentScan.steps.size() - stepNumber - 1;
					if(step.band == 1)
						reversedFrequency = hardwareDevice::currentScan.steps.value(reversedIndex).realFrequency;
					else
						reversedFrequency = hardwareDevice::currentScan.steps.value(reversedIndex).translatedFrequency;
					if(step.band == 3)
						step.LO3 = hardwareDevice::currentScan.steps.value(reversedIndex).realFrequency + configuration.TGoffset - configuration.LO2;
					else
						step.LO3 = configuration.LO2 + reversedFrequency + configuration.TGoffset;
				}
				step.LO3 = configuration.LO2 - configuration.finalFilterFrequency - configuration.TGoffset;
			}
			else if(configuration.scanType == hardwareDevice::SA_SG) {
				if(configuration.SGout <= configuration.LO2)
					step.LO3 = configuration.SGout + configuration.LO2;
				else if(configuration.SGout > (2*configuration.LO2))
					step.LO3 = configuration.SGout - configuration.LO2;
				else
					step.LO3 = configuration.SGout;
			}
			if(lmx) {
			ncount = step.LO3/(configuration.appxdds3/ lmx->getRCounter()); //approximates the Ncounter for PLL
			ncounter = (int)round(ncount); //approximates the ncounter for PLL
			lmx->setPFD(step.LO3/ncounter, stepNumber);//approx phase freq of PLL
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

quint32 deviceParser::parseDDSOutput(hardwareDevice::scanConfig configuration, int stepNumber, bool &error)
{
	error = false;
	double ddsoutput = 0;
	double fullbase = 0;
	double base = 0;
	switch (msadev) {
	case hardwareDevice::DDS1:
		switch (hwdev) {
		case hardwareDevice::AD9850:
			//	qDebug() << qSetRealNumberPrecision( 10 ) << "acounter" << Acounter << "bcounter" << Bcounter << "ncounter" << ncounter << "rcounter" << rcounter << "frequency" << scan.steps.value(step).frequency<<"LO1"<<scan.steps[step].LO1;
				ddsoutput = ((genericPLL*)deviceList.value(hardwareDevice::PLL1))->getPFD(stepNumber) * ((genericPLL*)deviceList.value(hardwareDevice::PLL1))->getRCounter();
				//if ddsoutput >= ddsclock/2 then  error
				//the formula for the frequency output of the DDS(AD9850, 9851, or any 32 bit DDS) is:
				//ddsoutput = base*ddsclock/2^32, where "base" is the decimal equivalent of command words
				//to find "base", first:
				fullbase=(ddsoutput*pow(2,32)/configuration.masterOscilatorFrequency); //decimal number, including fraction
				//then, round off fraction to the nearest whole number
				base = round(fullbase);
				//When entering this routine, ddsoutput was approximate. Now, the exact ddsoutput can be determined by:
				ddsoutput = base*64/pow(2,32);  //117c19
				((genericDDS*)deviceList.value(hardwareDevice::DDS1))->setDDSOutput(ddsoutput, stepNumber);
			//	qDebug() << qSetRealNumberPrecision( 10 ) << "target freq"<< scan.steps[step].frequency << "ddsoutput" << ddsoutput << "VCO" <<getVcoFrequency(ddsoutput) << "DIFF" <<10.7 - (1024 - getVcoFrequency(ddsoutput));
				if((ddsoutput - configuration.appxdds1) > (configuration.dds1Filterbandwidth / 2))
						error = true;
			break;
		default:
			break;
		}
		break;
	case hardwareDevice::DDS3:
		switch (hwdev) {
		case hardwareDevice::AD9850:
			//	qDebug() << qSetRealNumberPrecision( 10 ) << "acounter" << Acounter << "bcounter" << Bcounter << "ncounter" << ncounter << "rcounter" << rcounter << "frequency" << scan.steps.value(step).frequency<<"LO1"<<scan.steps[step].LO1;
				ddsoutput = ((genericPLL*)deviceList.value(hardwareDevice::PLL3))->getPFD(stepNumber) * ((genericPLL*)deviceList.value(hardwareDevice::PLL3))->getRCounter();
				//if ddsoutput >= ddsclock/2 then  error
				//the formula for the frequency output of the DDS(AD9850, 9851, or any 32 bit DDS) is:
				//ddsoutput = base*ddsclock/2^32, where "base" is the decimal equivalent of command words
				//to find "base", first:
				fullbase=(ddsoutput*pow(2,32)/configuration.masterOscilatorFrequency); //decimal number, including fraction
				//then, round off fraction to the nearest whole number
				base = round(fullbase);
				//When entering this routine, ddsoutput was approximate. Now, the exact ddsoutput can be determined by:
				ddsoutput = base*64/pow(2,32);  //117c19
				((genericDDS*)deviceList.value(hardwareDevice::DDS3))->setDDSOutput(ddsoutput, stepNumber);
			//	qDebug() << qSetRealNumberPrecision( 10 ) << "target freq"<< scan.steps[step].frequency << "ddsoutput" << ddsoutput << "VCO" <<getVcoFrequency(ddsoutput) << "DIFF" <<10.7 - (1024 - getVcoFrequency(ddsoutput));
				if((ddsoutput- configuration.appxdds3) > (configuration.dds3Filterbandwidth / 2))
						error = true;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}	
	return base;
}
