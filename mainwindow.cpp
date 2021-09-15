#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QtAlgorithms>
#include <QMediaPlayer>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    network_meneger = new QNetworkAccessManager(this);
    m_serial = new  mSerialPort();
    ptr_pars_msgcan  = new pars_msgcan();
    timer_send_to_server = new QTimer(this);
    timer_cnct_gadget = new QTimer(this);
    dt_graph = new data_to_graph();
    p_sens = new Sensor();



    msgBox_put_sensor.setText("Данный датчик выбран,выберите другой датчик!");
    current_naame_file = "";
    addr_dir_files_table = "/table_humidity.txt";
    addr_dir_files_table = QCoreApplication::applicationDirPath() + addr_dir_files_table;

    set_palette();

    connect(this,&MainWindow::signal_save_position_sensor,this,&MainWindow::slot_save_position_sensor);
    connect(this,&MainWindow::signal_set_id_in_combo_box,this,&MainWindow::slot_set_id_in_combo_box);
    connect(this,&MainWindow::open_serial_port,m_serial,&mSerialPort::open_port);
    connect(this,&MainWindow::close_serial_port,m_serial,&mSerialPort::close_port);
    connect(this,&MainWindow::signal_data_to_plot,this,&MainWindow::set_data_to_plot);

    connect(m_serial,&mSerialPort::all_Port,this,&MainWindow::OutPorts);
    connect(&thread_serial,&QThread::started,m_serial,&mSerialPort::getPorts);

    //connect(m_serial->&mSerialPort::pars_data,ptr_pars_msgcan,&pars_msgcan::parsingData);

    connect(ptr_pars_msgcan,&pars_msgcan::id_in_combo_signal,this,&MainWindow::IdCombobox);
    connect(ptr_pars_msgcan,&pars_msgcan::sensor_data_to_lcd,this,&MainWindow::MakeDataForSensor);

    connect(m_serial,&mSerialPort::pars_data_signal,ptr_pars_msgcan,&pars_msgcan::parsingData,Qt::QueuedConnection);
//    connect(timer_send_to_server,SIGNAL(timeout()),this,SLOT(send_file_server_h()));
    connect(this,&MainWindow::send_file_to_server,this,&MainWindow::send_file_server_h);


    connect(this,&MainWindow::send_msg_serial,m_serial,&mSerialPort::serialWriter);
    connect(timer_cnct_gadget,&QTimer::timeout,this,&MainWindow::cnct_gadget_timer_slot);
    connect(m_serial,&mSerialPort::msg_gadget,this,&MainWindow::msg_gadget_slot);

    connect(this,&MainWindow::signal_pars_data_temp_hum,dt_graph,&data_to_graph::slot_pars_data_temp_hum);
    connect(dt_graph,&data_to_graph::add_id,this,&MainWindow::add_id_to_combobox);

    ui->cb_interval_send_to_server->addItem("1");
    ui->cb_interval_send_to_server->addItem("5");
    ui->cb_interval_send_to_server->addItem("15");
    ui->cb_interval_send_to_server->addItem("30");
    ui->cb_interval_send_to_server->addItem("60");



    ui->cb_can_baudrate->addItem("125");
    ui->cb_can_baudrate->addItem("250");
    ui->cb_can_baudrate->addItem("500");

    m_serial->moveToThread(&thread_serial);
    ptr_pars_msgcan->moveToThread(&thread_pars_mg_can);
    dt_graph->moveToThread(&set_plots);



    thread_pars_mg_can.start();
    thread_serial.start();
    set_plots.start();
    //get_state_net();

        QFile file_table(addr_dir_files_table);
        if(file_table.open(QFile::ReadOnly))
        {
            if(file_table.size() != 0)
            {

                QString line;
                QTextStream stream_table_humidity(&file_table);
                int i = 0;
                while(!stream_table_humidity.atEnd())
               {
                   line = stream_table_humidity.readLine();
                   v_in_humidity_table.insert(i++,line.toFloat());
               }
                file_table.close();
                file_table.flush();
                h_table_create = true;
                ui->msg_box->append("Таблица считана из файла");

                p_sens->ptr_v_in_humidity_table = &this->v_in_humidity_table;
                p_sens->ptr_humidity_table = &this->humidity_table;
                 p_sens->table_hum_create = &h_table_create;

            }

        }
        else {
              h_table_create = false;
        }

 create_data_path();
 set_ptr_combobox();
 get_sensor_psitions();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::OutPorts(void)
{
     for(int i = 0 ;i < m_serial->list_port.size();i++)
     {
         ui->cmbx_SerialPort->addItem(m_serial->list_port.value(i));
     }
}

void MainWindow::IdCombobox(QString id)
{
    /*произведем поиск по списку ID*/
      QList<QString>::ConstIterator Iter = std::find(ptr_pars_msgcan->id_sensors_list.begin(),ptr_pars_msgcan->id_sensors_list.end(),id);
      /*id встечается впервые*/
      if(Iter == ptr_pars_msgcan->id_sensors_list.end())
      {
          for(int i = 0 ; i < 20; i++)
          {
            ptr_combo.at(i)->addItem(id);
          }

          ui->combo_humidity->addItem(id);
          ui->combo_id_temp->addItem(id);
          ui->combo_id_hum->addItem(id);

          ptr_pars_msgcan->id_sensors_list.push_back(id);

          Sensor *lcd_sens = new Sensor();

          lcd_sens->ptr_termistor_table = &this->termistor_table;
          lcd_sens->ptr_temperature_table = &this->temperature_table;
          lcd_sens->table_hum_create = &h_table_create;
          lcd_sens->ptr_v_in_humidity_table = &this->v_in_humidity_table;
          lcd_sens->ptr_humidity_table = &this->humidity_table;

          p_sens_lcd.push_back(lcd_sens);

      }
}

void MainWindow::MakeDataForSensor(QString id, QString type_data, QString data)
{
    /*получим индекс для данного ID*/
    int idx = ptr_pars_msgcan->id_sensors_list.indexOf(id);
    Sensor *psenslcd;
    /*получим ссылку на данный объект (датчик)*/
    psenslcd = p_sens_lcd.at(idx);
    /*если датчик включен в обработку данных от него*/
    if(psenslcd->state)
    {
        if(type_data == "00000777")
        {
            QStringList buff = data.split("-");
            QString result_measure;
            result_measure.append(buff[4]);
            result_measure.append(buff[5]);
            /*отправим данные на расчет и вывод на электронный индикатор*/
            psenslcd->temperature(result_measure);
        }
        else if (type_data == "00000555")
        {
            QStringList buff = data.split("-");
            QString result_measure;
            result_measure.append(buff[4]);
            result_measure.append(buff[5]);

            if(*psenslcd->table_hum_create == true)
            {
                /*отправим данные на расчет и вывод на электронный индикатор*/
                psenslcd->humidity(result_measure);
            }
            else
            {
                if(psenslcd->calibration_zero)
                {
                    /*получим значение Vin на входе в АЦП для 0%*/
                    humidity_v_in_0 = psenslcd->calculatore_vin(result_measure);
                    psenslcd->calibration_zero = false;
                    prDialog_put_humidity_table->setLabelText("Процесс завершен");
                    prDialog_put_humidity_table->setValue(100);
                    prDialog_put_humidity_table->setCancelButtonText("Закрыть");


                }
                else if (psenslcd->calibration_hundred)
                {
                    /*получим значение Vin на входе в АЦП для 100%*/
                    humidity_v_in_100 = psenslcd->calculatore_vin(result_measure);
                    psenslcd->calibration_hundred  = false;
                    prDialog_put_humidity_table->setLabelText("Процесс завершен");
                    prDialog_put_humidity_table->setValue(100);
                    prDialog_put_humidity_table->setCancelButtonText("Закрыть");


                }
                else
                {
                    result_measure = "0";
                    psenslcd->humidity(result_measure);
                }
            }

        }
    }

}

