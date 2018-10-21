#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "hardware/lmx2326.h"
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

private:
	Ui::MainWindow *ui;
	lmx2326 lmx;
};

#endif // MAINWINDOW_H
