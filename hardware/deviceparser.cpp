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

QHash<deviceParser::MSAdevice, hardwareDevice*> deviceParser::deviceList;

deviceParser::deviceParser(deviceParser::MSAdevice dev, hardwareDevice *parent):QObject(parent),msadev(dev), device(parent)
{
	lmx2326 *l = dynamic_cast<lmx2326*>(parent);
	ad9850 *a = dynamic_cast<ad9850*>(parent);
	if(l) {
		hwdev = LMX2326;
		deviceList.insert(dev, dynamic_cast<genericPLL*>(parent));
	}
	else if(a) {
		hwdev = AD9850;
		deviceList.insert(dev, a);
	}
}

double deviceParser::parsePLLRCounter(hardwareDevice::scanConfig config)
{
	double ret;
	switch (msadev) {
	case PLL1:
		switch (hwdev) {
		case LMX2326:
			ret = round(config.appxdds1/config.PLL1phasefreq);
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
	double ncounter;
	double ncount;
	genericPLL *lmx;
	switch (msadev) {
	case PLL1:
		switch (hwdev) {
		case LMX2326:
			lmx = (genericPLL*)deviceList.value(msadev);
			step.LO1 = configuration.baseFrequency + step.frequency + configuration.LO2 - configuration.finalFrequency;
			ncount = step.LO1/(configuration.appxdds1/ lmx->getRCounter()); //approximates the Ncounter for PLL
			ncounter = (int)round(ncount); //approximates the ncounter for PLL
			lmx->setPFD(step.LO1/ncounter, stepNumber);//approx phase freq of PLL
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

quint32 deviceParser::parseDDSOutput(hardwareDevice::scanConfig configuration, int stepNumber)
{
	double ddsoutput;
	double fullbase;
	double base;
	switch (msadev) {
	case DDS1:
		switch (hwdev) {
		case AD9850:
			//	qDebug() << qSetRealNumberPrecision( 10 ) << "acounter" << Acounter << "bcounter" << Bcounter << "ncounter" << ncounter << "rcounter" << rcounter << "frequency" << scan.steps.value(step).frequency<<"LO1"<<scan.steps[step].LO1;
				ddsoutput = ((genericPLL*)deviceList.value(PLL1))->getPFD(stepNumber) * ((genericPLL*)deviceList.value(PLL1))->getRCounter();
				//if ddsoutput >= ddsclock/2 then  error
				//the formula for the frequency output of the DDS(AD9850, 9851, or any 32 bit DDS) is:
				//ddsoutput = base*ddsclock/2^32, where "base" is the decimal equivalent of command words
				//to find "base", first:
				fullbase=(ddsoutput*pow(2,32)/configuration.masterOscilatorFrequency); //decimal number, including fraction
				//then, round off fraction to the nearest whole number
				base = round(fullbase);
				//When entering this routine, ddsoutput was approximate. Now, the exact ddsoutput can be determined by:
				ddsoutput = base*64/pow(2,32);  //117c19
				((genericDDS*)deviceList.value(DDS1))->setDDSOutput(ddsoutput, stepNumber);
			//	qDebug() << qSetRealNumberPrecision( 10 ) << "target freq"<< scan.steps[step].frequency << "ddsoutput" << ddsoutput << "VCO" <<getVcoFrequency(ddsoutput) << "DIFF" <<10.7 - (1024 - getVcoFrequency(ddsoutput));
		default:
			break;
		}
		break;
	default:
		break;
	}
	return base;
}
