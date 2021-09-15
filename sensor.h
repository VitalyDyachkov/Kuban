#ifndef SENSOR_H
#define SENSOR_H

#include <QObject>
#include <QLCDNumber>
#include <QtAlgorithms>
class Sensor : public QObject
{
    Q_OBJECT
public:
    explicit Sensor(QObject *parent = nullptr);
    QLCDNumber *p_lcd_t;
    QLCDNumber *p_lcd_h;
    bool state = false;
    bool calibration_zero = false;
    bool calibration_hundred = false;
    bool* table_hum_create;
     QVector<double>* ptr_termistor_table;
     QVector<double>* ptr_temperature_table;
     QVector<float>* ptr_v_in_humidity_table;
     QVector<float>* ptr_humidity_table;



public slots:
    void temperature(QString data);
    QString calc_temperature(QString data);
    void humidity(QString data);
    QString calc_humidity(QString data);
    double interpolation_temperature(int r_termistor);
    float interpolation_humidity(float v_in);
    float calculatore_vin(QString data);
signals:
    void signal_lcd(QString data,QLCDNumber *lcd_n);
};

#endif // SENSOR_H
