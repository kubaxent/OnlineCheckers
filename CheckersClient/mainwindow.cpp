#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QObject>

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    tcpSocket(new QTcpSocket(this)),
    ui(new Ui::MainWindow)
{
    connect(tcpSocket, &QIODevice::readyRead, this, &MainWindow::readData);
    ui->setupUi(this);
    typedef void (QAbstractSocket::*QAbstractSocketErrorSignal)(QAbstractSocket::SocketError);
    connect(tcpSocket, static_cast<QAbstractSocketErrorSignal>(&QAbstractSocket::error), this, &MainWindow::displayError);
    resetBoard();
}

MainWindow::~MainWindow()
{
    delete ui;
}

QWidget* MainWindow::widget(QWidget * parent, QString search){
    return parent->findChild<QWidget*>(search);
}

QList<QWidget*> MainWindow::widgets(QWidget * parent, QString search){
    QRegularExpression exp(search);
    return parent->findChildren<QWidget*>(exp);
}

void MainWindow::resetBoard(){
    QList<QWidget*> blacks = widgets(ui->groupBox,"bl");
    QList<QWidget*> whites = widgets(ui->groupBox,"wh");

    for(int i = 0; i < blacks.count(); i++){
        blacks[i]->setAttribute(Qt::WA_TransparentForMouseEvents);
        if((blacks[i]->objectName().remove(0,2).toInt()/10)>2){
            blacks[i]->setVisible(false);
        }else{
            blacks[i]->setVisible(true);
        }
    }

    for(int i = 0; i < whites.count(); i++){
        whites[i]->setAttribute(Qt::WA_TransparentForMouseEvents);
        if((whites[i]->objectName().remove(0,2).toInt()/10)<5){
            whites[i]->setVisible(false);
        }else{
            whites[i]->setVisible(true);
        }
    }
}

QString color;
QString prev = "";
void MainWindow::tileClicked(){
    color="wh"; //will get this form server
    QPushButton *button = (QPushButton *)sender();
    if(prev==""){
        prev = button->objectName().remove(0,1);
    }else{
        if(true){ //here the accept from the server
            //QMessageBox::about(this,color,color+(button->objectName().remove(0,1)));
            widget(ui->groupBox,color+button->objectName().remove(0,1))->setVisible(true);
            widget(ui->groupBox,color+prev)->setVisible(false);
        }
        prev = "";
    }
}

void MainWindow::on_connect_clicked(){
    tcpSocket->connectToHost(ui->serverName->text(), ui->serverPort->text().toInt());
    //QByteArray text = tcpSocket->writ();
}
