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

deviceParser::~deviceParser()
{
}

deviceParser::deviceParser(msa::MSAdevice dev, hardwareDevice *parent):QObject(parent),msadev(dev), device(parent)
{
	lmx2326 *l = dynamic_cast<lmx2326*>(parent);
	ad9850 *a = dynamic_cast<ad9850*>(parent);
	genericADC *adc = dynamic_cast<genericADC*>(parent);
	if(l) {
		hwdev = hardwareDevice::LMX2326;
	}
	else if(a) {
		hwdev = hardwareDevice::AD9850;
	}
	else if(adc) {
		hwdev = adc->getHardwareType();
	}
}

double deviceParser::parsePLLRCounter(msa::scanConfig config)
{
	double ret = -1;
	switch (msadev) {
	case msa::PLL1:
		switch (hwdev) {
		case hardwareDevice::LMX2326:
			ret = round(config.appxdds1/config.PLL1phasefreq);
			break;
		default:
			break;
		}
		break;
	case msa::PLL2:
		switch (hwdev) {
		case hardwareDevice::LMX2326:
			ret = round(config.masterOscilatorFrequency / config.PLL2phasefreq);
			break;
		default:
			break;
		}
		break;
	case msa::PLL3:
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
#define my//qDebug() //qDebug() << fixed << qSetRealNumberPrecision(12)

double deviceParser::parsePLLNCounter(msa::scanConfig configuration, msa::scanStep &step, quint32 stepNumber, bool &error)
{
	error = false;
	double ncounter = 0;
	double ncount = 0;
	genericPLL *lmx = nullptr;
	switch (msadev) {
	case msa::PLL1:
		if(stepNumber == quint32(HW_INIT_STEP))
			return -1;
		switch (hwdev) {
		case hardwareDevice::LMX2326:
			lmx = dynamic_cast<genericPLL*>(msa::getInstance().currentHardwareDevices.value(msadev));
			step.LO1 = configuration.baseFrequency + step.translatedFrequency + configuration.LO2 - configuration.finalFilterFrequency;
			if(step.LO1 > 2200) {
				msa::getInstance().currentInterface->errorOcurred(msadev, QString("LO1 will be above 2200MHz for step %1").arg(stepNumber));
				error = true;
			}
			else if(step.LO1 < 950) {
				msa::getInstance().currentInterface->errorOcurred(msadev, QString("LO1 will be below 950MHz for step %1").arg(stepNumber));
				error = true;
			}
			//my//qDebug() << "LO1 step:"<< stepNumber << step.LO1 <<"="<< configuration.baseFrequency <<"+"<< step.translatedFrequency <<"+"<< configuration.LO2 <<"-"<< configuration.finalFilterFrequency;
			ncount = step.LO1/(configuration.appxdds1/ lmx->getRCounter()); //approximates the Ncounter for PLL
			ncounter = int(round(ncount)); //approximates the ncounter for PLL
			lmx->setPFD(step.LO1/ncounter, stepNumber);//approx phase freq of PLL
			//my//qDebug()<< "PLL1 "<< "step:"<<stepNumber << "PFD:"<<step.LO1/ncounter;
			break;
		default:
			break;
		}
		break;
	case msa::PLL2:
		switch (hwdev) {
		case hardwareDevice::LMX2326:
			lmx = dynamic_cast<genericPLL*>(msa::getInstance().currentHardwareDevices.value(msadev));
			ncount = configuration.LO2/(configuration.masterOscilatorFrequency / lmx->getRCounter()); //approximates the Ncounter for PLL
			ncounter = int(round(ncount)); //approximates the ncounter for PLL
			lmx->setPFD(configuration.LO2/ncounter, stepNumber);//approx phase freq of PLL
			break;
		default:
			break;
		}
		break;
	case msa::PLL3:
		step.LO3 = 0;
		if(stepNumber == quint32(HW_INIT_STEP))
			return -1;
		switch (hwdev) {
		case hardwareDevice::LMX2326:
			lmx = dynamic_cast<genericPLL*>(msa::getInstance().currentHardwareDevices.value(msadev));
			if(configuration.scanType == msa::SA_TG) {
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
					quint32 reversedIndex = uint(msa::getInstance().currentScan.steps.size()) - stepNumber - 1;
					if(step.band == 1)
						reversedFrequency = msa::getInstance().currentScan.steps.value(reversedIndex).realFrequency;
					else
						reversedFrequency = msa::getInstance().currentScan.steps.value(reversedIndex).translatedFrequency;
					if(step.band == 3)
						step.LO3 = msa::getInstance().currentScan.steps.value(reversedIndex).realFrequency + configuration.TGoffset - configuration.LO2;
					else
						step.LO3 = configuration.LO2 + reversedFrequency + configuration.TGoffset;
				}
				step.LO3 = configuration.LO2 - configuration.finalFilterFrequency - configuration.TGoffset;
			}
			else if(configuration.scanType == msa::SA_SG) {
				if(configuration.SGout <= configuration.LO2)
					step.LO3 = configuration.SGout + configuration.LO2;
				else if(configuration.SGout > (2*configuration.LO2))
					step.LO3 = configuration.SGout - configuration.LO2;
				else
					step.LO3 = configuration.SGout;
			}
			if(lmx) {
				ncount = step.LO3/(configuration.appxdds3/ lmx->getRCounter()); //approximates the Ncounter for PLL
				ncounter = int(round(ncount)); //approximates the ncounter for PLL
				lmx->setPFD(step.LO3/ncounter, stepNumber);//approx phase freq of PLL
			}
			if(step.LO1 > 2200) {
				msa::getInstance().currentInterface->errorOcurred(msadev, QString("LO1 will be above 2200MHz for step %1").arg(stepNumber));
				error = true;
			}
			else if(step.LO1 < 950) {
				msa::getInstance().currentInterface->errorOcurred(msadev, QString("LO1 will be below 950MHz for step %1").arg(stepNumber));
				error = true;
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

quint32 deviceParser::parseDDSOutput(msa::scanConfig configuration, quint32 stepNumber, bool &error)
{
	error = false;
	double ddsoutput = 0;
	double fullbase = 0;
	double base = 0;
	switch (msadev) {
	case msa::DDS1:
		switch (hwdev) {
		case hardwareDevice::AD9850:
			//	//qDebug() << qSetRealNumberPrecision( 10 ) << "acounter" << Acounter << "bcounter" << Bcounter << "ncounter" << ncounter << "rcounter" << rcounter << "frequency" << scan.steps.value(step).frequency<<"LO1"<<scan.steps[step].LO1;
				ddsoutput = ((genericPLL*)msa::getInstance().currentHardwareDevices.value(msa::PLL1))->getPFD(stepNumber) * ((genericPLL*)msa::getInstance().currentHardwareDevices.value(msa::PLL1))->getRCounter();
				//if ddsoutput >= ddsclock/2 then  error
				//the formula for the frequency output of the DDS(AD9850, 9851, or any 32 bit DDS) is:
				//ddsoutput = base*ddsclock/2^32, where "base" is the decimal equivalent of command words
				//to find "base", first:
				fullbase=(ddsoutput*pow(2,32)/configuration.masterOscilatorFrequency); //decimal number, including fraction
				//qDebug()<< "ddsoutput" << ddsoutput <<"=pfd:"<< ((genericPLL*)msa::getInstance().currentHardwareDevices.value(msa::PLL1))->getPFD(stepNumber) << "* rcounter:"<< ((genericPLL*)msa::getInstance().currentHardwareDevices.value(msa::PLL1))->getRCounter() << "  fullbase:"<< fullbase;
				//then, round off fraction to the nearest whole number
				base = round(fullbase);
				//When entering this routine, ddsoutput was approximate. Now, the exact ddsoutput can be determined by:
				ddsoutput = base*msa::getInstance().currentScan.configuration.masterOscilatorFrequency/pow(2,32);  //117c19
				(dynamic_cast<genericDDS*>(msa::getInstance().currentHardwareDevices.value(msa::DDS1)))->setDDSOutput(ddsoutput, stepNumber);
			//	//qDebug() << qSetRealNumberPrecision( 10 ) << "target freq"<< scan.steps[step].frequency << "ddsoutput" << ddsoutput << "VCO" <<getVcoFrequency(ddsoutput) << "DIFF" <<10.7 - (1024 - getVcoFrequency(ddsoutput));
                if(qAbs(ddsoutput - configuration.appxdds1) > (configuration.dds1Filterbandwidth / 2)) {
						error = true;
						msa::getInstance().currentInterface->errorOcurred(msadev, QString(
					"DDS1 output would be outside its output filter bandwidth for step %1").arg(stepNumber));
				}
			break;
		default:
			break;
		}
		break;
	case msa::DDS3:
		switch (hwdev) {
		case hardwareDevice::AD9850:
			//	//qDebug() << qSetRealNumberPrecision( 10 ) << "acounter" << Acounter << "bcounter" << Bcounter << "ncounter" << ncounter << "rcounter" << rcounter << "frequency" << scan.steps.value(step).frequency<<"LO1"<<scan.steps[step].LO1;
				ddsoutput = ((genericPLL*)msa::getInstance().currentHardwareDevices.value(msa::PLL3))->getPFD(stepNumber) * ((genericPLL*)msa::getInstance().currentHardwareDevices.value(msa::PLL3))->getRCounter();
				//if ddsoutput >= ddsclock/2 then  error
				//the formula for the frequency output of the DDS(AD9850, 9851, or any 32 bit DDS) is:
				//ddsoutput = base*ddsclock/2^32, where "base" is the decimal equivalent of command words
				//to find "base", first:
				fullbase=(ddsoutput*pow(2,32)/configuration.masterOscilatorFrequency); //decimal number, including fraction
				//then, round off fraction to the nearest whole number
				base = round(fullbase);
				//When entering this routine, ddsoutput was approximate. Now, the exact ddsoutput can be determined by:
				ddsoutput = base*msa::getInstance().currentScan.configuration.masterOscilatorFrequency/pow(2,32);  //117c19
				((genericDDS*)msa::getInstance().currentHardwareDevices.value(msa::DDS3))->setDDSOutput(ddsoutput, stepNumber);
			//	//qDebug() << qSetRealNumberPrecision( 10 ) << "target freq"<< scan.steps[step].frequency << "ddsoutput" << ddsoutput << "VCO" <<getVcoFrequency(ddsoutput) << "DIFF" <<10.7 - (1024 - getVcoFrequency(ddsoutput));
				if((ddsoutput- configuration.appxdds3) > (configuration.dds3Filterbandwidth / 2)) {
						error = true;
						msa::getInstance().currentInterface->errorOcurred(msadev, QString(
					"DDS3 output would be outside its output filter bandwidth for step %1").arg(stepNumber));
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
