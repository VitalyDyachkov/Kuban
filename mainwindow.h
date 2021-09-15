#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#define TEMPERATURE 1
#define HUMYDITI 2

#include <QDebug>
#include <QMainWindow>
#include <QLCDNumber>
#include <QPalette>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <mserialport.h>
#include <pars_msgcan.h>
#include "data_to_graph.h"
#include <QThread>
#include <QMessageBox>
#include <QProgressDialog>
#include "sensor.h"
#include "senslcd.h"
#include <QFile>
#include <QTextStream>
#include <QIODevice>
#include <QDir>
#include <QDateTime>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QFileDialog>
#include<QMediaPlayer>
#include <QCoreApplication>
#include <QtSql>
#include <QSqlDatabase>
#include <data_sensor_plot.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    #define CHECK_NET "http://109.172.13.50"
    #define SERVER_ADDR "ftp://109.172.13.50/kubanfolder/"
    #define CLIENT_NAME "kuban"
    #define PASSWORD "197198"
    QVector<double> termistor_table = {
       0.2488,0.2986,0.3603,0.4369,0.5327,0.6531,0.8057,1.0,1.249,1.571,1.99,2.539,
       3.265,4.232,5.533,7.293,9.707,13.04,17.7,24.26,33.65,47.17,67.01,96.3};

    QVector<double> temperature_table = {
       60.0,55.0,50.0,45.0,40.0,35.0,30.0,25.0,20.0,15.0,10.0,5.0,0.0,-5.0,-10.0,
       -15.0,-20.0,-25.0,-30.0,-35.0,-40.0,-45.0,-50.0,-55.0};

    QVector<float> v_in_humidity_table;
    QVector<float>humidity_table = {0.0,
                                   2.5,
                                   5.0,
                                   7.5,
                                   10.0,
                                   12.5,
                                   15.0,
                                   17.5,
                                   20.0,
                                   22.5,
                                   25.0,
                                   27.5,
                                   30.0,
                                   32.5,
                                   35.0,
                                   37.5,
                                   40.0,
                                   42.5,
                                   45.0,
                                   47.5,
                                   50.0,
                                   52.5,
                                   55.0,
                                   57.5,
                                   60.0,
                                   62.5,
                                   65.0,
                                   67.5,
                                   70.0,
                                   72.5,
                                   75.0,
                                   77.5,
                                   80.0,
                                   82.5,
                                   85.0,
                                   87.5,
                                   90.0,
                                   92.5,
                                   95.0,
                                   97.5,
                                   100};
    bool h_table_create = false ;
    bool send_file_flag = false;
    QString baudrate_can = "G";
    QString baudrate_litera[3] = {"G","H","J"};
    QString current_naame_file;
    QString file_name_from_graph;
    QString addr_dir_files_log = ":/";
    QString addr_dir_files_table;
    QString name_send_files_log;
    bool make_data = false;
private slots:
    void OutPorts(void);
    void IdCombobox(QString id);
    void MakeDataForSensor(QString id,QString type_data,QString data);
    void on_bt_SerialOpen_clicked(void);
    void on_pushButton_1sensors_clicked(void);

    void createDialogBox(void);

    void on_pushButton_2sensors_clicked();
    void lcd_set_data(QString data,QLCDNumber *lcd_n);
    bool create_ptr_lcd(QString id,QLCDNumber *lcd_temperature,QLCDNumber *lcd_humidity);
    void delete_ptr_lcd(QString id);
    void on_pushButton_3sensors_clicked();
    void set_palette();
    void send_file_server_h(QString name_file);
    bool get_state_net();
    QString create_name_file_log(void);

    void on_pushButton_4sensors_clicked();

    void on_pushButton_5sensors_clicked();

    void on_pushButton_6sensors_clicked();

    void on_pushButton_7sensors_clicked();

    void on_pushButton_8sensors_clicked();

    void on_pushButton_9sensors_clicked();

    void on_pushButton_10sensors_clicked();

    void on_pushButton_11sensors_clicked();

    void on_pushButton_12sensors_clicked();

    void on_pushButton_13sensors_clicked();

    void on_pushButton_14sensors_clicked();

    void on_pushButton_15sensors_clicked();

    void on_pushButton_16sensors_clicked();

    void on_pushButton_17sensors_clicked();

    void on_pushButton_18sensors_clicked();

    void on_pushButton_19sensors_clicked();

    void on_pushButton_20sensors_clicked();

    void on_pushButton_clicked();

    void on_humidity_zero_clicked();

    void on_humidity_hundred_clicked();

    void on_humidity_calc_table_clicked();

    void on_bt_ServerConnect_clicked();

    void on_checkBox_stateChanged(int arg1);
    void write_log(QString id,QString type_data,QString data);
    void on_pb_connect_server_clicked();

    void on_cb_send_to_server_stateChanged(int arg1);


    void on_pb_can_baudrate_set_clicked();

    void on_cb_can_baudrate_currentIndexChanged(int index);
    void msg_gadget_slot(QString type_msg);
    void cnct_gadget_timer_slot(void);

    void on_tabWidget_currentChanged(int index);

    void create_data_path();
    void add_id_to_combobox(QString id,int type_data);

    void set_data_to_plot(QVector<QString>time,QVector<double>data,int type_data);


    void on_combo_id_temp_currentIndexChanged(const QString &arg1);
    void slot_save_position_sensor(QString id_sensor, int position_sensor);
    void get_sensor_psitions(void);
    void slot_set_id_in_combo_box(QString id_sensor, int position_sensor);
    void set_ptr_combobox(void);
    void on_combo_id_temp_currentIndexChanged(int index);

signals:
    void signal_set_id_in_combo_box(QString id_sensor, int position_sensor);
    void signal_save_position_sensor(QString id_sensor, int position_sensor);
    void open_serial_port(QString name_port);
    void close_serial_port(void);
    void signal_temperature(QString data);
    void signal_humidity(QString data);
    void signal_log(QString id,QString type_data,QString data);
    void send_msg_serial(QString data);
    void send_file_to_server(QString name_file);
    void signal_pars_data_temp_hum(QString id,QString type_data,QString data,QString time,QString name_file,Sensor* p_sens);
    void signal_data_to_plot(QVector<QString>time,QVector<double>data,int type_data);
private:
    Ui::MainWindow *ui;
    bool init_plot_temperature = false;
    bool init_plot_hum = false;
    QPalette palette_lcd;
    mSerialPort* m_serial;
    pars_msgcan *ptr_pars_msgcan;
    data_to_graph *dt_graph;
    QList<QComboBox*> ptr_combo;
    QList<Sensor*> ptr_sensor;       
    QMessageBox msgBox_put_sensor;
    QList<Sensor*> p_sens_lcd;
    QList<data_sensor_plot*> ptr_data_sensor;
    QList<QString>list_id_plot;
    float humidity_v_in_0 = 0;
    float humidity_v_in_100 = 0;
   // QFile* log_file;
    QProgressDialog* prDialog_put_humidity_table;
    Sensor* p_sens;
    QFile* log_to_server_file;
    QNetworkAccessManager *network_meneger;
    QTimer* timer_send_to_server;
    QTimer *timer_cnct_gadget;
    QVector<float>temperature_x,temperature_y,humy_x,humy_y;


    QThread thread_serial;
    QThread thread_pars_mg_can;
    QThread set_plots;
};

#endif // MAINWINDOW_H