void MainWindow::on_bt_SerialOpen_clicked(void)
{
    try
    {
        if(!m_serial->serial->isOpen())
        {
            QString name_port = ui->cmbx_SerialPort->currentText();
            emit open_serial_port(name_port);
            ui->msg_box->append("Порт открыт!");
            ui->bt_SerialOpen->setText("Закрыть");
        }
        else
        {
           emit close_serial_port();
          ui->msg_box->append("Порт закрыт!");
           ui->bt_SerialOpen->setText("Открыть");
        }
    }
    catch (...)
    {


    }
}

void MainWindow::createDialogBox(void)
{
    prDialog_put_humidity_table = new QProgressDialog("Идет процесс получения данных...","Отмена",0,100);


    prDialog_put_humidity_table->show();
    prDialog_put_humidity_table->setAutoReset(false);
    prDialog_put_humidity_table->setAutoClose(false);
}

void MainWindow::on_pushButton_1sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_1->currentText();

    if(current_id != "")
    {
        if(ui->pushButton_1sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_1,ui->lcdNumber_2))
            {
                ui->pushButton_1sensors->setText("Стоп");
                ui->comboBox_1->setDisabled(true);
                emit signal_save_position_sensor(current_id,1);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_1sensors->setText("Старт");
            ui->comboBox_1->setDisabled(false);            
        }
    }

}

void MainWindow::on_pushButton_2sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_2->currentText();

    if(current_id != "")
    {
        if(ui->pushButton_2sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_3,ui->lcdNumber_4))
            {
                ui->pushButton_2sensors->setText("Стоп");
                ui->comboBox_2->setDisabled(true);
                emit signal_save_position_sensor(current_id,2);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_2sensors->setText("Старт");
            ui->comboBox_2->setDisabled(false);            
        }
    }
}

void MainWindow::on_pushButton_3sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_3->currentText();

    if(current_id != "")
    {
        if(ui->pushButton_3sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_5,ui->lcdNumber_6))
            {
                ui->pushButton_3sensors->setText("Стоп");
                ui->comboBox_3->setDisabled(true);
                emit signal_save_position_sensor(current_id,3);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_3sensors->setText("Старт");
            ui->comboBox_3->setDisabled(false);            
        }
    }
}

void MainWindow::on_pushButton_4sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_4->currentText();

    if(current_id != "")
    {
        if(ui->pushButton_4sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_7,ui->lcdNumber_8))
            {
                ui->pushButton_4sensors->setText("Стоп");
                ui->comboBox_4->setDisabled(true);
                emit signal_save_position_sensor(current_id,4);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_4sensors->setText("Старт");
            ui->comboBox_4->setDisabled(false);
        }
    }
}

void MainWindow::on_pushButton_5sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_5->currentText();

    if(current_id != "")
    {
        if(ui->pushButton_5sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_9,ui->lcdNumber_10))
            {
                ui->pushButton_5sensors->setText("Стоп");
                ui->comboBox_5->setDisabled(true);
                emit signal_save_position_sensor(current_id,5);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_5sensors->setText("Старт");
            ui->comboBox_5->setDisabled(false);            
        }
    }
}

void MainWindow::on_pushButton_6sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_6->currentText();

    if(current_id != "")
    {
        if(ui->pushButton_6sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_11,ui->lcdNumber_12))
            {
                ui->pushButton_6sensors->setText("Стоп");
                ui->comboBox_6->setDisabled(true);
                emit signal_save_position_sensor(current_id,6);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_6sensors->setText("Старт");
            ui->comboBox_6->setDisabled(false);            
        }
    }
}

void MainWindow::on_pushButton_7sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_7->currentText();

    if(current_id != "")
    {
        if(ui->pushButton_7sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_13,ui->lcdNumber_14))
            {
                ui->pushButton_7sensors->setText("Стоп");
                ui->comboBox_7->setDisabled(true);
                emit signal_save_position_sensor(current_id,7);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_7sensors->setText("Старт");
            ui->comboBox_7->setDisabled(false);

        }
    }
}

void MainWindow::on_pushButton_8sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_8->currentText();

    if(current_id != "")
        {
        if(ui->pushButton_8sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_15,ui->lcdNumber_16))
            {
                ui->pushButton_8sensors->setText("Стоп");
                ui->comboBox_8->setDisabled(true);
                emit signal_save_position_sensor(current_id,8);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_8sensors->setText("Старт");
            ui->comboBox_8->setDisabled(false);            
        }
    }
}

void MainWindow::on_pushButton_9sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_9->currentText();

    if(current_id != "")
    {
        if(ui->pushButton_9sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_17,ui->lcdNumber_18))
            {
                ui->pushButton_9sensors->setText("Стоп");
                ui->comboBox_9->setDisabled(true);
                emit signal_save_position_sensor(current_id,9);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_9sensors->setText("Старт");
            ui->comboBox_9->setDisabled(false);            
        }
    }
}

void MainWindow::on_pushButton_10sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_10->currentText();

    if(current_id != "")
    {
        if(ui->pushButton_10sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_19,ui->lcdNumber_20))
            {
                ui->pushButton_10sensors->setText("Стоп");
                ui->comboBox_10->setDisabled(true);
                emit signal_save_position_sensor(current_id,10);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_10sensors->setText("Старт");
            ui->comboBox_10->setDisabled(false);            
        }
    }
}

void MainWindow::on_pushButton_11sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_11->currentText();

    if(current_id != "")
    {
        if(ui->pushButton_11sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_21,ui->lcdNumber_22))
            {
                ui->pushButton_11sensors->setText("Стоп");
                ui->comboBox_11->setDisabled(true);
                emit signal_save_position_sensor(current_id,11);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_11sensors->setText("Старт");
            ui->comboBox_11->setDisabled(false);            
        }
    }
}

void MainWindow::on_pushButton_12sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_12->currentText();

    if(current_id != "")
    {
        if(ui->pushButton_12sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_23,ui->lcdNumber_24))
            {
                ui->pushButton_12sensors->setText("Стоп");
                ui->comboBox_12->setDisabled(true);
                emit signal_save_position_sensor(current_id,12);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_12sensors->setText("Старт");
            ui->comboBox_12->setDisabled(false);            
        }
    }
}

void MainWindow::on_pushButton_13sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_13->currentText();

    if(current_id != "")
    {
        if(ui->pushButton_13sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_25,ui->lcdNumber_26))
            {
                ui->pushButton_13sensors->setText("Стоп");
                ui->comboBox_13->setDisabled(true);
                emit signal_save_position_sensor(current_id,13);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_13sensors->setText("Старт");
            ui->comboBox_13->setDisabled(false);            
        }
    }
}

void MainWindow::on_pushButton_14sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_14->currentText();

    if(current_id != "")
    {
        if(ui->pushButton_14sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_27,ui->lcdNumber_28))
            {
                ui->pushButton_14sensors->setText("Стоп");
                ui->comboBox_14->setDisabled(true);
                emit signal_save_position_sensor(current_id,14);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_14sensors->setText("Старт");
            ui->comboBox_14->setDisabled(false);            
        }
    }
}

void MainWindow::on_pushButton_15sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_15->currentText();

    if(current_id != "")
    {
        if(ui->pushButton_15sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_29,ui->lcdNumber_30))
            {
                ui->pushButton_15sensors->setText("Стоп");
                ui->comboBox_15->setDisabled(true);
                emit signal_save_position_sensor(current_id,15);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_15sensors->setText("Старт");
            ui->comboBox_15->setDisabled(false);            
        }
    }
}

