#include "sensor.h"

Sensor::Sensor(QObject *parent) : QObject(parent)
{

}

void Sensor::temperature(QString data)
{
    emit signal_lcd(calc_temperature(data),p_lcd_t);
}

QString Sensor::calc_temperature(QString data)
{
    float v_source = 3.26;
    float one_quant = 0.000795;
    float v_volts = 0.0;
    int r_term = 0;
    int r1 = 10000;
    int v_adc = QString(data).toInt(nullptr,16);
    v_volts = v_adc * one_quant;
    r_term = r1/((v_source/v_volts) - 1);
    double temperature = interpolation_temperature(r_term);
    QString string_temperature = QString::number(temperature,'f',1);

    return string_temperature;
}

void Sensor::humidity(QString data)
{
    emit signal_lcd(calc_humidity(data),p_lcd_h);
}

QString Sensor::calc_humidity(QString data)
{
    float one_quant = 0.000795;
    float v_volts = 0.0;
    int v_adc = QString(data).toInt(nullptr,16);
    v_volts = v_adc * one_quant;
    QString string_humidity = "0.0";
    if(*table_hum_create == true)
    {
        float humidity = interpolation_humidity(v_volts);
        string_humidity = QString::number(humidity,'f',1);
    }
    return string_humidity;
}

double Sensor::interpolation_temperature(int r_termistor)
{
    double temperature = 0.0;
    double r_25deg = 10000.0;
    double c = r_termistor/r_25deg;
    QVector<double> termistor_table = {
       0.2488,0.2986,0.3603,0.4369,0.5327,0.6531,0.8057,1.0,1.249,1.571,1.99,2.539,
       3.265,4.232,5.533,7.293,9.707,13.04,17.7,24.26,33.65,47.17,67.01,96.3};

    QVector<double> temperature_table = {
       60.0,55.0,50.0,45.0,40.0,35.0,30.0,25.0,20.0,15.0,10.0,5.0,0.0,-5.0,-10.0,
       -15.0,-20.0,-25.0,-30.0,-35.0,-40.0,-45.0,-50.0,-55.0};
    QVector<double>::Iterator position;
    ptr_termistor_table = &termistor_table;
    ptr_temperature_table = &temperature_table;
   /*произведем поиск в массиве*/
    int idx_search = ptr_termistor_table->indexOf(c);
   //  int idx_search = termistor_table.indexOf(c);
    if(idx_search != -1)
    {
       /*полщучим температуру из массива*/
        temperature = ptr_temperature_table->value(idx_search);
        //   temperature = temperature_table.value(idx_search);
    }
    else
    {
        position = std::lower_bound(ptr_termistor_table->begin(),ptr_termistor_table->end(),c);
        // position = std::lower_bound(termistor_table.begin(),termistor_table.end(),c);
        int idx = position - ptr_termistor_table->begin();
        //  int idx = position - termistor_table.begin();
        /*
            temperature = b + (c - a/ e - a)* (f - b /1)
        */
        double a,b,e,f;

        a = ptr_termistor_table->value(idx);
        b = ptr_temperature_table->value(idx);
        e = ptr_termistor_table->value(idx - 1);
        f = ptr_temperature_table->value(idx - 1);

//        a = termistor_table.value(idx);
//        b = temperature_table.value(idx);
//        e = termistor_table.value(idx - 1);
//        f = temperature_table.value(idx - 1);

        temperature = b + ((c - a)/(e - a))* (f - b /1);

    }
    return temperature;
}

float Sensor::interpolation_humidity(float v_in)
{
    float humidity;
    QVector<float>::Iterator position;
   /*произведем поиск в массиве*/
    int idx_search = ptr_v_in_humidity_table->indexOf(v_in);
    if(idx_search != -1)
    {
       /*полщучим температуру из массива*/
        humidity = ptr_v_in_humidity_table->value(idx_search);
    }
    else
    {
        position = std::lower_bound(ptr_v_in_humidity_table->begin(),ptr_v_in_humidity_table->end(),v_in);
        int idx_vin_t = position - ptr_v_in_humidity_table->begin();
        if(idx_vin_t == 0)
        {
            humidity = 100.0;
        }
        else
        {
            int idx_hum_t = ptr_v_in_humidity_table->end() - position;
            /*
                humidity = b + (c - a/ e - a)* (f - b /1)
            */
            float a,b,e,f;

            a = ptr_v_in_humidity_table->value(idx_vin_t);
            b = ptr_humidity_table->value(idx_hum_t);
            e = ptr_v_in_humidity_table->value(idx_vin_t - 1);
            f = ptr_humidity_table->value(idx_hum_t - 1);

            humidity = b + ((v_in - a)/(e - a))* (f - b /1);
        }

    }
    return humidity;
}

float Sensor::calculatore_vin(QString data)
{
    float one_quant = 0.000795;
    float v_volts = 0.0;
    int v_adc = QString(data).toInt(nullptr,16);
    v_volts = v_adc * one_quant;
    return v_volts;
}
