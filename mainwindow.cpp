#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPort>
#include <QDebug>
#include <QMessageBox>
#include <QSerialPortInfo>
#include <QTextBlock>
#pragma execution_character_set("utf-8")

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    fillPortsInfo();
    my_serial=new QSerialPort(this);
    QObject::connect(my_serial,SIGNAL(errorOccurred(QSerialPort::SerialPortError)),this,SLOT(error(QSerialPort::SerialPortError)));
    QObject::connect(this, SIGNAL(getData(QByteArray)), this, SLOT(writeData(QByteArray)));
    QObject::connect(my_serial, &QSerialPort::readyRead, this, &MainWindow::readData);
    QObject::connect(ui->plainTextEdit2,SIGNAL(cursorPositionChanged()),this,SLOT(plainTextEdit_cursorPositionChanged()));
    //传输速率
    ui->cb_speed->addItem(QStringLiteral("4800"), QSerialPort::Baud4800);
    ui->cb_speed->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->cb_speed->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->cb_speed->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->cb_speed->addItem(tr("Custom"));
    //输出字符
    ui->cb_input->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->cb_input->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->cb_input->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->cb_input->addItem(QStringLiteral("8"), QSerialPort::Data8);
    ui->cb_input->setCurrentIndex(2);
    //校验方式
    ui->cb_style->addItem(tr("None"), QSerialPort::NoParity);
    ui->cb_style->addItem(tr("Even"), QSerialPort::EvenParity);
    ui->cb_style->addItem(tr("Odd"), QSerialPort::OddParity);
    ui->cb_style->addItem(tr("Mark"), QSerialPort::MarkParity);
    ui->cb_style->addItem(tr("Space"), QSerialPort::SpaceParity);
    ui->cb_style->setCurrentIndex(1);
    //停止位
    ui->cb_stop->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    ui->cb_stop->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    ui->cb_stop->addItem(QStringLiteral("2"), QSerialPort::TwoStop);
    ui->cb_stop->setCurrentIndex(2);
    //流量控制
    ui->cb_flow->addItem(tr("None"), QSerialPort::NoFlowControl);
    ui->cb_flow->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    ui->cb_flow->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);

    //ui->plainTextEdit2->setPlainText("请按格式输入发送内容");

    ui->plainTextEdit2->installEventFilter(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}
//查找串口信息
void MainWindow::fillPortsInfo()
{
    ui->cb_port->clear();
    QString description;
    QString manufacturer;
    QString serialNumber;
    QString str;
    const auto infos = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &info : infos) {
        QStringList list;

        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : 0)
             << (!manufacturer.isEmpty() ? manufacturer :0 )
             << (!serialNumber.isEmpty() ? serialNumber :0 )
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : 0)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) :0 );

        ui->cb_port->addItem(list.first(), list);
        str+="\n";
        str+=list.join(" ");

        }
    ui->textEdit->setPlaceholderText(str);
}
//单个字符
char MainWindow::ConvertHexChar(char c)
{
    if((c >= '0') && (c <= '9'))
            return c - 0x30;
        else if((c >= 'A') && (c <= 'F'))
            return c - 'A' + 10;//'A' = 65;
        else if((c >= 'a') && (c <= 'f'))
            return c - 'a' + 10;
        else
            return -1;

}

QByteArray MainWindow::QString2Hex(QString str)
{
   QByteArray senddata;
   int hexdata,lowhexdata;
   int hexdatalen = 0;
   int len = str.length();
    senddata.resize(len/2);
      char lstr,hstr;
    for(int i=0; i<len; )
    {
       hstr=str[i].toLatin1();

      if(hstr == ' ')
      {
        i++;
        continue;
      }
      i++;
      if(i >= len)
          break;
      lstr = str[i].toLatin1();

      hexdata = ConvertHexChar(hstr);
      lowhexdata = ConvertHexChar(lstr);

       if((hexdata == 16) || (lowhexdata == 16))
         break;
      else
        hexdata = hexdata*16+lowhexdata;

       i++;
       senddata[hexdatalen] = (char)hexdata;
       hexdatalen++;
       qDebug()<<hexdata;
    }
     senddata.resize(hexdatalen);
     return senddata;
}
//事件过滤器
bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if(object==ui->plainTextEdit2){
        if(event->type()==QEvent::KeyPress){
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            if(keyEvent->key()==Qt::Key_Return){
                QString T;
                QByteArray Temp;

     T = ui->plainTextEdit2->document()->findBlockByLineNumber(line).text();
                //T = this->ui->plainTextEdit2->toPlainText();
                if(ui->cb_send->currentIndex()==0){
                   Temp = T.toLatin1();
                   Temp[T.size()]='\r';

                }else{
                   Temp=QString2Hex(T);
                }

                emit getData(Temp);

                qDebug()<<Temp;
            }

        }
    }
    return false;
}


