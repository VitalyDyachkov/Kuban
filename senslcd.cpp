#include "senslcd.h"
#include <mainwindow.h>

SensLCD::SensLCD()
{
//    QObject::connect(this,SIGNAL(signal_temperature),this,SLOT(temperature));
//    QObject::connect(this,SIGNAL(signal_humidity),this,SLOT(humidity()));
}

void SensLCD::temperature(QString data)
{
   QString copy_data;
   copy_data = data;
   //emit this->signal_lcd("24",p_lcd_t);
}

void SensLCD::humidity(QString data)
{
    QString copy_data;
    copy_data = data;
   //  emit signal_lcd("5",p_lcd_h);
}
