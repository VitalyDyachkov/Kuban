#ifndef DATA_SENSOR_PLOYT_H
#define DATA_SENSOR_PLOYT_H

#include <QObject>

class data_sensor_ployt : public QObject
{
    Q_OBJECT
public:
    explicit data_sensor_ployt(QObject *parent = nullptr);

signals:

};

#endif // DATA_SENSOR_PLOYT_H
