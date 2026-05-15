#include "mainwindow.h"

#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QLCDNumber>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QFrame>
#include <QIntValidator>
#include <QListWidget>

namespace
{

    const QVector<QColor> kElevatorColors = {
        QColor(52, 152, 219),
        QColor(46, 204, 113),
        QColor(241, 196, 15),
        QColor(155, 89, 182),
        QColor(231, 76, 60),
    };

}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("电梯调度模拟程序");
    setWindowState(Qt::WindowMaximized);

    // 初始化各数组
    for (int i = 0; i < ELEVATOR_NUM; ++i)
    {
        m_elevator[i] = 1;
        m_elevator_direction[i] = Direction::stop;
    }

    for (int i = 0; i < HEIGHT + 1; ++i)
    {
        m_button_checked[i][0] = false;
        m_button_checked[i][1] = false;
        m_external_assignment[i][0] = -1;
        m_external_assignment[i][1] = -1;
    }

    m_up_button.resize(HEIGHT);
    m_down_button.resize(HEIGHT);
    for (int i = 0; i < HEIGHT; ++i)
    {
        m_up_button[i] = nullptr;
        m_down_button[i] = nullptr;
    }

    auto central_widget = new QWidget(this);
    auto main_layout = new QHBoxLayout(central_widget);
    main_layout->setSpacing(12);
    main_layout->setContentsMargins(10, 10, 10, 10);

    auto left_widget = new QWidget(this);
    m_grid_layout = new QGridLayout(left_widget);
    m_grid_layout->setSpacing(2);
    m_grid_layout->setContentsMargins(5, 5, 5, 5);
    for (int i = 0; i < HEIGHT; ++i)
        m_grid_layout->setRowStretch(i, 1);
    for (int i = 0; i < 7; ++i)
        m_grid_layout->setColumnStretch(i, 1);
    left_widget->setLayout(m_grid_layout);

    auto separator = new QFrame(this);
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Sunken);

    auto right_widget = new QWidget(this);
    auto right_layout = new QHBoxLayout(right_widget);
    right_layout->setSpacing(8);
    for (int i = 0; i < ELEVATOR_NUM; ++i)
    {
        auto panel = createElevatorPanel(i);
        right_layout->addWidget(panel);
    }
    right_widget->setLayout(right_layout);

    main_layout->addWidget(left_widget, 3);
    main_layout->addWidget(separator);
    main_layout->addWidget(right_widget, 2);
    central_widget->setLayout(main_layout);
    setCentralWidget(central_widget);

    initInterface();

    connect(this, &MainWindow::upDownButtonChanged,
            this, [this](int index, int direction)
            {
                if (direction == 0)
                    m_up_button[index]->setChecked(m_button_checked[index][direction]);
                else
                    m_down_button[index]->setChecked(m_button_checked[index][direction]); });

    // 内呼请求直接加入目标集合
    connect(this, &MainWindow::floorRequested, this, [this](int elevIdx, int floor)
            {
    m_elevator_targets[elevIdx].insert(floor);
    // 如果电梯此时停止，则根据请求的方向设置电梯方向
    if (m_elevator_direction[elevIdx] == Direction::stop) {
        if (floor > m_elevator[elevIdx]) m_elevator_direction[elevIdx] = Direction::up;
        else if (floor < m_elevator[elevIdx]) m_elevator_direction[elevIdx] = Direction::down;
    } });

    // 定时器更新电梯移动
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]()
            {
        updateElevators();
        updateElevatorDisplay(); });
    timer->start(500);
}

