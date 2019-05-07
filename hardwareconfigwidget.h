/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2019
 * @brief      hardwareconfigwidget.h file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   hardwareConfigWidget
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
#ifndef HARDWARECONFIGWIDGET_H
#define HARDWARECONFIGWIDGET_H

#include <QWidget>
#include <QSettings>
#include "calparser.h"
#include <hardware/msa.h>
#include "hardware/controllers/interface.h"

namespace Ui {
class hardwareConfigWidget;
}

class hardwareConfigWidget : public QWidget
{
	Q_OBJECT

public:
	typedef struct {
		quint16 serverPort;
		int debugLevel;
		unsigned int readWriteDelay;
		interface::interface_types currentInterfaceType;
		QHash<msa::MSAdevice, hardwareDevice::HWdevice> devices;
	}appSettings_t;

	explicit hardwareConfigWidget(QWidget *parent = nullptr);
	~hardwareConfigWidget();

	msa::scanConfig getConfig() const;
	void setConfig(const msa::scanConfig &value);

	appSettings_t getAppSettings() const;
	void setAppSettings(const appSettings_t &value);

	bool getSaveSettingsOnExit() const;
	void setSaveSettingsOnExit(bool value);

private:
	Ui::hardwareConfigWidget *ui;
	QSettings *settings;
	msa::scanConfig config;
	appSettings_t appSettings;
	bool saveSettingsOnExit;
	void loadCalibrationFiles();
	calParser m_calParser;
public slots:
	void loadSavedSettings(bool loadDefaults = false);
	void saveSettings();
	void setSettingsFromGui();
	void loadSettingsToGui();
private slots:
	void on_pb_add_video_filter_clicked();
	void on_pb_delete_video_filter_clicked();
	void on_pb_add_resolution_filter_clicked();
	void on_pb_delete_resolution_filter_clicked();
	void on_pb_load_defaults_clicked();
	void on_pb_save_and_exit_clicked();
	void on_pb_apply_clicked();
	void on_pb_cancel_clicked();
	void on_pb_CommandDDS1_clicked();

	void on_pbResetDDS1_clicked();

	void on_pb_CommandDDS3_clicked();

	void on_pbResetDDS3_clicked();

	void on_resolution_filters_table_currentCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

signals:
	void triggerMessage(int type, QString title, QString text, int duration);
	void requiresReinit();
	void requiresHwReinit();
};

#endif // HARDWARECONFIGWIDGET_H
