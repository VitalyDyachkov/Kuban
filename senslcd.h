#ifndef SENSLCD_H
#define SENSLCD_H

#include <QLCDNumber>
#include <QString>
#include <QObject>
class SensLCD : public QLCDNumber
{
   //Q_OBJECT
public:
    SensLCD();
    QLCDNumber *p_lcd_t;
    QLCDNumber *p_lcd_h;
    bool state = false;
public slots:
    void temperature(QString data);
    void humidity(QString data);
signals:
    void signal_lcd(QString data,QLCDNumber *lcd_n);
};


#endif // SENSLCD_H
