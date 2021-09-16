#ifndef MSERIALPORT_H
#define MSERIALPORT_H

#include <QObject>
#include <QMainWindow>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QDebug>
#include <QMessageBox>
class mSerialPort : public QObject
{
  //comment
    Q_OBJECT
public:
    explicit mSerialPort(QObject *parent = nullptr);
    QSerialPort* serial;
    QSerialPortInfo serial_info;
    QList<QSerialPortInfo> name_sport;
    QList<QString> list_port;

    bool data_view = false;
    bool data_plot = false;
    bool data_split = false;
    bool data_out_to_txt_box = false;
    bool data_to_table_1 = false;
    bool data_to_table_2 = false;

signals:
    void out_data(QString);
    void write_data(QString data);
    void all_Port();
    void dumm_signal();
    void data_to_plot(QString data);
    void pars_data_signal(QString data);
    void split_data(QString data);
    void msg_gadget(QString type_msg);
public slots:
    void serialWriter(QString data);
    void serialReceived();
    void run();
    void getPorts();
    void open_port(QString name_port);
    void close_port();
};

#endif // MSERIALPORT_H
