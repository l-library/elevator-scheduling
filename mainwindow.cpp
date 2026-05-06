#include "mainwindow.h"
#include <QGridLayout>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("电梯调度模拟程序");
    setWindowState(Qt::WindowMaximized);

    // 创建并设置网格布局（20*15）
    m_grid_layout = new QGridLayout(this);
    for (int i = 0; i < 20; ++i)
    {
        m_grid_layout->setRowStretch(i, 1); // 将行均分为20份
    }
    for (int i = 0; i < 15; ++i)
    {
        m_grid_layout->setColumnStretch(i, 1); // 将列均分为15份
    }
    centralWidget()->setLayout(m_grid_layout);

    initInterface();
}

void MainWindow::initInterface()
{
}

MainWindow::~MainWindow()
{
}