//建立连接
void MainWindow::on_actionconnect_triggered()
{


    if(my_serial->isOpen()){
        my_serial->clear();
        my_serial->close();
    }

    com=ui->cb_port->currentText();
    speedBound= static_cast<QSerialPort::BaudRate>(
                        ui->cb_speed->itemData(ui->cb_speed->currentIndex()).toInt());
    dataBit=static_cast<QSerialPort::DataBits>(
                ui->cb_input->itemData(ui->cb_input->currentIndex()).toInt());
    flowControl=static_cast<QSerialPort::FlowControl>(
                ui->cb_flow->itemData(ui->cb_flow->currentIndex()).toInt());
    parity=static_cast<QSerialPort::Parity>(
                ui->cb_style->itemData(ui->cb_style->currentIndex()).toInt());
    stopBit=static_cast<QSerialPort::StopBits>(
                ui->cb_stop->itemData(ui->cb_stop->currentIndex()).toInt());

        my_serial->setPortName(com);
        my_serial->setBaudRate(speedBound);
        my_serial->setDataBits(dataBit);
        my_serial->setParity(parity);
        my_serial->setStopBits(stopBit);
        my_serial->setFlowControl(flowControl);




    if (my_serial->open(QIODevice::ReadWrite)){
        qDebug()<<"以读写方式打开";
        ui->actionconnect->setEnabled(false);
        ui->actiondisconnect->setEnabled(true);

    }


    //QMessageBox::information(this,"提示","已建立连接！");

}
//取消连接
void MainWindow::on_actiondisconnect_triggered()
{
    if (my_serial->isOpen())
        my_serial->close();
    ui->actiondisconnect->setEnabled(false);
    ui->actionconnect->setEnabled(true);
  QMessageBox::information(this,"提示","已断开连接！");
}

void MainWindow::on_pb_send_clicked()
{

    /*if(my_serial->isOpen()){
        QByteArray b;
        b=ui->plainTextEdit2->toPlainText().toLocal8Bit();
        qDebug()<<b;
        emit getData(b);

    }else{
        qDebug()<<"串口未打开";
    }*/
    if(my_serial->isOpen()){
        QString T;
        QByteArray Temp;


 T = ui->plainTextEdit2->document()->findBlockByLineNumber(line).text();
       // T = this->ui->plainTextEdit2->toPlainText();
        if(ui->cb_send->currentIndex()==0){
           Temp = T.toLatin1();
           Temp[T.size()]='\r';

        }else{
           Temp=QString2Hex(T);
        }

        //QString ret(Temp.toHex().toUpper());

        //Temp[T.size()-1]='\r';

        /*QString ret(Temp.toHex().toUpper());
         *
            int len = ret.length()/2;
            //添加空格
            for(int i=1;i<len;i++)
            {

                ret.insert(2*i+i-1," ");
            }

        //QByteArray res=ret.toLatin1();
        //emit getData(res);*/
        //QByteArray re=QString2Hex(ret);


        emit getData(Temp);

        qDebug()<<Temp;


    }else{

        qDebug()<<my_serial->error();

    }




}

void MainWindow::readData()
{


    /*QByteArray buf;
    if(my_serial->bytesAvailable() >= 0)
           {
            buf = my_serial->readAll();

            if(!buf.isEmpty())
            {

                ui->textEdit1->setText(buf);
                qDebug()<<buf<<"size of buffer:"<<buf.size();

            }
        }else
        {
            qDebug()<<"接受数据出错" + QString::number(my_serial->bytesAvailable());
        }*/

  //读取串口数据
      QByteArray readComData = my_serial->readAll();

      //将读到的数据显示到数据接收区的te中
      if(readComData !=" ")
      {
          putText.append(readComData);

      }
      ui->plainTextEdit1->setPlainText(putText);
      qDebug()<<"接收"<<putText;

}

void MainWindow::writeData(const QByteArray &data)
{
    my_serial->write(data);
    my_serial->waitForBytesWritten(5);
    //if(my_serial->waitForReadyRead()){
        //qDebug()<<"received";
   //}
}

void MainWindow::error(QSerialPort::SerialPortError serialPortError)
{
    if(serialPortError){
        qDebug()<<serialPortError;
      QMessageBox::warning(this,"警告",my_serial->errorString());
    }

}


void MainWindow::on_pushButton_clicked()
{
    putText.clear();
    ui->plainTextEdit1->setPlainText(putText);
}
//获取行号
void MainWindow::plainTextEdit_cursorPositionChanged()
{
      QTextCursor cursor;
      cursor=ui->plainTextEdit2->textCursor();
      line=cursor.blockNumber();
      qDebug()<<QString::number(line);


}
