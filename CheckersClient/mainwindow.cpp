#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QObject>

#define MESSAGE_BUFFER 500

using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    tcpSocket(new QTcpSocket(this)),
    ui(new Ui::MainWindow)
{
    connect(tcpSocket, &QIODevice::readyRead, this, &MainWindow::readData);
    ui->setupUi(this);
    //typedef void (QAbstractSocket::*QAbstractSocketErrorSignal)(QAbstractSocket::SocketError);
    model = new QStringListModel();
    resetBoard();
}

void MainWindow::readData(){
    QByteArray text = tcpSocket->read(MESSAGE_BUFFER);
    QString text_string = QString(text);
    //QString DataAsString = QTextCodec::codecForMib(1015)->toUnicode(Data);

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

    QRegularExpression exp3("/ma");
    QRegularExpressionMatch match3 = exp3.match(text_string);
    if(match3.hasMatch()){
        QList<QString> spl = text_string.split(' ');
        if(spl.count()>1){
            QString killed = text_string.split(' ')[1];
            widget(ui->groupBox,((color=="wh")?"bl":"wh")+killed)->setVisible(false);
        }
        moveAccepted = true;
    }

    QRegularExpression exp4("/move");
    QRegularExpressionMatch match4 = exp4.match(text_string);
    if(match4.hasMatch()){
        QString from = text_string.split(' ')[1].simplified();
        QString to = text_string.split(' ')[2].simplified();
        QString opcolor = (color=="wh")?"bl":"wh";
        if(inGame){
            widget(ui->groupBox,opcolor+to)->setVisible(true);
            widget(ui->groupBox,opcolor+from)->setVisible(false);
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

/*int safe_read(int socket_fd, char input[MESSAGE_BUFFER]){
    int response, n = 0;
    while(n!=MESSAGE_BUFFER){
        response = read(socket_fd, &input[n], MESSAGE_BUFFER-n);
        if(response<=0){
            return -1;
        }
        n+=response;
    }
    return 0;
}

int safe_write(int socket_fd, char message[MESSAGE_BUFFER]){
    int response, n = 0;
    while(n!=MESSAGE_BUFFER){
        response = write(socket_fd, &message[n], MESSAGE_BUFFER-n);
        if(response==-1){
            return -1;
        }
        n+=response;
    }
    return 0;
}*/

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


        //while(!moveAccepted){
            if(inGame && moveAccepted){ //here the accept from the server
                //QMessageBox::about(this,color,color+(button->objectName().remove(0,1)));
                widget(ui->groupBox,color+button->objectName().remove(0,1))->setVisible(true);
                widget(ui->groupBox,color+prev)->setVisible(false);

                moveAccepted = false;
               // break;
            }
        //}
        prev = "";
    }
}

void MainWindow::on_connect_clicked(){
    tcpSocket->connectToHost(ui->serverName->text(), ui->serverPort->text().toInt());
    int sent = tcpSocket->write(ui->userName->text().toLocal8Bit().data(),MESSAGE_BUFFER);
    //QMessageBox::about(this,"Bytes sent:",QString::number(sent));
}


void MainWindow::on_playagainst_clicked(){
    QString text = "/playagainst " + ui->opponentName->text();
    int sent = tcpSocket->write(text.toLocal8Bit().data(),MESSAGE_BUFFER);
}

MainWindow::~MainWindow()
{
    delete ui;
}
