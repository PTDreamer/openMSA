#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "hardware/lmx2326.h"
#include "hardware/controllers/slimusb.h"
#include <QDebug>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();

private slots:
	void on_pushButton_clicked();
	void dataReady(int, double, double);
private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
