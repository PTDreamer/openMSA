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
	enum interface_types {USB, SIMULATOR};
	interface(QObject *parent);
	~interface();
	typedef enum {status_halted, status_paused, status_scanning} status;
public slots:
	void commandNextStep();
	void commandPreviousStep();
	void pauseScan();
	void resumeScan();
	virtual bool isScanning() = 0;
	void autoScan();
	void cancelScan();
	void setStatus(status stat);
public:
	status getCurrentStatus() { return  currentStatus;}
	virtual void on_commandNextStep() = 0;
	virtual void on_commandPreviousStep() = 0;
	virtual bool getIsConnected() const = 0;
	virtual bool init(int debugLevel) = 0;
	virtual void setWriteReadDelay_us(unsigned long value) = 0;
	virtual bool initScan();
	void setScanConfiguration(msa::scanConfig configuration);
	virtual void hardwareInit();
	void errorOcurred(msa::MSAdevice dev, QString text, bool critical, bool sendToGUI);
	int getDebugLevel() const;
	void setDebugLevel(int value);
	virtual interface_types type() = 0;
signals:
	void dataReady(quint32 step, quint32 magnitude, quint32 phase);
	void connected();
	void disconnected();
	void errorTriggered(QString, bool, bool);
protected:
	virtual void on_autoscan() = 0;
	virtual void on_cancelscan() = 0;
	virtual void on_pausescan() = 0;
	virtual void on_resumescan() = 0;

	quint32 currentStep;
	quint32 lastCommandedStep;
	quint32 numberOfSteps;
	int debugLevel;
	status currentStatus;
private:
};

#endif // INTERFACE_H
