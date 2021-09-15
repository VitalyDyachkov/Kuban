#include "mserialport.h"

mSerialPort::mSerialPort(QObject *parent) : QObject(parent)
{
    serial = new QSerialPort(this);
    connect(serial,SIGNAL(readyRead()),this,SLOT(serialReceived()));
}

void mSerialPort::serialWriter(QString data)
{
    if(serial->isOpen())
    {
        QByteArray b_array_data;
        QByteArray temp_b_array = b_array_data = QString(data).toUtf8();

        const qint64 bytesWritten = serial->write(b_array_data);
        if(bytesWritten  == -1)
        {
            qDebug() << "Error serial 1";
        }
        else if(bytesWritten  != temp_b_array.size())
        {
            qDebug() << "Error serial 2";
        }
        else
        {
            qDebug() << "Send ok";
        }
    }

}

void mSerialPort::serialReceived()
{
    try {
        QByteArray b_array_data;

        QString one_msg = serial->readAll();
        b_array_data = QString(one_msg).toUtf8();
        if(b_array_data[0] == 'A')
        {
            if(b_array_data[3] == 'S')
            {
                if(b_array_data[4] == 'C')
                {
                    if(b_array_data[6] == 'O')
                    {
                        if(b_array_data[7] == 'K') emit msg_gadget("SC_OK");
                    }
                    else if(b_array_data[6] == 'E')
                    {
                        if(b_array_data[7] == 'R') emit msg_gadget("SC_ER");
                    }
                }
            }

        }
        else
        {
                /*запишем сообщение в файл*/
                emit pars_data_signal(one_msg);
        }

    }
    catch (...)
    {

    }

}

void mSerialPort::run()
{
   // emit all_Port();
}

void mSerialPort::getPorts()
{
    /*получим список подключенных к ПК устройств*/

    QString one_name_port;
        name_sport = serial_info.availablePorts();
        for(int i = 0 ;i < name_sport.size();i++)
        {
            one_name_port = name_sport[i].portName();
            if(name_sport[i].manufacturer() == "STMicroelectronics.")
            {
               list_port.push_back(one_name_port);
            }

        }
    emit all_Port();
}

void mSerialPort::open_port(QString name_port)
{
    serial->setPortName(name_port);
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::NoParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->setFlowControl(QSerialPort::NoFlowControl);
    serial->open(QIODevice::ReadWrite);
}

void mSerialPort::close_port(void)
{
    serial->close();
}
