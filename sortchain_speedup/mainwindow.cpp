#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QDateTime>

#include "sortchain.h"

#define HOLTER_MODE   0
#define ECG_MODE      1

uint8_t mode;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 刚开始默认是Holter模式
    mode = HOLTER_MODE;
    ui->checkBox->setChecked(true);
    ui->checkBox_2->setChecked(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

schh_t handle[12];

void MainWindow::on_pushButton_clicked()
{
    QString fileloc = ui->lineEdit->text();    // file location
    QString dfileloc = fileloc;
    uint32_t startline = ui->lineEdit_2->text().toInt();
    uint32_t totalline = ui->lineEdit_3->text().toInt();
    uint32_t buffer_size;

    dfileloc.remove(dfileloc.length() - strlen(".txt"), strlen(".txt"));
    dfileloc.append("__proc.txt");
    QFile dfile(dfileloc);

    if(dfile.exists() == true)
    {
        dfile.remove();
    }

    ui->statusBar->clearMessage();

    // 打开源文件，读取源文件，把源文件的内容处理后存储到目标文件
    QFile file(fileloc);
    if(file.open(QIODevice::ReadOnly) == true)
    {
        char buffer[300];
        QString line;
        QStringList list;
        schdat_t mid[12];
        schdat_t calc[12];
        schres_t res;

        if(mode == HOLTER_MODE)
        {
            buffer_size = 128;
        }
        else
        {
            buffer_size = 2000;
        }
        for(uint32_t i = 0; i < 12; i++)
        {
            sortchain_init(&handle[i], buffer_size, 40);
        }

        // 获取起始时间
        QDateTime startTime = QDateTime::currentDateTime();
        QString startTimeS = startTime.toString("ss");

        for(uint32_t i = 0; i < startline + totalline; i++)
        {
            // 略过不要的行
            if(i < startline)
            {
                continue;
            }

            // 读取一行
            memset(buffer, 0, sizeof(buffer));
            file.readLine(buffer, sizeof(buffer));
            line = QString("%1").arg(buffer);
            list = line.split(',');

            // 求出中值。第一列是时间
            memset(mid, 0, sizeof(mid));
            memset(calc, 0, sizeof(calc));

            ////////////////////////////////////////////////
            for(uint32_t j = 0; j < 12; j++)
            {
                res = sortchain_add(&handle[j], list[j + 1].toFloat(), &mid[j]);
            }
            ////////////////////////////////////////////////

            // 最后的结果等于当前采样值减去中值
            if(res == SCHRES_OK)
            {
                for(unsigned int i = 0; i < 12; i++)
                {
                    calc[i] = list[i + 1].toFloat() - mid[i];
                    /// calc[i] =  mid[i];
                }
            }
            else
            {
                for(unsigned int i = 0; i < 12; i++)
                {
                    calc[i] = 0;
                }
            }

            // 存到新的文件
            if(dfile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
            {
                QTextStream in(&dfile);
                in << list[0] << ",";
                in << calc[0] << "," << calc[1] << "," << calc[2] << "," << calc[3] << "," \
                   << calc[4] << "," << calc[5] << "," << calc[6] << "," << calc[7] << "," \
                   << calc[8] << "," << calc[9] << "," << calc[10] << "," << calc[11] << "\n";
            }
            dfile.close();
        }

        // 获取结束时间
        QDateTime endTime = QDateTime::currentDateTime();
        QString endTimeS = endTime.toString("ss");

        int intervalTime = endTimeS.toInt() - startTimeS.toInt();
        ui->statusBar->showMessage(QString::number(intervalTime) + "S", 30000);
    }
    file.close();
}

void MainWindow::on_checkBox_clicked()
{
    mode = HOLTER_MODE;
    ui->checkBox->setChecked(true);
    ui->checkBox_2->setChecked(false);
}

void MainWindow::on_checkBox_2_clicked()
{
    mode = ECG_MODE;
    ui->checkBox->setChecked(false);
    ui->checkBox_2->setChecked(true);
}


