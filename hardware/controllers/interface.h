/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2018
 * @brief      interface.h file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   interface
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
#ifndef INTERFACE_H
#define INTERFACE_H

#include <QThread>
#include "../hardwaredevice.h"
#include "../msa.h"

class interface: public QThread
{
	Q_OBJECT
public:
	interface(QObject *parent);
	~interface();
protected slots:
	virtual void commandNextStep() = 0;
	virtual void commandPreviousStep() = 0;
	virtual void autoScan() = 0;
	virtual void pauseScan() = 0;
	virtual void resumeScan() = 0;
	virtual bool isScanning() = 0;
public:
	virtual void initScan();
	void setScanConfiguration(msa::scanConfig configuration);
	virtual void hardwareInit();

signals:
	void dataReady(quint32 step, quint32 magnitude, quint32 phase);
	void connected();
	void disconnected();
protected:
	qint32 currentStep;
	qint32 lastCommandedStep;
	quint32 numberOfSteps;
private:
};

#endif // INTERFACE_H