void MainWindow::on_pushButton_16sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_16->currentText();

    if(current_id != "")
    {
        if(ui->pushButton_16sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_31,ui->lcdNumber_32))
            {
                ui->pushButton_16sensors->setText("Стоп");
                ui->comboBox_16->setDisabled(true);
                emit signal_save_position_sensor(current_id,16);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_16sensors->setText("Старт");
            ui->comboBox_16->setDisabled(false);            
        }
    }
}

void MainWindow::on_pushButton_17sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_17->currentText();

    if(current_id != "")
    {
        if(ui->pushButton_17sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_33,ui->lcdNumber_34))
            {
                ui->pushButton_17sensors->setText("Стоп");
                ui->comboBox_17->setDisabled(true);
                emit signal_save_position_sensor(current_id,17);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_17sensors->setText("Старт");
            ui->comboBox_17->setDisabled(false);            
        }
    }
}

void MainWindow::on_pushButton_18sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_18->currentText();

    if(current_id != "")
    {
        if(ui->pushButton_18sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_35,ui->lcdNumber_36))
            {
                ui->pushButton_18sensors->setText("Стоп");
                ui->comboBox_18->setDisabled(true);
                emit signal_save_position_sensor(current_id,18);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_18sensors->setText("Старт");
            ui->comboBox_18->setDisabled(false);            
        }
    }
}

void MainWindow::on_pushButton_19sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_19->currentText();

    if(current_id != "")
    {
        if(ui->pushButton_19sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_37,ui->lcdNumber_38))
            {
                ui->pushButton_19sensors->setText("Стоп");
                ui->comboBox_19->setDisabled(true);
                emit signal_save_position_sensor(current_id,19);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_19sensors->setText("Старт");
            ui->comboBox_19->setDisabled(false);            
        }
    }
}

void MainWindow::on_pushButton_20sensors_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->comboBox_20->currentText();

    if(current_id != "")
    {
        if(ui->pushButton_20sensors->text() == "Старт")
        {
            if(create_ptr_lcd(current_id,ui->lcdNumber_39,ui->lcdNumber_40))
            {
                ui->pushButton_20sensors->setText("Стоп");
                ui->comboBox_20->setDisabled(true);
                emit signal_save_position_sensor(current_id,20);
            }
        }
        else
        {
            delete_ptr_lcd(current_id);
            ui->pushButton_20sensors->setText("Старт");
            ui->comboBox_20->setDisabled(false);            
        }
    }
}

void MainWindow::set_palette(void)
{
    palette_lcd = ui->lcdNumber_1->palette();
    ui->lcdNumber_1->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_1->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_2->palette();
    ui->lcdNumber_2->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_2->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_3->palette();
    ui->lcdNumber_3->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_3->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_4->palette();
    ui->lcdNumber_4->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_4->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_5->palette();
    ui->lcdNumber_5->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_5->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_6->palette();
    ui->lcdNumber_6->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_6->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_7->palette();
    ui->lcdNumber_7->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_7->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_8->palette();
    ui->lcdNumber_8->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_8->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_9->palette();
    ui->lcdNumber_9->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_9->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_10->palette();
    ui->lcdNumber_10->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_10->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_11->palette();
    ui->lcdNumber_11->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_11->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_12->palette();
    ui->lcdNumber_12->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_12->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_13->palette();
    ui->lcdNumber_13->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_13->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_14->palette();
    ui->lcdNumber_14->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_14->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_15->palette();
    ui->lcdNumber_15->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_15->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_16->palette();
    ui->lcdNumber_16->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_16->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_17->palette();
    ui->lcdNumber_17->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_17->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_18->palette();
    ui->lcdNumber_18->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_18->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_19->palette();
    ui->lcdNumber_19->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_19->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_20->palette();
    ui->lcdNumber_20->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_20->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_21->palette();
    ui->lcdNumber_21->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_21->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_22->palette();
    ui->lcdNumber_22->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_22->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_23->palette();
    ui->lcdNumber_23->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_23->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_24->palette();
    ui->lcdNumber_24->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_24->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_25->palette();
    ui->lcdNumber_25->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_25->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_26->palette();
    ui->lcdNumber_26->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_26->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_27->palette();
    ui->lcdNumber_27->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_27->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_28->palette();
    ui->lcdNumber_28->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_28->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_29->palette();
    ui->lcdNumber_29->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_29->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_30->palette();
    ui->lcdNumber_30->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_30->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_31->palette();
    ui->lcdNumber_31->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_31->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_32->palette();
    ui->lcdNumber_32->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_32->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_33->palette();
    ui->lcdNumber_33->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_33->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_34->palette();
    ui->lcdNumber_34->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_34->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_35->palette();
    ui->lcdNumber_35->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_35->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_36->palette();
    ui->lcdNumber_36->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_36->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_37->palette();
    ui->lcdNumber_37->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_37->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_38->palette();
    ui->lcdNumber_38->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_38->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_39->palette();
    ui->lcdNumber_39->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_39->setPalette(palette_lcd);

    palette_lcd = ui->lcdNumber_40->palette();
    ui->lcdNumber_40->setSegmentStyle(QLCDNumber::Flat);
    palette_lcd.setColor(QPalette::Background,Qt::darkBlue);
    palette_lcd.setColor(QPalette::WindowText,Qt::green);

    ui->lcdNumber_40->setPalette(palette_lcd);
}

void MainWindow::lcd_set_data(QString data, QLCDNumber *lcd_n)
{
    lcd_n->display(data);
}

bool MainWindow::create_ptr_lcd(QString id, QLCDNumber *lcd_temperature, QLCDNumber *lcd_humidity)
{
    bool result = false;
    /*получим индекс для данного ID*/
    int idx = ptr_pars_msgcan->id_sensors_list.indexOf(id);
    Sensor *psenslcd;
    /*получим ссылку на данный объект (датчик)*/
    psenslcd = p_sens_lcd.at(idx);
    if(!psenslcd->state)
    {
        /*полю объекта датчик присвоим ссылку на LCD_Temperature*/
        psenslcd->p_lcd_t = lcd_temperature;
        /*полю объекта датчик присвоим ссылку на LCD_Humidity*/
        psenslcd->p_lcd_h = lcd_humidity;
        /*включим датчик в обработку данных*/
        psenslcd->state = true;
        connect(psenslcd,&Sensor::signal_lcd,this,&MainWindow::lcd_set_data);
        result = true;
    }
    else
    {
        msgBox_put_sensor.exec();
    }
    return  result;
}

void MainWindow::delete_ptr_lcd(QString id)
{
    /*получим индекс для данного ID*/
    int idx = ptr_pars_msgcan->id_sensors_list.indexOf(id);
    //Sensor *qlcd;
    //qlcd = ptr_sensor.at(idx);
    Sensor *psenslcd;
    /*получим ссылку на данный объект (датчик)*/
    psenslcd = p_sens_lcd.at(idx);
    /*полю объекта датчик присвоим ссылку на LCD_Temperature*/
    psenslcd->p_lcd_t = nullptr;
    /*полю объекта датчик присвоим ссылку на LCD_Humidity*/
    psenslcd->p_lcd_h = nullptr;
    /*включим датчик в обработку данных*/
    psenslcd->state = false;
    disconnect(psenslcd,&Sensor::signal_lcd,this,&MainWindow::lcd_set_data);
}

