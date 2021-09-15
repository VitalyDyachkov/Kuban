#include "pars_msgcan.h"
#include "mainwindow.h"
pars_msgcan::pars_msgcan()
{

}

void pars_msgcan::parsingData(QString data)
{
    QString one_msg = data;
    /*разделим сообщение на составные части*/
    QStringList msg_buff = one_msg.split("\n");
    QString str_id = msg_buff[0];
    int leng_data = msg_buff.size() - 1;
    for(int i = 0; i < leng_data;i++)
    {
        /*разделим сообщение на составные части*/
        QStringList split_msg = msg_buff[i].split(" ");
        /*создадим имя файла из номера can и id */
        QString id;

        QStringList buff = split_msg[2].split("-");


        for(int i = 0 ; i < 4;i++)
        {
            id.append(buff[i]);
        }

         /*произведем поиск id в списке*/
            int idx = id_sensors_list.indexOf(id);
            /*если id уже встречался ранее произведем проверку на состояние старта*/
            if(idx != -1)
            {
                emit sensor_data_to_lcd(id,split_msg[1],split_msg[2]);
                //emit signal_pars_data_temp_hum(split_line[1],split_line[2],split_line[3],split_line[0],log_dir_list[i],p_sens);
            }
            else
            {
                emit id_in_combo_signal(id);
            }

      }
}
