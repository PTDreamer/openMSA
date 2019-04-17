/**
 ******************************************************************************
 *
 * @file       comprotocol.h
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
#define SEND_TIMEOUT 3000
class ComProtocol : public QObject
{
	Q_OBJECT
public:
	typedef enum {DUAL_DAC, MAG_DAC, PH_DAC, DEBUG_VALUES, DEBUG_SETUP, SCAN_SETUP, SCAN_CONFIG, ERROR_INFO, FINAL_FILTER} messageType;
	typedef enum {MESSAGE_REQUEST, MESSAGE_SEND, MESSAGE_SEND_REQUEST_ACK, ACK} messageCommandType;
	typedef enum {SA, SA_TG, SA_SG,  VNA_Trans, VNA_Rec, SNA} scanType_t;
	typedef struct {
		uint32_t step;
		double mag;
		double phase;
	} msg_dual_dac;
	typedef struct {
		uint32_t step;
		uint32_t phase;
	} msg_ph_dac;
	typedef struct {
		char* name[10];
		double center_frequency;
		double bandwidth;
	} msg_final_filter;
	typedef struct {
		uint32_t step;
		uint32_t mag;
	} msg_mag_dac;
	typedef struct {
		double start;
		double stop;
		double step_freq;
		quint32 start_multi;
		quint32 stop_multi;
		quint32 step_freq_multi;
		double center_freq;
		quint32 center_freq_multi;
		double span_freq;
		quint32 span_freq_multi;
		quint32 steps_number;
		scanType_t scanType;
		bool isStepInSteps;
		bool stepModeAuto;
		bool isInvertedScan;
		int band;
		bool TGreversed;
		double TGoffset; //tracking generator offset
		quint32 TGoffset_multi;
		double SGout; //signal generator output frequency
		quint32 SGout_multi;
	} msg_scan_config;
	typedef struct {
		char text[sizeof (msg_scan_config) - sizeof (bool)];
		bool isCritical;
	} msg_error_info;

	typedef struct {
		QByteArray data;
		quint16 size;
		QTimer *timer;
		uint retries;
	} messageBackup;
	QHash<quint32, messageBackup> messagesBackup;
	QHash<messageType, unsigned long> messageSize;
	QByteArray messageSendBuffer;
	explicit ComProtocol(QObject *parent, int debugLevel);

	bool startServer();
	bool startServer(quint16 port);
	quint16 prepareMessage(messageType type, messageCommandType command, void *data, quint32 &msgNumber);
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
	void ackedReceived(messageType, QByteArray);
	void serverConnected();
    void clientConnected();
	void errorOcorred(QString, QString);
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
	void handleAck(messageType, QByteArray);
private slots:
	void newConnection();
	void bytesWritten(qint64);
	void processReceivedMessage();
	void clientDisconnected();
	void retrySendMessage();
};

#endif // COMPROTOCOL_H
