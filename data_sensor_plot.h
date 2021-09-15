#ifndef DATA_SENSOR_PLOT_H
#define DATA_SENSOR_PLOT_H

#include <QObject>
#include <QString>
#include <QVector>

class data_sensor_plot : public QObject
{
    Q_OBJECT
public:
    explicit data_sensor_plot(QObject *parent = nullptr);
    QString name;
    QVector<QString> hum_x,temp_x;
    QVector<double> temp_y;
    QVector<double> hum_y;
signals:

};

#endif // DATA_SENSOR_PLOT_H