void MainWindow::on_humidity_zero_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->combo_humidity->currentText();
    /*получим индекс для данного ID*/
    int idx = ptr_pars_msgcan->id_sensors_list.indexOf(current_id);
    Sensor *psenslcd;
    /*получим ссылку на данный объект (датчик)*/
    psenslcd = p_sens_lcd.at(idx);
    psenslcd->calibration_zero = true;

    if(h_table_create)
        h_table_create = false;
    createDialogBox();
    //prDialog_put_humidity_table->setLabelText("Идет процесс получения данных...");
    //prDialog_put_humidity_table->setWindowModality(Qt::WindowModal);
    prDialog_put_humidity_table->setValue(50);

}

void MainWindow::on_humidity_hundred_clicked(void)
{
    /*получим текущий ID из выпадающего списка*/
    QString current_id = ui->combo_humidity->currentText();

    /*получим индекс для данного ID*/
    int idx = ptr_pars_msgcan->id_sensors_list.indexOf(current_id);
    Sensor *psenslcd;
    /*получим ссылку на данный объект (датчик)*/
    psenslcd = p_sens_lcd.at(idx);
    psenslcd->calibration_hundred = true;

    if(h_table_create)
        h_table_create = false;
    createDialogBox();
    //prDialog_put_humidity_table->setLabelText("Идет процесс получения данных...");
    //prDialog_put_humidity_table->setWindowModality(Qt::WindowModal);
    prDialog_put_humidity_table->setValue(50);

}

void MainWindow::on_humidity_calc_table_clicked(void)
{       
    QString current_id = ui->combo_humidity->currentText();
    prDialog_put_humidity_table->setWindowModality(Qt::WindowModal);
    prDialog_put_humidity_table->setValue(50);
    prDialog_put_humidity_table->setLabelText("Идет рассчет таблицы");
    /*получим индекс для данного ID*/
   // int idx = ptr_pars_msgcan->id_sensors_list.indexOf(current_id);
   // Sensor *psenslcd;

    float delta = 0.0;
    float humidity_temp = humidity_v_in_100;
    delta = humidity_v_in_0 - humidity_v_in_100;
    delta /= 40;
    if(v_in_humidity_table.size() == 41)
    v_in_humidity_table.remove(0,41);

    v_in_humidity_table.insert(0,humidity_v_in_100);
    for(int i = 1; i < 40;i++)
    {
        humidity_temp += delta;
        v_in_humidity_table.insert(i,humidity_temp);

    }
    v_in_humidity_table.insert(40,humidity_v_in_0);
    h_table_create = true;

    QFile file_table(addr_dir_files_table);

    if(file_table.open(QFile::WriteOnly))
    {
        QTextStream stream_table_humidity(&file_table);
        for (QVector<float>::iterator iter = v_in_humidity_table.begin();iter != v_in_humidity_table.end();iter++)
        {
             stream_table_humidity << *iter;
             stream_table_humidity<<"\r\n";
        }
        file_table.close();
        file_table.flush();

        log_to_server_file = new QFile(addr_dir_files_table);
        QFileInfo file_info(*log_to_server_file);
        QString path_to_server = SERVER_ADDR;
        QString name_file_upload = "";
        QUrl server = (path_to_server + file_info.fileName());
        server.setUserName(CLIENT_NAME);
        server.setPassword(PASSWORD);
        server.setPort(21);

        if(log_to_server_file->open(QIODevice::ReadOnly))
        {
            QNetworkReply *reply = network_meneger->put(QNetworkRequest(server),log_to_server_file);
            if(reply->error() == QNetworkReply::NoError)
            {
                ui->msg_box->append("Успешно отправлено");
            }
            else {
                 ui->msg_box->append("Ошибка сети");
            }
    }
    prDialog_put_humidity_table->setValue(100);
    prDialog_put_humidity_table->setLabelText("Рассчет произведен");

}
}

void MainWindow::on_bt_ServerConnect_clicked(void)
{

}

void MainWindow::on_checkBox_stateChanged(int arg1)
{
    if(arg1 == 2)
    {
        connect(ptr_pars_msgcan,&pars_msgcan::sensor_data_to_lcd,this,&MainWindow::write_log);
    }
    else
    {
        disconnect(ptr_pars_msgcan,&pars_msgcan::sensor_data_to_lcd,this,&MainWindow::write_log);
    }

}

void MainWindow::write_log(QString id,QString type_data,QString data)
{

    QDateTime date_time_current;
    QDateTime date_time_stamp;
    QString s_date_time,time_file;
    QStringList data_split = data.split("-");
    QString data_sensor = data_split[4] + data_split[5] + "\r";
    date_time_stamp = date_time_current.currentDateTime();
    time_file = s_date_time = date_time_stamp.toString(Qt::ISODateWithMs);




    /*откроем файл для записи*/
    if(current_naame_file == "")
    {
        file_name_from_graph = current_naame_file = create_name_file_log();
        current_naame_file = QCoreApplication::applicationDirPath()+"/log/" + current_naame_file;
        QFile l_file(current_naame_file);




        l_file.open(QFile::WriteOnly| QFile::Append);
        QTextStream stream_log(&l_file);
        s_date_time.append(" ");
        s_date_time.append(id);
        s_date_time.append(" ");
        s_date_time.append(type_data);
        s_date_time.append(" ");
        s_date_time.append(data_split[4]);
        s_date_time.append(data_split[5]);
        s_date_time.append("\r\n");
        /*запишем принятое сообщение в файл*/
        stream_log << s_date_time;
        /**/
        l_file.flush();
        /*закроем файл*/
        l_file.close();
    }
    else {
         QFile l_file(current_naame_file);
       //  if(l_file.size() >= 1048576)
         if(l_file.size() >= 524288)
         {
             if(send_file_flag)
             {
                 /*отправим сигнал слоту отправки на сервер*/
                emit send_file_server_h(current_naame_file);
             }

         file_name_from_graph = current_naame_file = create_name_file_log();
         current_naame_file = QCoreApplication::applicationDirPath()+"/log/" + current_naame_file;
         QFile l_file(current_naame_file);


         l_file.open(QFile::WriteOnly| QFile::Append);
         QTextStream stream_log(&l_file);

         s_date_time.append(" ");
         s_date_time.append(id);
         s_date_time.append(" ");
         s_date_time.append(type_data);
         s_date_time.append(" ");
         s_date_time.append(data_split[4]);
         s_date_time.append(data_split[5]);
         s_date_time.append("\r\n");
         //stream_log <<  msg_buff[i];//one_msg;
         /*запишем принятое сообщение в файл*/
         stream_log << s_date_time;
         /**/
         l_file.flush();
         /*закроем файл*/
         l_file.close();
    }
    else {


        l_file.open(QFile::WriteOnly| QFile::Append);
        QTextStream stream_log(&l_file);

        s_date_time.append(" ");
        s_date_time.append(id);
        s_date_time.append(" ");
        s_date_time.append(type_data);
        s_date_time.append(" ");
        s_date_time.append(data_split[4]);
        s_date_time.append(data_split[5]);
        s_date_time.append("\r\n");
        //stream_log <<  msg_buff[i];//one_msg;
        /*запишем принятое сообщение в файл*/
        stream_log << s_date_time;
        /**/
        l_file.flush();
        /*закроем файл*/
        l_file.close();
    }
    }
        //signal_pars_data_temp_hum(QString id,QString type_data,QString data,QString time,QString name_file,Sensor* p_sens);
        emit signal_pars_data_temp_hum(id,type_data,data_sensor,time_file,file_name_from_graph,p_sens);
}

void MainWindow::on_pb_connect_server_clicked(void)
{
    bool result;

    result = get_state_net();

    if(result)
    {
        ui->pb_connect_server->setStyleSheet("background-color:green");
        ui->msg_box->append("Сервер активен");}
    else {
        ui->msg_box->append("Сервер не активен");
    }

}

