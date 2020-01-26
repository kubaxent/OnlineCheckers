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
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void tileClicked();

private slots:
    void on_connect_clicked();
    void readData();
    //void displayError(QAbstractSocket::SocketError socketError);

    void on_playagainst_clicked();

private:
    QTcpSocket *tcpSocket;
    Ui::MainWindow *ui;
    QString prev;
    QString color;
    QList<QString> log;
    bool inGame;
    bool moveAccepted;
    void resetBoard();
    QStringListModel* model;
    QList<QWidget*> widgets(QWidget * parent, QString search);
    QWidget* widget(QWidget * parent, QString search);
};

#endif // MAINWINDOW_H