QWidget *MainWindow::createElevatorPanel(int elevatorIndex)
{
    auto color = kElevatorColors[elevatorIndex];

    auto groupBox = new QGroupBox(
        QString("  电梯 %1 内部  ").arg(elevatorIndex + 1), this);
    groupBox->setStyleSheet(QString(
                                "QGroupBox {"
                                "   font-weight: bold; font-size: 14px;"
                                "   border: 2px solid %1;"
                                "   border-radius: 8px;"
                                "   margin-top: 14px;"
                                "   padding: 12px 8px 8px 8px;"
                                "}"
                                "QGroupBox::title {"
                                "   subcontrol-origin: margin;"
                                "   subcontrol-position: top center;"
                                "   color: %1;"
                                "   padding: 0 6px;"
                                "}")
                                .arg(color.name()));

    auto layout = new QVBoxLayout(groupBox);
    layout->setSpacing(6);

    auto lcd = new QLCDNumber(this);
    lcd->setDigitCount(2);
    lcd->setSegmentStyle(QLCDNumber::Flat);
    lcd->display(m_elevator[elevatorIndex]);
    lcd->setMinimumHeight(70);
    lcd->setStyleSheet(
        "QLCDNumber {"
        "   background-color: #0d1b2a;"
        "   color: #00ff88;"
        "   border: 2px solid #1b2838;"
        "   border-radius: 6px;"
        "}");
    layout->addWidget(lcd);
    m_lcd_displays.append(lcd);

    auto dirLabel = new QLabel("● 停止", this);
    dirLabel->setAlignment(Qt::AlignCenter);
    dirLabel->setStyleSheet(
        "font-size: 18px; font-weight: bold; padding: 4px;"
        "color: #95a5a6;");
    layout->addWidget(dirLabel);
    m_direction_indicators.append(dirLabel);

    layout->addSpacing(8);

    auto inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(6);

    auto floorLabel = new QLabel("目标楼层:", this);
    floorLabel->setStyleSheet("font-size: 13px; font-weight: bold;");
    inputLayout->addWidget(floorLabel);

    auto lineEdit = new QLineEdit(this);
    lineEdit->setPlaceholderText("1-20");
    lineEdit->setValidator(new QIntValidator(1, HEIGHT, this));
    lineEdit->setMaxLength(2);
    lineEdit->setAlignment(Qt::AlignCenter);
    lineEdit->setStyleSheet(
        QString("QLineEdit {"
                "   padding: 4px;"
                "   border: 2px solid #bdc3c7;"
                "   border-radius: 4px;"
                "   font-size: 14px;"
                "}"
                "QLineEdit:focus {"
                "   border-color: %1;"
                "}")
            .arg(color.name()));
    inputLayout->addWidget(lineEdit);
    m_floor_inputs.append(lineEdit);

    auto confirmBtn = new QPushButton("确认", this);
    confirmBtn->setStyleSheet(
        QString("QPushButton {"
                "   background-color: %1;"
                "   color: white;"
                "   border-radius: 4px;"
                "   padding: 6px 12px;"
                "   font-weight: bold;"
                "   font-size: 13px;"
                "}"
                "QPushButton:hover {"
                "   background-color: %2;"
                "}"
                "QPushButton:pressed {"
                "   background-color: %3;"
                "}"
                "QPushButton:disabled {"
                "   background-color: #95a5a6;"
                "}")
            .arg(color.name(),
                 color.darker(110).name(),
                 color.darker(140).name()));
    confirmBtn->setFixedWidth(60);
    inputLayout->addWidget(confirmBtn);

    layout->addLayout(inputLayout);

    // 确认按钮和回车触发楼层请求信号
    connect(confirmBtn, &QPushButton::clicked, this, [this, elevatorIndex]()
            {
        bool ok = false;
        int floor = m_floor_inputs[elevatorIndex]->text().toInt(&ok);
        if (ok && floor >= 1 && floor <= HEIGHT) {
            emit floorRequested(elevatorIndex, floor);
            m_floor_inputs[elevatorIndex]->clear();
        } });

    connect(lineEdit, &QLineEdit::returnPressed,
            confirmBtn, &QPushButton::click);

    auto taskList = new QListWidget(this);
    taskList->setStyleSheet(QString(
                                "QListWidget {"
                                "   background-color: #f0f0f5;"
                                "   border: 1px solid %1;"
                                "   border-radius: 4px;"
                                "   font-size: 12px;"
                                "   padding: 2px;"
                                "}"
                                "QListWidget::item {"
                                "   padding: 2px 4px;"
                                "   color: #2c3e50;"
                                "}")
                                .arg(color.name()));
    taskList->setMaximumHeight(120);
    taskList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    layout->addWidget(taskList);
    m_task_lists.append(taskList);

    groupBox->setLayout(layout);
    return groupBox;
}

