#ifndef PARS_MSGCAN_H
#define PARS_MSGCAN_H
#include <QObject>
#include <QMainWindow>
class pars_msgcan: public QObject
{
    Q_OBJECT

    bool m_running = false;

public:
    pars_msgcan();
    QList<QString> id_sensors_list;
public slots:
      void parsingData(QString data);
signals:
    void id_in_combo_signal(QString split_msg);
    void sensor_data_to_lcd(QString id,QString type_data,QString data);
};

#endif // PARS_MSGCAN_H