void MainWindow::on_cb_send_to_server_stateChanged(int arg1)
{
   if(arg1 == 2)
   {
       send_file_flag = true;
   }
   else {
       send_file_flag = false;
   }
}

void MainWindow::send_file_server_h(QString name_file)
{
    qDebug() << "update...";

                log_to_server_file = new QFile(name_file);
                QFileInfo file_info(*log_to_server_file);
                QString path_to_server = "ftp://91.122.55.62/kubanfolder/";
                QString name_file_upload = "";
                QUrl server = (path_to_server + file_info.fileName());
                server.setUserName("kuban");
                server.setPassword("197198");
                server.setPort(21);

                if(log_to_server_file->open(QIODevice::ReadOnly))
                {
                    QNetworkReply *reply = network_meneger->put(QNetworkRequest(server),log_to_server_file);
                    if(reply->error() == QNetworkReply::NoError)
                    {
                         ui->msg_box->append("Успешно отправлено");
                    }
                    else {
                         ui->msg_box->append("Ошибка сети");
                    }
                }
                else
                {
                      ui->msg_box->append("Невозможно открыть файл");
                }
}

bool MainWindow::get_state_net(void)
{
    QNetworkAccessManager nam;
    //QNetworkRequest req(QUrl("http://www.google.com"));
     QNetworkRequest req(QUrl(CHECK_NET));
    QNetworkReply *reply = nam.get(req);
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    if(reply->bytesAvailable())
    {
        QMessageBox::information(this, "Info", "Соединение с сервером установлено");
        ui->cb_send_to_server->setDisabled(false);
        return true;
    }

    else
    {
    QMessageBox::critical(this, "Info", "Нет соединения с сервером");
     return false;
    }
}

QString MainWindow::create_name_file_log(void)
{
    QString new_name_file;
    //получим текущее время
    QDateTime date_time_current;
    QDateTime date_time_stamp;
    QString s_date_time;

    date_time_stamp = date_time_current.currentDateTime();
    s_date_time = date_time_stamp.toString(Qt::ISODateWithMs);
    //создадим имя файла
    QStringList data_split_1 = s_date_time.split("-");
    QStringList data_split_2 = data_split_1[2].split(":");
    QStringList data_split_3 = data_split_2[2].split(".");

    //new_name_file = addr_dir_files_log;
    new_name_file.append(data_split_1[0]);
    new_name_file.append(data_split_1[1]);
    new_name_file.append(data_split_2[0]);
    new_name_file.append(data_split_2[1]);
    new_name_file.append(data_split_3[0]);
    new_name_file.append(data_split_3[1]);
    new_name_file.append(".log");
    return new_name_file;
}

void MainWindow::on_pb_can_baudrate_set_clicked(void)
{
    QString data_msg;
    data_msg.append("AT+CANSPD:");
    data_msg.append("1");
    data_msg.append(baudrate_can);
    data_msg.append("\r\n");

    emit send_msg_serial(data_msg);
    timer_cnct_gadget->setInterval(1000);
    timer_cnct_gadget->start();
}

void MainWindow::msg_gadget_slot(QString type_msg)
{
    timer_cnct_gadget->stop();
    if(type_msg == "SC_OK")
    {
      QMessageBox::information(this,tr("Сообщение"),tr("Скорость установлена"));
    }
    if(type_msg == "SC_ER")
    {
      QMessageBox::information(this,tr("Сообщение"),tr("Ошибка"));
    }
    if(type_msg == "NO_CNCT")
    {
      QMessageBox::warning(this,tr("Сообщение"),tr("Нет связи с устройством"));
    }

}

void MainWindow::cnct_gadget_timer_slot(void)
{
    msg_gadget_slot("NO_CNCT");
}

