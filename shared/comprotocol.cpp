/**
 ******************************************************************************
 *
 * @file       comprotocol.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2019
 * @brief      comprotocol.cpp file
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
#include "comprotocol.h"
#include <QDebug>

ComProtocol::ComProtocol(QObject *parent, int debugLevel) : QObject(parent),
	server(nullptr),
	serverPort(1234),
	socket(nullptr),
	bytesWaitingToBeSent(0),
	msgNumber(0),
	debugLevel(debugLevel)
{
	messageSize.insert(DUAL_DAC, sizeof(msg_dual_dac));
	messageSize.insert(PH_DAC, sizeof(msg_ph_dac));
	messageSize.insert(MAG_DAC, sizeof(msg_mag_dac));
	messageSize.insert(SCAN_CONFIG, sizeof(msg_scan_config));
	messageSize.insert(ERROR_INFO, sizeof(msg_error_info));
	QList<unsigned long> sizes = messageSize.values();
	double max = *std::max_element(sizes.begin(), sizes.end());
	startOfData = 3 + sizeof(quint32);
	messageSendBuffer.resize(int(max + startOfData + sizeof(quint16) + 2));
	messageSendBuffer[0] = SYNC_BYTE;

	connect(&clientReconnectTimer, SIGNAL(timeout()), this, SLOT(connectToServer()));
}

bool ComProtocol::startServer()
{
	if (server) {
		server->close();
		server->deleteLater();
	}
	server = new QTcpServer(this);

	connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));

	if (!server->listen(QHostAddress::Any, serverPort)) {
		if (debugLevel > 0)
			qDebug() << "Server could not start!";
		return false;
	} else {
		if (debugLevel > 0)
			qDebug() << "Server started!";
		return true;
	}
}

bool ComProtocol::startServer(quint16 port)
{
	serverPort = port;
	return startServer();
}

bool ComProtocol::connectToServer()
{
	if (!socket) {
		socket = new QTcpSocket(this);
		connect(socket, SIGNAL(bytesWritten(qint64)), this, SLOT(bytesWritten(qint64)));
		connect(socket, SIGNAL(disconnected()), this, SLOT(clientDisconnected()));
		connect(socket, SIGNAL(readyRead()), this, SLOT(processReceivedMessage()));
	}
	if (socket->state() == QTcpSocket::ConnectedState)
		socket->disconnectFromHost();

	qDebug() << "Connecting,..";

	socket->connectToHost(serverAddress, serverPort);
	bool connected = socket->waitForConnected(1000);
	if (connected) {
		clientReconnectTimer.stop();
        emit clientConnected();
	} else {
		if (autoClientReconnection)
			clientReconnectTimer.start(1000);
	}
	qDebug() << "Client connected:" << connected;
	return connected;
}

void ComProtocol::sendMessage(messageType type, messageCommandType command, void *data)
{
	bytesWaitingToBeSentLock.lock();
	quint16 size = prepareMessage(type, command, data);
	if (socket && socket->state() == QTcpSocket::ConnectedState) {
		quint16 msgSize = 0;
		if (messageSize.contains(type))
			msgSize = quint16(messageSize.value(type));
		bytesWaitingToBeSent += size;
		socket->write(messageSendBuffer.constData(), size);
	}
	if (debugLevel > 2)
		qDebug() << "bytesWaitingToBeSent:"<< bytesWaitingToBeSent;
	bytesWaitingToBeSentLock.unlock();
}

QString ComProtocol::getServerAddress() const
{
	return serverAddress;
}

void ComProtocol::setServerAddress(const QString &value)
{
	serverAddress = value;
}

bool ComProtocol::getAutoClientReconnection() const
{
	return autoClientReconnection;
}

void ComProtocol::setAutoClientReconnection(bool value)
{
    autoClientReconnection = value;
}

bool ComProtocol::isConnected()
{
	if (socket)
		return socket->state() == QTcpSocket::ConnectedState;
	else
		return false;
}

quint16 ComProtocol::prepareMessage(messageType type, messageCommandType command, void *data)
{
	messageSendBuffer[1] = type;
	messageSendBuffer[2] = command;
    quint16 msgSize = 0;
	if (messageSize.contains(type))
        msgSize = quint16(messageSize.value(type));
	memcpy((messageSendBuffer.data() + 3), &msgNumber, sizeof(quint32));
	++msgNumber;
	if (msgSize)
        memcpy((messageSendBuffer.data() + startOfData), data, msgSize);
	quint16 checksum = qChecksum(messageSendBuffer.constData() + 1, msgSize + 2 + sizeof(quint32));
	memcpy((messageSendBuffer.data() + startOfData + msgSize), (&checksum), sizeof(quint16));
	return startOfData + sizeof(quint16) + msgSize;
}

bool ComProtocol::unpackMessage(QByteArray rmessage, messageType &type, messageCommandType &command,
								quint32 &msgNumber, void *data)
{
	if (rmessage.at(0) != SYNC_BYTE) {
		qDebug() << "WTF";
		return false;
	}
    type = messageType(rmessage.at(1));
    command = messageCommandType(rmessage.at(2));
    quint16 msgSize = 0;
	if (messageSize.contains(type))
		msgSize = quint16(messageSize.value(type));
	memcpy(&msgNumber, rmessage.constData() + 3, sizeof(quint32));
	quint16 calcChecksum = qChecksum((rmessage.constData() + 1), msgSize + 2 + sizeof(quint32));
	quint16 receivedChecksum;
	memcpy(&receivedChecksum, (rmessage.constData() + startOfData + msgSize), sizeof(quint16));

	if (calcChecksum != receivedChecksum) {
		qDebug() << "wrong checksum";
		return false;
	}
	memcpy(data, (rmessage.constData() + startOfData), msgSize);
	return true;
}

quint16 ComProtocol::getServerPort() const
{
	return serverPort;
}

void ComProtocol::setServerPort(const quint16 &value)
{
	serverPort = value;
}

void ComProtocol::newConnection()
{
	qDebug() << "Server new connection";
	if (socket) {
		if (socket->state() == QTcpSocket::ConnectedState)
			socket->close();
		delete socket;
	}
	socket = server->nextPendingConnection();
	connect(socket, &QTcpSocket::bytesWritten, this, &ComProtocol::bytesWritten);
	connect(socket, &QTcpSocket::readyRead, this, &ComProtocol::processReceivedMessage);
	emit serverConnected();
}

void ComProtocol::bytesWritten(qint64 count)
{
	bytesWaitingToBeSentLock.lock();
	bytesWaitingToBeSent -= count;
	if (debugLevel > 2)
		qDebug() << "bytesWaiting" << bytesWaitingToBeSent;
	bytesWaitingToBeSentLock.unlock();
}

void ComProtocol::processReceivedMessage()
{
	typedef enum {
		LOOKING_FOR_SYNC, LOOKING_FOR_MSG_TYPE, LOOKING_FOR_MSG_CHECKSUM
	}status;
	static QByteArray receiveBuffer;
	static status currentStatus = status::LOOKING_FOR_SYNC;
	static messageType currentType;

	receiveBuffer.append(socket->readAll());
	bool repeat = true;
	while (repeat) {
		repeat = false;
		switch (currentStatus) {
		case status::LOOKING_FOR_SYNC:
			if (receiveBuffer.contains(SYNC_BYTE)) {
				receiveBuffer
					= receiveBuffer.right(receiveBuffer.length()
										  - receiveBuffer.indexOf(SYNC_BYTE));
				repeat = true;
				currentStatus = LOOKING_FOR_MSG_TYPE;
			}
			break;
		case status::LOOKING_FOR_MSG_TYPE:
			if (receiveBuffer.length() > 2) {
                currentType = messageType(receiveBuffer.at(1));
				if (messageSize.contains(currentType)) {
					repeat = true;
					currentStatus = LOOKING_FOR_MSG_CHECKSUM;
				} else {
					repeat = true;
					currentStatus = LOOKING_FOR_SYNC;
					receiveBuffer[0] = 0;
				}
			}
			break;
		case status::LOOKING_FOR_MSG_CHECKSUM:
			if (receiveBuffer.length()
				>= int(startOfData + messageSize.value(currentType) + sizeof(quint16))) {
				quint16 receivedChecksum;
				memcpy(&receivedChecksum,
					   (receiveBuffer.constData() + startOfData + messageSize.value(currentType)),
					   sizeof(quint16));
				quint16 calculateChecksum
					= qChecksum(receiveBuffer.constData() + 1,
								uint(messageSize.value(currentType) + 2 + sizeof(quint32)));
				if (receivedChecksum != calculateChecksum) {
					qDebug() << "Wrong checksum received";
					receiveBuffer[0] = 0;
					repeat = true;
					currentStatus = LOOKING_FOR_SYNC;
				} else {
					QByteArray array;
					array
						= receiveBuffer.left(int(startOfData + messageSize.value(currentType)
												 + sizeof(quint16)));
					emit packetReceived(currentType, array);
					receiveBuffer = receiveBuffer.right(receiveBuffer.length() - array.length());
					currentStatus = LOOKING_FOR_SYNC;
					repeat = true;
				}
			}
			break;
		}
	}
}

void ComProtocol::clientDisconnected()
{
	if (autoClientReconnection)
		clientReconnectTimer.start(1000);
}
