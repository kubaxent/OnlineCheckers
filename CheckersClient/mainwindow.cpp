#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QObject>
#include <QDataStream>

#define MESSAGE_BUFFER 500

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    tcpSocket(new QTcpSocket(this)),
    ui(new Ui::MainWindow)
{
    connect(tcpSocket, &QIODevice::readyRead, this, &MainWindow::readData);
    ui->setupUi(this);
    in.setDevice(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);
    model = new QStringListModel();
    resetBoard();
}


void MainWindow::readData(){
    //QByteArray text = tcpSocket->read(MESSAGE_BUFFER);
    //QString text_string = QString(text);

    QString text_string;

    do{
        in.startTransaction();
        in >> text_string;
    }while(!in.commitTransaction());

    QRegularExpression exp("/wtp");
    QRegularExpressionMatch match = exp.match(text_string);
    if(match.hasMatch()){
        QString enemy = text_string.split(' ')[1];

        QMessageBox msgBox;
        msgBox.setText(enemy+" wants to play.");
        msgBox.setInformativeText("Do you accept?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::Yes);
        int ret = msgBox.exec();

        QString response;
        switch (ret) {
          case QMessageBox::Yes:
              response = "/accept";
              tcpSocket->write(response.toLocal8Bit().data(),MESSAGE_BUFFER);
              inGame = true;
              break;
          case QMessageBox::No:
              response = "no";
              tcpSocket->write(response.toLocal8Bit().data(),MESSAGE_BUFFER);
              break;
          default:
              // should never be reached
              break;
        }

    }

    QRegularExpression exp0("/bl");
    QRegularExpressionMatch match0 = exp0.match(text_string);
    if(match0.hasMatch()){
        color="bl";
    }

    QRegularExpression exp1("/wh");
    QRegularExpressionMatch match1 = exp1.match(text_string);
    if(match1.hasMatch()){
        color="wh";
    }

    QRegularExpression exp2("/gs");
    QRegularExpressionMatch match2 = exp2.match(text_string);
    if(match2.hasMatch()){
        inGame = true;
    }

    QRegularExpression t1("/yt");
    QRegularExpressionMatch t1m = t1.match(text_string);
    if(t1m.hasMatch()){
        turn = true;
    }
    QRegularExpression t2("/ot");
    QRegularExpressionMatch t2m = t2.match(text_string);
    if(t2m.hasMatch()){
        turn = false;
    }

    QRegularExpression exp4("/move");
    QRegularExpressionMatch match4 = exp4.match(text_string);
    if(match4.hasMatch()){

        QList<QString> spl = text_string.split(' ');

        QString movecolor = spl[3].trimmed();
        QString killedcolor = (movecolor=="wh")?"bl":"wh";

        QString from = spl[1].trimmed();
        QString to = spl[2].trimmed();

        if(spl.count()>4){
            QString killed = spl[4].trimmed();
            widget(ui->groupBox,killedcolor+killed)->setVisible(false);
        }

        if(inGame){
            widget(ui->groupBox,movecolor+to)->setVisible(true);
            widget(ui->groupBox,movecolor+from)->setVisible(false);
        }
    }

    QRegularExpression exp5("/ge");
    QRegularExpressionMatch match5 = exp5.match(text_string);
    if(match5.hasMatch()){
        inGame=false;
        resetBoard();
    }

    QString mes = "Server: " + text_string;
    log.insert(0,mes);
    model->setStringList(log);
    ui->logList->setModel(model);
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

void MainWindow::tileClicked(){
    color="wh"; //will get this form server
    QPushButton *button = (QPushButton *)sender();
    if(prev==""){
        prev = button->objectName().remove(0,1);
    }else{

        QString move = "/move " + prev + " " + button->objectName().remove(0,1);
        tcpSocket->write(move.toLocal8Bit().data(),MESSAGE_BUFFER);

        prev = "";
    }
}

void MainWindow::on_connect_clicked(){
    tcpSocket->connectToHost(ui->serverName->text(), ui->serverPort->text().toInt());
    tcpSocket->write(ui->userName->text().toLocal8Bit().data(),MESSAGE_BUFFER);
}


void MainWindow::on_playagainst_clicked(){
    QString text = "/playagainst " + ui->opponentName->text();
    tcpSocket->write(text.toLocal8Bit().data(),MESSAGE_BUFFER);
}

MainWindow::~MainWindow()
{
    delete ui;
}
