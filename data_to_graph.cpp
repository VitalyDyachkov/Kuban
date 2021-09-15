#include "data_to_graph.h"

/*
brief
Класс для отравки данных на график
*/
data_to_graph::data_to_graph(QObject *parent) : QObject(parent)
{

}
/*функция перевода данных из представления датчика в представления программы*/
void data_to_graph::slot_pars_data_temp_hum(QString id,QString type_data, QString data, QString time,QString name_file,Sensor* p_sens)
{
     QString calc_data;

    /*определим тип данных*/
    if(type_data == "00000777")
    {
       QList<QString>::ConstIterator Iter = std::find(list_temperature.begin(),list_temperature.end(),id);
       /*id встретился впервые*/
       if(Iter == list_temperature.end())
       {
           list_temperature.push_back(id);
           emit add_id(id,TEMPERATURE);
           /*создадим файл в папке с температурными данными*/
           QDir d_data_dir =  QCoreApplication::applicationDirPath();
           QString s_data_dir = QCoreApplication::applicationDirPath()+"/data/temperature/" + id;
           /* создадим папку /data */
           if(d_data_dir.mkdir(s_data_dir) == true)
           {
               /*создадим файл с именем log файла*/
               s_data_dir += "/" + name_file;
               QFile l_file(s_data_dir);
               QString s_date_time;
               l_file.open(QFile::WriteOnly| QFile::Append);
               QTextStream stream_log(&l_file);
               s_date_time.append(time);
               s_date_time.append(" ");
               QStringList buff =  data.split('\r');
               calc_data = p_sens->calc_temperature(buff[0]);
               s_date_time.append(calc_data);
               s_date_time.append("\r\n");
               /*запишем принятое сообщение в файл*/
               stream_log << s_date_time;
               /**/
               l_file.flush();
               /*закроем файл*/
               l_file.close();
               addr_list_temperature.push_back(s_data_dir);
           }


       }
       else
       {
           QString s_data_dir = QCoreApplication::applicationDirPath()+"/data/temperature/" + id;
           s_data_dir += "/" + name_file;
           QList<QString>::ConstIterator Iter_addr = std::find(addr_list_temperature.begin(),addr_list_temperature.end(),s_data_dir);
           int idx;

           if(Iter_addr != addr_list_temperature.end())
           {
             idx = list_temperature.indexOf(*Iter);
             s_data_dir = addr_list_temperature[idx];
           }

           QFile l_file(s_data_dir);
           QString s_date_time;
           l_file.open(QFile::WriteOnly| QFile::Append);
           QTextStream stream_log(&l_file);
           s_date_time.append(time);
           s_date_time.append(" ");
           QStringList buff =  data.split('\r');
            calc_data = p_sens->calc_temperature(buff[0]);
           s_date_time.append(calc_data);
           s_date_time.append("\r\n");
           /*запишем принятое сообщение в файл*/
           stream_log << s_date_time;
           /**/
           l_file.flush();
           /*закроем файл*/
           l_file.close();
       }
    }
    else if(type_data == "00000555")
    {

        QList<QString>::ConstIterator Iter = std::find(list_humyditi.begin(),list_humyditi.end(),id);
        /*id встретился впервые*/
        if(Iter == list_humyditi.end())
        {
            list_humyditi.push_back(id);
            emit add_id(id,HUMYDITI);
            /*создадим файл в папке с данными влажности*/
            QDir d_data_dir =  QCoreApplication::applicationDirPath();
            QString s_data_dir = QCoreApplication::applicationDirPath()+"/data/humidity/" + id;
            /* создадим папку /data */
            if(d_data_dir.mkdir(s_data_dir) == true)
            {
                /*создадим файл си менем log файла*/
                s_data_dir += "/" + name_file;
                QFile l_file(s_data_dir);
                QString s_date_time;
                l_file.open(QFile::WriteOnly| QFile::Append);
                QTextStream stream_log(&l_file);
                s_date_time.append(time);
                s_date_time.append(" ");
                QStringList buff =  data.split('\r');
                calc_data = p_sens->calc_humidity(buff[0]);
                s_date_time.append(calc_data);
                s_date_time.append("\r\n");
                /*запишем принятое сообщение в файл*/
                stream_log << s_date_time;
                /**/
                l_file.flush();
                /*закроем файл*/
                l_file.close();
                addr_list_humyditi.push_back(s_data_dir);
            }
        }
        else
        {
            QString s_data_dir = QCoreApplication::applicationDirPath()+"/data/humidity/" + id;
            s_data_dir += "/" + name_file;
            QList<QString>::ConstIterator Iter_addr = std::find(addr_list_humyditi.begin(),addr_list_humyditi.end(),s_data_dir);
            int idx;

            if(Iter_addr != addr_list_humyditi.end())
            {
              idx = list_humyditi.indexOf(*Iter);
              s_data_dir = addr_list_humyditi[idx];
            }

            QFile l_file(s_data_dir);
            QString s_date_time;
            l_file.open(QFile::WriteOnly| QFile::Append);
            QTextStream stream_log(&l_file);
            s_date_time.append(time);
            s_date_time.append(" ");
            QStringList buff =  data.split('\r');
            calc_data = p_sens->calc_humidity(buff[0]);
            s_date_time.append(calc_data);
            s_date_time.append("\r\n");
            /*запишем принятое сообщение в файл*/
            stream_log << s_date_time;
            /**/
            l_file.flush();
            /*закроем файл*/
            l_file.close();
        }
    }
}