void MainWindow::on_cb_can_baudrate_currentIndexChanged(int index)
{
    baudrate_can = baudrate_litera[index];
}
/*обработаем выбор вкладки*/
void MainWindow::on_tabWidget_currentChanged(int index)
{
 QString file_curr_id;
  //  qDebug() << "click" << index;

    if(index == 1)
    {
        /*если график не создан*/
        if(!init_plot_temperature)
        {
            ui->temperature_plot->setInteraction(QCP::iRangeZoom,true);
            ui->temperature_plot->setInteraction(QCP::iRangeDrag,true);
            ui->temperature_plot->axisRect()->setRangeDrag(Qt::Horizontal);
            ui->temperature_plot->axisRect()->setRangeZoom(Qt::Horizontal);
            ui->temperature_plot->xAxis->setTickLabelType(QCPAxis::ltNumber);//ltDateTime
          //  ui->temperature_plot->xAxis->setDateTimeFormat("hh:mm:ss.zzz");//Qt::ISODateWithMs
            //ui->temperature_plot->xAxis->setDateTimeFormat("YYYY-MM-DDTHH:mm:ss.zzz");
            ui->temperature_plot->xAxis->setAutoTickStep(true);

            ui->temperature_plot->yAxis->setRange(-10,50);

            ui->temperature_plot->xAxis2->setVisible(true);
            ui->temperature_plot->yAxis2->setVisible(true);
            ui->temperature_plot->xAxis2->setTicks(false);
            ui->temperature_plot->yAxis2->setTicks(false);
            ui->temperature_plot->xAxis2->setTickLabels(false);
            ui->temperature_plot->yAxis2->setTickLabels(false);

            ui->temperature_plot->addGraph();

            ui->temperature_plot->graph(0)->setPen(QPen(QColor(Qt::red)));//(QPen(QColor(255, 110, 40)));
            ui->temperature_plot->graph(0)->setLineStyle(QCPGraph::lsLine);

            ui->temperature_plot->replot();

            init_plot_temperature = true;
        }
        /*если combo_box не пуст*/
        if(ui->combo_id_temp->count() != 0)
        {
            //QDir d_data_dir =  QCoreApplication::applicationDirPath();
            QStringList log_dir_list;
            /*произведем вывод данных на график*/
            QString curr_id = ui->combo_id_temp->currentText();
            /*проверим есть ли класс с таким именем*/
            if(list_id_plot.count()!= 0)
            {
                /*произведем поиск по списку ID*/
                  QList<QString>::ConstIterator Iter = std::find(list_id_plot.begin(),list_id_plot.end(),curr_id);
                  /*класс с таким именем создан выведем его данные в график*/
                  if(Iter != list_id_plot.end())
                  {
                      data_sensor_plot* temp_sens_ptr = new  data_sensor_plot();
                      int idx = list_id_plot.indexOf(*Iter);
                      temp_sens_ptr = ptr_data_sensor[idx];
                     /*получим ссылку на его содержимое*/
                      emit signal_data_to_plot(temp_sens_ptr->temp_x,temp_sens_ptr->temp_y,TEMPERATURE);
                  }
                  /*иначе создадим новый класс и передадим на него указатель*/
                  else
                  {
                      data_sensor_plot* temp_sens_ptr = new  data_sensor_plot();
                      /*получим содержимое папки и внесем его в поля класса*/
                      /*прочитаем данные для этого ID*/
                      QDir d_data_dir = QCoreApplication::applicationDirPath()+"/data/temperature/"+ curr_id;
                      /*получим список файлов*/
                      /*проверим содержимое папки на наличие файлов *.log */
                      log_dir_list = d_data_dir.entryList(QStringList() << "*.log",QDir::Files);
                      /*переберем все файлы прочитав построчно*/
                      for(int i = 0; i < log_dir_list.count(); i++)
                      {
                          file_curr_id =  QCoreApplication::applicationDirPath()+"/data/temperature/"+ curr_id+"/" + log_dir_list[i];

                          QFile log_file(file_curr_id);

                          if(log_file.open(QFile::ReadOnly|QIODevice::ReadOnly))
                          {
                              QTextStream stream_log(&log_file);
                              /*прочитаем файл до конца*/
                              while(!log_file.atEnd())
                              {
                                  QString line = log_file.readLine();
                                  QStringList split_line = line.split(" ");
                                  QStringList without_cr = split_line[1].split("/");
                                  temp_sens_ptr->temp_x.push_back(split_line[0]);
                                  temp_sens_ptr->temp_y.push_back(without_cr[0].toDouble());
                              }
                              /**/
                              log_file.flush();
                              /*закроем файл*/
                              log_file.close();
                          }
                      }
                      emit signal_data_to_plot(temp_sens_ptr->temp_x,temp_sens_ptr->temp_y,TEMPERATURE);
                      ptr_data_sensor.push_back(temp_sens_ptr);
                      /*внесем в список наш новый ID*/
                      list_id_plot.push_back(curr_id);
                  }
            }
            else
            {
                data_sensor_plot* temp_sens_ptr = new  data_sensor_plot();
                /*получим содержимое папки и внесем его в поля класса*/
                /*прочитаем данные для этого ID*/
                QDir d_data_dir = QCoreApplication::applicationDirPath()+"/data/temperature/"+ curr_id;
                /*получим список файлов*/
                /*проверим содержимое папки на наличие файлов *.log */
                log_dir_list = d_data_dir.entryList(QStringList() << "*.log",QDir::Files);
                /*переберем все файлы прочитав построчно*/
                for(int i = 0; i < log_dir_list.count(); i++)
                {
                    file_curr_id =  QCoreApplication::applicationDirPath()+"/data/temperature/"+ curr_id+"/" + log_dir_list[i];

                    QFile log_file(file_curr_id);

                    if(log_file.open(QFile::ReadOnly|QIODevice::ReadOnly))
                    {
                        QTextStream stream_log(&log_file);
                        /*прочитаем файл до конца*/
                        while(!log_file.atEnd())
                        {
                            QString line = log_file.readLine();
                            QStringList split_line = line.split(" ");
                            QStringList without_cr = split_line[1].split("\r");
                            temp_sens_ptr->temp_x.push_back(split_line[0]);
                            temp_sens_ptr->temp_y.push_back(without_cr[0].toDouble());
                        }
                        /**/
                        log_file.flush();
                        /*закроем файл*/
                        log_file.close();
                    }
                }
                emit signal_data_to_plot(temp_sens_ptr->temp_x,temp_sens_ptr->temp_y,TEMPERATURE);
                ptr_data_sensor.push_back(temp_sens_ptr);
                /*внесем в список наш новый ID*/
                list_id_plot.push_back(curr_id);
            }
        }
    }
    else if(index == 2)
    {
        if(!init_plot_hum)
        {
            ui->hum_plot->setInteraction(QCP::iRangeZoom,true);
            ui->hum_plot->setInteraction(QCP::iRangeDrag,true);
            ui->hum_plot->axisRect()->setRangeDrag(Qt::Horizontal);
            ui->hum_plot->axisRect()->setRangeZoom(Qt::Horizontal);
            ui->hum_plot->xAxis->setTickLabelType(QCPAxis::ltNumber);
            //ui->hum_plot->xAxis->setTickLabelType(QCPAxis::ltDateTime);
           // ui->hum_plot->xAxis->setDateTimeFormat("hh:mm:ss.zzz");
            //ui->hum_plot->xAxis->setDateTimeFormat("YYYY-MM-DDTHH:mm:ss.zzz");
            ui->hum_plot->xAxis->setAutoTickStep(true);

            ui->hum_plot->yAxis->setRange(0,100);

            ui->hum_plot->xAxis2->setVisible(true);
            ui->hum_plot->yAxis2->setVisible(true);
            ui->hum_plot->xAxis2->setTicks(false);
            ui->hum_plot->yAxis2->setTicks(false);
            ui->hum_plot->xAxis2->setTickLabels(false);
            ui->hum_plot->yAxis2->setTickLabels(false);

            ui->hum_plot->addGraph();

            ui->hum_plot->graph(0)->setPen(QPen(QColor(Qt::red)));//(QPen(QColor(255, 110, 40)));
            ui->hum_plot->graph(0)->setLineStyle(QCPGraph::lsLine);

            ui->hum_plot->replot();

            init_plot_hum = true;
        }
        /*произведем вывод данных на график*/
    }
}
/*функция подготовки данных для графиков*/
void MainWindow::create_data_path(void)
{
    QApplication::processEvents();
    QStringList all_dir;
    QStringList log_dir_list;
    QDir log_path;
      /*получим ссылку на основную директорию*/
      QDir prog_path =  QCoreApplication::applicationDirPath();
      /*проверим содержимое главной директории на наличие папки /log */
      QStringList general_dir_list = prog_path.entryList(QStringList() << "log",QDir::Dirs);
      /*проверим есть ли папка LOG в главной директории*/
      if(general_dir_list.count() != 0)
      {
           log_path =  QCoreApplication::applicationDirPath()+"/log";
           /*проверим содержимое папки на наличие файлов *.log */
           log_dir_list = log_path.entryList(QStringList() << "*.log",QDir::Files);
           if(log_dir_list.count() != 0)
           {
               /*получим ссылку на основную директорию*/
               prog_path =  QCoreApplication::applicationDirPath();
               /*проверим содержимое главной директории на наличие папки /data */
               all_dir = prog_path.entryList(QStringList() << "data",QDir::Dirs);
               /*если папки DATA нет,то создадим ее и ее содержимое*/
               if(all_dir.count() == 0)
               {
                       QDir d_data_dir =  QCoreApplication::applicationDirPath();
                       QString s_data_dir = QCoreApplication::applicationDirPath()+"/data";
                       /* создадим папку /data */
                       if(d_data_dir.mkdir(s_data_dir) == true)
                       {
                          /*создадим папки температура и влажность*/

                           s_data_dir = QCoreApplication::applicationDirPath()+"/data/temperature";
                           d_data_dir.mkdir(s_data_dir);
                           s_data_dir = QCoreApplication::applicationDirPath()+"/data/humidity";
                           d_data_dir.mkdir(s_data_dir);
                           /*создадим поток для файла*/
                              QDateTime date_time_current;
                              QDateTime date_time_stamp;
                              QString s_date_time;
                              QString addr_file_log_string;
                              // QFile log_file;
                           /*переберем все файлы и разберем их построчно*/
                           for(int i = 0;i < log_dir_list.count();i++ )
                           {
                               addr_file_log_string =  QCoreApplication::applicationDirPath()+"/log/" + log_dir_list[i];

                               QFile log_file(addr_file_log_string);

                               if(log_file.open(QFile::ReadOnly|QIODevice::ReadOnly))
                               {
                                   QTextStream stream_log(&log_file);
                                   /*прочитаем файл до конца*/
                                   while(!log_file.atEnd())
                                   {
                                       QString line = log_file.readLine();
                                       QStringList split_line = line.split(" ");
                                        //(QString id,QString type_data, QString data, QString time,QString name_file)
                                       emit signal_pars_data_temp_hum(split_line[1],split_line[2],split_line[3],split_line[0],log_dir_list[i],p_sens);

                                   }
                                   /**/
                                   log_file.flush();
                                   /*закроем файл*/
                                   log_file.close();
                               }

                           }
                       }
               }
               else {
                   /*иначе произведем разбор по ID и заполним комбобоксы*/
                   /*получим ссылку на основную директорию*/
                   prog_path = QCoreApplication::applicationDirPath()+"/data";
                   /*проверим содержимое главной директории на наличие папки /data */
                   all_dir = prog_path.entryList(QStringList() << "temperature",QDir::Dirs);
                   /*получим содержимое папки "temperature"*/
                   if(all_dir.count() != 0)
                   {
                       QStringList id_list;

                       prog_path = QCoreApplication::applicationDirPath()+"/data/temperature";

                       /*получим содержимое папки(id)*/
                       id_list = prog_path.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

                       for (int i = 0 ;i < id_list.count();i++) {

                          add_id_to_combobox(id_list[i], TEMPERATURE);
                       }
                   }
                   prog_path = QCoreApplication::applicationDirPath()+"/data";
                   /*проверим содержимое главной директории на наличие папки /data */
                   all_dir = prog_path.entryList(QStringList() << "humidity",QDir::Dirs);
                   /*получим содержимое папки "humidity"*/
                   if(all_dir.count() != 0)
                   {
                       QStringList id_list;

                       prog_path = QCoreApplication::applicationDirPath()+"/data/humidity";

                       /*получим содержимое папки(id)*/
                       id_list = prog_path.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

                       for (int i = 0 ;i < id_list.count();i++) {

                          add_id_to_combobox(id_list[i], HUMYDITI);
                       }
                   }

                   /*получим содержимое папки "влажность"*/

               }
           }
      }
    /*установим признак подготовки данных для графиков*/
    make_data = true;
}

