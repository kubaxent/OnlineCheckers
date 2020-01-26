#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QString prev;
    QString color;
    void resetBoard();
    QList<QWidget*> widgets(QWidget * parent, QString search);
    QWidget* widget(QWidget * parent, QString search);
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void tileClicked();

private slots:
    void on_connect_clicked();

private:
    QTcpSocket *tcpSocket;
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
