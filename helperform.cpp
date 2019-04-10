/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2019
 * @brief      helperform.cpp file
 * @see        The GNU Public License (GPL) Version 3
 * @defgroup   HelperForm
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
#include "helperform.h"
#include "ui_helperform.h"
#include <QDateTime>

HelperForm::HelperForm(QWidget *parent) :
	QWidget(parent),
	ui(new Ui::HelperForm)
{
	ui->setupUi(this);
	ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

HelperForm::~HelperForm()
{
	delete ui;
}

void HelperForm::showMessage(int type, QString title, QString text, int duration)
{
	QString s;
	QColor c;
	switch (type) {
	case 0:
		s = "Info";
		c = Qt::white;
		break;
	case 1:
		s = "Warning";
		c = Qt::yellow;
		break;
	case 2:
		s = "Error";
		c = Qt::red;
		break;
	}
	ui->tableWidget->insertRow(ui->tableWidget->rowCount());
	ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0,  new QTableWidgetItem(s));
	ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 1,  new QTableWidgetItem(title + " " + text));
	ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 2,  new QTableWidgetItem(QDateTime::currentDateTime().toString()));
	ui->tableWidget->item(ui->tableWidget->rowCount()-1, 0)->setBackgroundColor(c);
	ui->tableWidget->item(ui->tableWidget->rowCount()-1, 1)->setBackgroundColor(c);
	ui->tableWidget->item(ui->tableWidget->rowCount()-1, 2)->setBackgroundColor(c);

}