void MainWindow::add_id_to_combobox(QString id, int type_data)
{
    if(type_data == TEMPERATURE)
    {
        ui->combo_id_temp->addItem(id);
    }
    else if (type_data == HUMYDITI)
    {
        ui->combo_id_hum->addItem(id);
    }
}

void MainWindow::set_data_to_plot(QVector<QString> time, QVector<double> data, int type_data)
{
    if(type_data == TEMPERATURE)
    {
        /*если график не создан*/
        if(!init_plot_temperature)
        {
            ui->temperature_plot->setInteraction(QCP::iRangeZoom,true);
            ui->temperature_plot->setInteraction(QCP::iRangeDrag,true);
            ui->temperature_plot->axisRect()->setRangeDrag(Qt::Horizontal);
            ui->temperature_plot->axisRect()->setRangeZoom(Qt::Horizontal);
            ui->temperature_plot->xAxis->setTickLabelType(QCPAxis::ltNumber);//ltDateTime
          //  ui->temperature_plot->xAxis->setDateTimeFormat("hh:mm:ss.zzz");//Qt::ISODateWithMs
            //ui->temperature_plot->xAxis->setDateTimeFormat("YYYY-MM-DDTHH:mm:ss.zzz");
            ui->temperature_plot->xAxis->setAutoTickStep(true);

            ui->temperature_plot->yAxis->setRange(-10,50);

            ui->temperature_plot->xAxis2->setVisible(true);
            ui->temperature_plot->yAxis2->setVisible(true);
            ui->temperature_plot->xAxis2->setTicks(false);
            ui->temperature_plot->yAxis2->setTicks(false);
            ui->temperature_plot->xAxis2->setTickLabels(false);
            ui->temperature_plot->yAxis2->setTickLabels(false);

            ui->temperature_plot->addGraph();

            ui->temperature_plot->graph(0)->setPen(QPen(QColor(Qt::red)));//(QPen(QColor(255, 110, 40)));
            ui->temperature_plot->graph(0)->setLineStyle(QCPGraph::lsLine);

            ui->temperature_plot->replot();

            init_plot_temperature = true;
        }




        ui->temperature_plot->graph(0)->clearData();

        for(uint32_t i = 0; i < time.count(); i++)
        {
            ui->temperature_plot->graph(0)->addData(i,data[i]);//
            //ui->temperature_plot->xAxis->setRange(new_point, 256, Qt::AlignRight);
            ui->temperature_plot->replot();
            ui->temperature_plot->update();
        }

    }
    else if (type_data == HUMYDITI)
    {

    }
}

void MainWindow::on_combo_id_temp_currentIndexChanged(const QString &arg1)
{
     QString file_curr_id;
if(make_data)
{
        QStringList log_dir_list;
        /*произведем вывод данных на график*/
        QString curr_id = arg1;// ui->combo_id_temp->currentText();
        /*проверим есть ли класс с таким именем*/
        if(list_id_plot.count()!= 0)
        {
            /*произведем поиск по списку ID*/
              QList<QString>::ConstIterator Iter = std::find(list_id_plot.begin(),list_id_plot.end(),curr_id);
              /*класс с таким именем создан выведем его данные в график*/
              if(Iter != list_id_plot.end())
              {
                  data_sensor_plot* temp_sens_ptr = new  data_sensor_plot();
                  int idx = list_id_plot.indexOf(*Iter);
                  temp_sens_ptr = ptr_data_sensor[idx];
                 /*получим ссылку на его содержимое*/
                  emit signal_data_to_plot(temp_sens_ptr->temp_x,temp_sens_ptr->temp_y,TEMPERATURE);
              }
              /*иначе создадим новый класс и передадим на него указатель*/
              else
              {
                  data_sensor_plot* temp_sens_ptr = new  data_sensor_plot();
                  /*получим содержимое папки и внесем его в поля класса*/
                  /*прочитаем данные для этого ID*/
                  QDir d_data_dir = QCoreApplication::applicationDirPath()+"/data/temperature/"+ curr_id;
                  /*получим список файлов*/
                  /*проверим содержимое папки на наличие файлов *.log */
                  log_dir_list = d_data_dir.entryList(QStringList() << "*.log",QDir::Files);
                  /*переберем все файлы прочитав построчно*/
                  for(int i = 0; i < log_dir_list.count(); i++)
                  {
                      file_curr_id =  QCoreApplication::applicationDirPath()+"/data/temperature/"+ curr_id+"/" + log_dir_list[i];

                      QFile log_file(file_curr_id);

                      if(log_file.open(QFile::ReadOnly|QIODevice::ReadOnly))
                      {
                          QTextStream stream_log(&log_file);
                          /*прочитаем файл до конца*/
                          while(!log_file.atEnd())
                          {
                              QString line = log_file.readLine();
                              QStringList split_line = line.split(" ");
                              QStringList without_cr = split_line[1].split("/");
                              temp_sens_ptr->temp_x.push_back(split_line[0]);
                              temp_sens_ptr->temp_y.push_back(without_cr[0].toDouble());
                          }
                          /**/
                          log_file.flush();
                          /*закроем файл*/
                          log_file.close();
                      }
                  }
                  emit signal_data_to_plot(temp_sens_ptr->temp_x,temp_sens_ptr->temp_y,TEMPERATURE);
                  ptr_data_sensor.push_back(temp_sens_ptr);
                  /*внесем в список наш новый ID*/
                  list_id_plot.push_back(curr_id);
              }
        }
        else
        {
            data_sensor_plot* temp_sens_ptr = new  data_sensor_plot();
            /*получим содержимое папки и внесем его в поля класса*/
            /*прочитаем данные для этого ID*/
            QDir d_data_dir = QCoreApplication::applicationDirPath()+"/data/temperature/"+ curr_id;
            /*получим список файлов*/
            /*проверим содержимое папки на наличие файлов *.log */
            log_dir_list = d_data_dir.entryList(QStringList() << "*.log",QDir::Files);
            /*переберем все файлы прочитав построчно*/
            for(int i = 0; i < log_dir_list.count(); i++)
            {
                file_curr_id =  QCoreApplication::applicationDirPath()+"/data/temperature/"+ curr_id+"/" + log_dir_list[i];

                QFile log_file(file_curr_id);

                if(log_file.open(QFile::ReadOnly|QIODevice::ReadOnly))
                {
                    QTextStream stream_log(&log_file);
                    /*прочитаем файл до конца*/
                    while(!log_file.atEnd())
                    {
                        QString line = log_file.readLine();
                        QStringList split_line = line.split(" ");
                        QStringList without_cr = split_line[1].split("\r");
                        temp_sens_ptr->temp_x.push_back(split_line[0]);
                        temp_sens_ptr->temp_y.push_back(without_cr[0].toDouble());
                    }
                    /**/
                    log_file.flush();
                    /*закроем файл*/
                    log_file.close();
                }
            }
            emit signal_data_to_plot(temp_sens_ptr->temp_x,temp_sens_ptr->temp_y,TEMPERATURE);
            ptr_data_sensor.push_back(temp_sens_ptr);
            /*внесем в список наш новый ID*/
            list_id_plot.push_back(curr_id);
        }
}
}

