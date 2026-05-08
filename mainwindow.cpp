#include "mainwindow.h"
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("电梯调度模拟程序");
    setWindowState(Qt::WindowMaximized);

    // 创建并设置网格布局（20*15）
    // 初始化buttonChecked数组
    m_grid_layout = new QGridLayout(this);
    for (int i = 0; i < HEIGHT; ++i)
    {
        m_grid_layout->setRowStretch(i, 1); // 将行均分为20份
        // 初始化按钮状态
        buttonChecked[i][0] = 0;
        buttonChecked[i][1] = 0;
    }
    for (int i = 0; i < 15; ++i)
        m_grid_layout->setColumnStretch(i, 1); // 将列均分为15份
    m_grid_layout->setSpacing(0);
    m_grid_layout->setContentsMargins(0, 0, 0, 0);
    auto central_widget = new QWidget(this);
    central_widget->setContentsMargins(0, 0, 0, 0);
    central_widget->setLayout(m_grid_layout);
    setCentralWidget(central_widget);

    // 初始化电梯位置
    for (int i = 0; i < ELEVATOR_NUM; ++i)
        elevator[i] = 1;

    initInterface();

    // 连接按钮信号
    connect(this, &MainWindow::upDownButtonChanged, [this](int index, int direction)
            { if(direction==0)upButton[index]->setChecked(buttonChecked[index][direction] == true);
            else downButton[index]->setChecked(buttonChecked[index][direction] == true); });
}

void MainWindow::initInterface()
{
    // 初始化左侧楼层视图
    for (int i = 0; i < HEIGHT; ++i)
    {
        QPushButton *up = new QPushButton(this);
        up->setIcon(QIcon(":/asset/up.png"));
        m_grid_layout->addWidget(up, i, 0);
        up->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        up->setCheckable(true);
        upButton.append(up);

        QPushButton *down = new QPushButton(this);
        down->setIcon(QIcon(":/asset/down.png"));
        m_grid_layout->addWidget(down, i, 1);
        down->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        down->setCheckable(true);
        downButton.append(down);

        // 绑定事件
        connect(up, &QPushButton::toggled, [i, this](bool checked)
                { buttonChecked[i][0] = !buttonChecked[i][0];
                    emit upDownButtonChanged(i,0); });
        connect(down, &QPushButton::toggled, [i, this](bool checked)
                {   buttonChecked[i][1] = !buttonChecked[i][1]; 
                    emit upDownButtonChanged(i,1); });
    }

    // 初始化可视化电梯
    QVector<QColor> colors = {QColor(52, 152, 219), QColor(46, 204, 113), QColor(241, 196, 15),
                              QColor(155, 89, 182), QColor(231, 76, 60)};
    for (int i = 0; i < ELEVATOR_NUM; ++i)
    {
        auto label = new QLabel(this);
        label->setText(QString("电梯%1").arg(i + 1));
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet(
            QString("background-color: %1; color: white; font-size: 14px; font-weight: bold; "
                    "border-radius: 4px;")
                .arg(colors[i].name()));
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        elevatorLabels.append(label);
    }
    updateElevatorDisplay();
}

void MainWindow::updateElevatorDisplay()
{
    for (int i = 0; i < ELEVATOR_NUM; ++i)
    {
        m_grid_layout->removeWidget(elevatorLabels[i]);
    }
    for (int i = 0; i < ELEVATOR_NUM; ++i)
    {
        int row = HEIGHT - elevator[i];
        int col = 2 + i;
        m_grid_layout->addWidget(elevatorLabels[i], row, col);
        elevatorLabels[i]->show();
    }
}

MainWindow::~MainWindow()
{
}
