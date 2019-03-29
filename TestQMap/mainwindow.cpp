#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_map.clear();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    m_map["66666"] = "9999";
    qDebug()<<"m_map.size(): "<<m_map.size();
    qDebug()<<"m_map.key(\"66666\").size(): "<<m_map.key("66666").size();
    qDebug()<<"m_map.value(\"66666\"): "<<m_map.value("66666");
}