void MainWindow::slot_save_position_sensor(QString id_sensor, int position_sensor)
{
    QStringList log_dir_list;
    QDir log_path;

    /*получим ссылку на основную директорию*/
    QDir prog_path =  QCoreApplication::applicationDirPath();
    /*проверим содержимое главной директории на наличие папки /position_sensors */
    QStringList general_dir_list = prog_path.entryList(QStringList() << "position_sensors",QDir::Dirs);
    /*проверим есть ли папка position_sensors в главной директории*/
    if(general_dir_list.count() != 0)
    {
         log_path =  QCoreApplication::applicationDirPath()+"/position_sensors";
         /*проверим содержимое папки на наличие файлов *.log */
         log_dir_list = log_path.entryList(QStringList() << "*.log",QDir::Files);
         if(log_dir_list.count() != 0)
         {
            QString s_data_dir = QCoreApplication::applicationDirPath()+"/position_sensors";
            /*создадим файл position_sensors.log*/
            s_data_dir += "/position_sensors.log";
            QFile l_file(s_data_dir);
            QString s_date_time;
            l_file.open(QFile::WriteOnly| QFile::Append);
            QTextStream stream_log(&l_file);
            /*запишем принятое сообщение в файл*/
            l_file.seek((position_sensor * 8 ) - 8);
            stream_log << id_sensor;
            /**/
            l_file.flush();
            /*закроем файл*/
            l_file.close();
         }
         else
         {
             /*создадим файл position_sensors.log*/
             QDir d_data_dir =  QCoreApplication::applicationDirPath();
             QString s_data_dir = QCoreApplication::applicationDirPath()+"/position_sensors";
             /* создадим папку /position_sensors */
             if(d_data_dir.mkdir(s_data_dir) == true)
             {
                 /*создадим файл position_sensors.log*/
                 s_data_dir += "/position_sensors.log";
                 QFile l_file(s_data_dir);
                 QString s_date_time;
                 l_file.open(QFile::WriteOnly| QFile::Append);
                 QTextStream stream_log(&l_file);
                 /*запишем принятое сообщение в файл*/
                 l_file.seek((position_sensor * 8 ) - 8);
                 stream_log << id_sensor;
                 /**/
                 l_file.flush();
                 /*закроем файл*/
                 l_file.close();
             }
         }
     }
    /*создадим папку /position_sensors и файл  position_sensors.log*/
     else
    {
        QDir d_data_dir =  QCoreApplication::applicationDirPath();
        QString s_data_dir = QCoreApplication::applicationDirPath()+"/position_sensors";
        /* создадим папку /position_sensors */
        if(d_data_dir.mkdir(s_data_dir) == true)
        {
            /*создадим файл position_sensors.log*/
            s_data_dir += "/position_sensors.log";
            QFile l_file(s_data_dir);
            QString s_date_time;
            l_file.open(QFile::WriteOnly| QFile::Append);
            QTextStream stream_log(&l_file);
            /*заполним файл пробелами*/
            for(int i = 0; i < 160;i++)
            {
                stream_log << " ";
            }
            /**/
            l_file.flush();
            /*закроем файл*/
            l_file.close();
            /*откроем файл заново*/
            l_file.open(QFile::WriteOnly| QFile::Append);
            /*запишем принятое сообщение в файл*/
            l_file.seek((position_sensor * 8 ) - 8);
            stream_log << id_sensor;
            /**/
            l_file.flush();
            /*закроем файл*/
            l_file.close();
        }
    }


        /*откроем файл "position_sensors.log"*/



    /*в строку с номером позиции впишем номер id*/

    /*конец*/


}

void MainWindow::get_sensor_psitions(void)
{
    QStringList log_dir_list;
    QDir log_path;

    /*получим ссылку на основную директорию*/
    QDir prog_path =  QCoreApplication::applicationDirPath();
    /*проверим содержимое главной директории на наличие папки /position_sensors */
    QStringList general_dir_list = prog_path.entryList(QStringList() << "position_sensors",QDir::Dirs);
    /*проверим есть ли папка position_sensors в главной директории*/
    if(general_dir_list.count() != 0)
    {
         log_path =  QCoreApplication::applicationDirPath()+"/position_sensors";
         /*проверим содержимое папки на наличие файлов *.log */
         log_dir_list = log_path.entryList(QStringList() << "*.log",QDir::Files);
         if(log_dir_list.count() != 0)
         {
            QString id_sensor;
            QString s_data_dir = QCoreApplication::applicationDirPath()+"/position_sensors";
            /*создадим файл position_sensors.log*/
            s_data_dir += "/position_sensors.log";
            QFile l_file(s_data_dir);
            QString s_date_time;
            l_file.open(QFile::ReadOnly);
            QTextStream stream_log(&l_file);
            /*считаем строку с id*/
            id_sensor = l_file.readLine();
QString one_id;
int offset = 0;
for(int i = 0 ; i < 160; i += 8)
{
    //qDebug() << i;
    if(!(id_sensor.count() < (i + 7)))
    {
        offset = i;
        for(int j = offset ; j < (offset + 8);j++)
        {
            if(id_sensor.at(j) != "")
            {
                one_id.append(id_sensor.at(j));
            }
            //qDebug() << offset;
        }
        emit signal_set_id_in_combo_box(one_id,(i / 8));
        qDebug() << one_id;
        qDebug() << i;
        qDebug() << (i/8);
        one_id.clear();
    }
    else
    {
        break;
    }
}

            /**/
            l_file.flush();
            /*закроем файл*/
            l_file.close();
         }
    }
}

void MainWindow::slot_set_id_in_combo_box(QString id_sensor, int position_sensor)
{
    QComboBox* one_combo;
    one_combo = ptr_combo.at(position_sensor);

    IdCombobox(id_sensor);

    one_combo->setCurrentText(id_sensor);


}

void MainWindow::set_ptr_combobox(void)
{
    ptr_combo.append(ui->comboBox_1);
    ptr_combo.append(ui->comboBox_2);
    ptr_combo.append(ui->comboBox_3);
    ptr_combo.append(ui->comboBox_4);
    ptr_combo.append(ui->comboBox_5);
    ptr_combo.append(ui->comboBox_6);
    ptr_combo.append(ui->comboBox_7);
    ptr_combo.append(ui->comboBox_8);
    ptr_combo.append(ui->comboBox_9);
    ptr_combo.append(ui->comboBox_10);
    ptr_combo.append(ui->comboBox_11);
    ptr_combo.append(ui->comboBox_12);
    ptr_combo.append(ui->comboBox_13);
    ptr_combo.append(ui->comboBox_14);
    ptr_combo.append(ui->comboBox_15);
    ptr_combo.append(ui->comboBox_16);
    ptr_combo.append(ui->comboBox_17);
    ptr_combo.append(ui->comboBox_18);
    ptr_combo.append(ui->comboBox_19);
    ptr_combo.append(ui->comboBox_20);

}


