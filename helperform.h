/**
 ******************************************************************************
 *
 * @file       main.cpp
 * @author     Jose Barros (AKA PT_Dreamer) josemanuelbarros@gmail.com 2019
 * @brief      helperform.h file
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
#ifndef HELPERFORM_H
#define HELPERFORM_H

#include <QWidget>

namespace Ui {
class HelperForm;
}

class HelperForm : public QWidget
{
	Q_OBJECT

public:
	explicit HelperForm(QWidget *parent = nullptr);
	~HelperForm();

private:
	Ui::HelperForm *ui;
public slots:
	void showMessage(int type, QString title, QString text, int duration);
};

#endif // HELPERFORM_H
