#ifndef DATA_TO_GRAPH_H
#define DATA_TO_GRAPH_H

#include <QObject>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>
#include "sensor.h"


#define TEMPERATURE 1
#define HUMYDITI 2
class data_to_graph : public QObject
{
    Q_OBJECT
public:
    explicit data_to_graph(QObject *parent = nullptr);
QStringList list_humyditi;
QStringList list_temperature;

QStringList addr_list_humyditi;
QStringList addr_list_temperature;

signals:
void add_id(QString id,int type_data);
public slots:
    void slot_pars_data_temp_hum(QString id,QString type_data,QString data,QString time,QString name_file,Sensor* p_sens);

};

#endif // DATA_TO_GRAPH_H