void MainWindow::initInterface()
{
    // 初始化左侧楼层视图
    for (int i = 0; i < HEIGHT; ++i)
    {
        QLabel *label = new QLabel(this);
        label->setText(QString("第%1层").arg(HEIGHT - i));
        label->setAlignment(Qt::AlignCenter);
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_grid_layout->addWidget(label, i, 0);

        QGridLayout *layout = new QGridLayout();
        layout->setColumnStretch(0, 1);
        layout->setColumnStretch(1, 1);

        if (HEIGHT - i != HEIGHT)
        {
            QPushButton *up = new QPushButton(this);
            up->setIcon(QIcon(":/asset/up.png"));
            up->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            up->setCheckable(true);
            up->setStyleSheet(
                "QPushButton {"
                "   background-color: #ecf0f1;"
                "   border: 2px solid #bdc3c7;"
                "   border-radius: 4px;"
                "   padding: 2px;"
                "}"
                "QPushButton:checked {"
                "   background-color: #2ecc71;"
                "   border-color: #27ae60;"
                "}"
                "QPushButton:hover {"
                "   background-color: #d5dbdb;"
                "}"
                "QPushButton:checked:hover {"
                "   background-color: #27ae60;"
                "}");
            m_up_button[i] = up;
            layout->addWidget(up, 0, 0);
            // 连接信号：新请求
            connect(up, &QPushButton::toggled, this, [this, floorIdx = i, dir = 0](bool checked)
                    { onExternalRequest(floorIdx, dir, checked); });
        }

        if (HEIGHT - i != 1)
        {
            QPushButton *down = new QPushButton(this);
            down->setIcon(QIcon(":/asset/down.png"));
            down->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            down->setCheckable(true);
            down->setStyleSheet(
                "QPushButton {"
                "   background-color: #ecf0f1;"
                "   border: 2px solid #bdc3c7;"
                "   border-radius: 4px;"
                "   padding: 2px;"
                "}"
                "QPushButton:checked {"
                "   background-color: #e74c3c;"
                "   border-color: #c0392b;"
                "}"
                "QPushButton:hover {"
                "   background-color: #d5dbdb;"
                "}"
                "QPushButton:checked:hover {"
                "   background-color: #c0392b;"
                "}");
            m_down_button[i] = down;
            layout->addWidget(down, 0, 1);
            connect(down, &QPushButton::toggled, this, [this, floorIdx = i, dir = 1](bool checked)
                    { onExternalRequest(floorIdx, dir, checked); });
        }
        m_grid_layout->addLayout(layout, i, 1);
    }

    // 初始化可视化电梯
    for (int i = 0; i < ELEVATOR_NUM; ++i)
    {
        auto label = new QLabel(this);
        label->setText(QString("电梯%1").arg(i + 1));
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet(
            QString("background-color: %1; color: white; font-size: 14px; "
                    "font-weight: bold; border-radius: 4px;")
                .arg(kElevatorColors[i].name()));
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_elevator_labels.append(label);
    }
    updateElevatorDisplay();

    for (int col = 2; col <= 2 + ELEVATOR_NUM; ++col)
    {
        auto line = new QFrame(this);
        line->setFrameShape(QFrame::VLine);
        line->setFrameShadow(QFrame::Plain);
        line->setStyleSheet("QFrame { color: #bdc3c7; }");
        line->setLineWidth(2);
        m_grid_layout->addWidget(line, 0, col, HEIGHT, 1);
    }
}

void MainWindow::updateElevatorDisplay()
{
    for (int i = 0; i < ELEVATOR_NUM; ++i)
    {
        m_grid_layout->removeWidget(m_elevator_labels[i]);
    }
    for (int i = 0; i < ELEVATOR_NUM; ++i)
    {
        int row = HEIGHT - m_elevator[i];
        int col = 2 + i;
        m_grid_layout->addWidget(m_elevator_labels[i], row, col);
        m_elevator_labels[i]->show();

        if (i < m_lcd_displays.size())
            m_lcd_displays[i]->display(m_elevator[i]);

        if (i < m_direction_indicators.size())
        {
            switch (m_elevator_direction[i])
            {
            case Direction::up:
                m_direction_indicators[i]->setText("▲ 上行");
                m_direction_indicators[i]->setStyleSheet(
                    "font-size: 18px; font-weight: bold; padding: 4px;"
                    "color: #27ae60;");
                break;
            case Direction::down:
                m_direction_indicators[i]->setText("▼ 下行");
                m_direction_indicators[i]->setStyleSheet(
                    "font-size: 18px; font-weight: bold; padding: 4px;"
                    "color: #e74c3c;");
                break;
            default:
                m_direction_indicators[i]->setText("● 停止");
                m_direction_indicators[i]->setStyleSheet(
                    "font-size: 18px; font-weight: bold; padding: 4px;"
                    "color: #95a5a6;");
                break;
            }
        }

        if (i < m_task_lists.size())
        {
            m_task_lists[i]->clear();
            QStringList items;
            for (int t : m_elevator_targets[i])
                items << QString("前往 %1 楼").arg(t);
            m_task_lists[i]->addItems(items);
        }
    }
}

// 核心调度算法

