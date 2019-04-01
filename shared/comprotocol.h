/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2019
 * @brief      comprotocol.h file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   ComProtocol
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
#ifndef COMPROTOCOL_H
#define COMPROTOCOL_H

#include <QObject>
#include <QHash>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMutex>
#include <QTimer>

#define SYNC_BYTE 0x3D

class ComProtocol : public QObject
{
	Q_OBJECT
public:
	typedef enum {DUAL_DAC, MAG_DAC, PH_DAC, DEBUG_VALUES, DEBUG_SETUP, SCAN_SETUP, SCAN_CONFIG} messageType;
	typedef enum {MESSAGE_REQUEST, MESSAGE_SEND, MESSAGE_SEND_REQUEST_ACK, ACK} messageCommandType;
	typedef struct {
		uint32_t step;
		uint32_t mag;
		uint32_t phase;
	} msg_dual_dac;
	typedef struct {
		uint32_t step;
		uint32_t phase;
	} msg_ph_dac;
	typedef struct {
		uint32_t step;
		uint32_t mag;
	} msg_mag_dac;
	typedef struct {
		double start;
		double stop;
		double step_freq;
	} msg_scan_config;

	QHash<messageType, unsigned long> messageSize;
	QByteArray messageSendBuffer;
	explicit ComProtocol(QObject *parent, int debugLevel);

	bool startServer();
	bool startServer(quint16 port);
	void prepareMessage(messageType type, messageCommandType command, void *data);
	bool unpackMessage(QByteArray rmessage, messageType &type, messageCommandType &command, quint32 &msgNumber, void *data);
	quint16 getServerPort() const;
	void setServerPort(const quint16 &value);

	void sendMessage(messageType type, messageCommandType command, void *data);
	QString getServerAddress() const;
	void setServerAddress(const QString &value);

	bool getAutoClientReconnection() const;
	void setAutoClientReconnection(bool value);
    bool isConnected();
signals:
	void packetReceived(messageType, QByteArray);
	void serverConnected();
    void clientConnected();
private:
	quint8 startOfData;
	QTcpServer *server;
	quint16 serverPort;
	QString serverAddress;
	QTcpSocket *socket;
	qint64 bytesWaitingToBeSent;
	QMutex bytesWaitingToBeSentLock;
	bool autoClientReconnection;
	QTimer clientReconnectTimer;
	quint32 msgNumber;
	int debugLevel;
public slots:
	bool connectToServer();
private slots:
	void newConnection();
	void bytesWritten(qint64);
	void processReceivedMessage();
	void clientDisconnected();
};

#endif // COMPROTOCOL_H
