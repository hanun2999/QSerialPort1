#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include<QSerialPort>
#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QSerialPort *my_serial;
    //int start,stop,input;
    //QString flow,style,com;
    QString com;
    qint32 speed;
    QString putText;
    QSerialPort::DataBits dataBit;
    QSerialPort::FlowControl flowControl;
    QSerialPort::Parity parity;
    QSerialPort::StopBits stopBit;
    QSerialPort::BaudRate speedBound;
    int line;
    void fillPortsInfo();
    void writeSendText(const QByteArray &data);
    char ConvertHexChar(char c);
    QByteArray QString2Hex(QString str) ;
    bool eventFilter(QObject* object, QEvent* event);


private slots:
    void on_actionconnect_triggered();

    void on_actiondisconnect_triggered();

    void on_pb_send_clicked();

    void readData();

    void writeData(const QByteArray &data);

    void error(QSerialPort::SerialPortError serialPortError); // 操作错误槽函数
    void on_pushButton_clicked();
    void plainTextEdit_cursorPositionChanged();
signals:
    void getData(const QByteArray &data);





private:
    Ui::MainWindow *ui;



};

#endif // MAINWINDOW_H