// 调度入口函数
void MainWindow::onExternalRequest(int floorIdx, int direction, bool checked)
{
    int actualFloor = HEIGHT - floorIdx; // 实际楼层
    // 如果是新请求
    if (checked)
    {
        if (m_external_assignment[actualFloor][direction] != -1) // 已分配
            return;
        int best = findBestElevator(actualFloor, direction);
        if (best != -1) // 成功分配给某个电梯
        {
            m_external_assignment[actualFloor][direction] = best; // 记录分配
            m_elevator_targets[best].insert(actualFloor);         // 记录电梯目标
            if (m_elevator_direction[best] == Direction::stop)    // 若电梯静止，判断电梯运动方向
                if (actualFloor > m_elevator[best])
                    m_elevator_direction[best] = Direction::up;
                else if (actualFloor < m_elevator[best])
                    m_elevator_direction[best] = Direction::down;
        }
        else
        { // 无可用电梯
            qDebug() << "无可用电梯";
        }
    }
    else // 取消请求
    {
        int assigned = m_external_assignment[actualFloor][direction]; // 找到被分配的电梯序号
        if (assigned != -1)
        {                                                       // 的确被分配了
            m_elevator_targets[assigned].remove(actualFloor);   // 从待服务目标中去除
            m_external_assignment[actualFloor][direction] = -1; // 重新设置为未分配
        }
        else // 未分配
            qDebug() << "试图取消不存在的任务：电梯" << assigned << "前往" << actualFloor;
    }
}

// 最佳电梯选择
int MainWindow::findBestElevator(int floor, int reqDir)
{
    int best = -1;
    int minCost = INT_MAX;
    // 计算最低成本
    for (int i = 0; i < ELEVATOR_NUM; ++i)
    {
        int pos = m_elevator[i];
        Direction dir = m_elevator_direction[i];
        // 基础距离成本，乘10方便加权计算
        int bestDist = std::abs(pos - floor) * 10;
        int cost = bestDist;
        if (dir == Direction::stop)
            ; // 空闲电梯，无需计算其他成本
        // 同向顺路，也无需计算其他成本
        else if ((reqDir == Direction::up && dir == Direction::up && floor >= pos) ||
                 (reqDir == Direction::down && dir == Direction::down && floor <= pos))
            ;
        // 同向但是不顺路，增加一定cost
        else if ((reqDir == Direction::up && dir == Direction::up && floor < pos) ||
                 (reqDir == -1 && dir == -1 && floor > pos))
            cost += 100;
        // 方向相反
        else
            cost += 200;
        // 加入已有任务量的cost，避免饥饿
        cost += m_elevator_targets[i].size() * 30;
        // 更新最小cost
        if (cost < minCost)
        {
            minCost = cost;
            best = i;
        }
    }
    return best;
}

void MainWindow::updateElevators()
{
    for (int i = 0; i < ELEVATOR_NUM; ++i)
    {
        int current = m_elevator[i];             // 当前电梯位置
        Direction dir = m_elevator_direction[i]; // 当前电梯方向
        // 内呼：移除当前位置
        if (m_elevator_targets[i].contains(current))
        {
            m_elevator_targets[i].remove(current);
        }
        // 向上的外呼
        if (current != HEIGHT && m_external_assignment[current][0] == i)
        {
            m_external_assignment[current][0] = -1;
            // 复位对应按钮
            int floorIdx = HEIGHT - current;
            m_up_button[floorIdx]->blockSignals(true);
            m_up_button[floorIdx]->setChecked(false);
            m_up_button[floorIdx]->blockSignals(false);
        }
        // 向下的外呼
        if (current != 1 && m_external_assignment[current][1] == i)
        {
            m_external_assignment[current][1] = -1;
            // 复位对应按钮
            int floorIdx = HEIGHT - current;
            m_down_button[floorIdx]->blockSignals(true);
            m_down_button[floorIdx]->setChecked(false);
            m_down_button[floorIdx]->blockSignals(false);
        }
        // 方向决策
        if (m_elevator_targets[i].isEmpty())
            dir = Direction::stop;
        else
        {
            bool isInDir = false;
            for (int t : m_elevator_targets[i])
            {
                if ((dir == 1 && t > current) || (dir == -1 && t < current))
                {
                    isInDir = true;
                    break;
                }
            }
            if (!isInDir)
            {
                // 检查反方向有无请求
                bool hasAbove = false, hasBelow = false;
                for (int t : m_elevator_targets[i])
                {
                    if (t > current)
                        hasAbove = true;
                    if (t < current)
                        hasBelow = true;
                }
                // 反方向有请求：转向
                if (dir == Direction::up && hasBelow)
                    dir = Direction::down;
                else if (dir == Direction::down && hasAbove)
                    dir = Direction::up;
                // 从静止启动
                else if (dir == Direction::stop)
                {
                    int first = *m_elevator_targets[i].begin(); // set 自动排序
                    dir = (first > current) ? Direction::up : Direction::down;
                }
            }
        } // 方向决策结束
        m_elevator_direction[i] = dir;
        // 电梯移动一层
        if (dir != Direction::stop)
        {
            int next = current + (dir == Direction::up ? 1 : -1);
            if (next >= 1 && next <= HEIGHT)
                m_elevator[i] = next;
            else
                m_elevator_direction[i] = Direction::stop; // 边界
        }
    }
}

MainWindow::~MainWindow() {}