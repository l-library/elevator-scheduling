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

namespace {

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

    for (int i = 0; i < ELEVATOR_NUM; ++i) {
        elevator[i] = 1;
        elevatorDirection[i] = 0;
    }

    for (int i = 0; i < HEIGHT; ++i) {
        buttonChecked[i][0] = false;
        buttonChecked[i][1] = false;
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
    for (int i = 0; i < ELEVATOR_NUM; ++i) {
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
            this, [this](int index, int direction) {
                if (direction == 0)
                    upButton[index]->setChecked(buttonChecked[index][direction]);
                else
                    downButton[index]->setChecked(buttonChecked[index][direction]);
            });

    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, [this]() {
        elevator[1] = 3;
        elevatorDirection[1] = 1;
        updateElevatorDisplay();
    });
    timer->start(1000);
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
        "}").arg(color.name()));

    auto layout = new QVBoxLayout(groupBox);
    layout->setSpacing(10);
    layout->setAlignment(Qt::AlignTop);

    auto lcd = new QLCDNumber(this);
    lcd->setDigitCount(2);
    lcd->setSegmentStyle(QLCDNumber::Flat);
    lcd->display(elevator[elevatorIndex]);
    lcd->setMinimumHeight(70);
    lcd->setStyleSheet(
        "QLCDNumber {"
        "   background-color: #0d1b2a;"
        "   color: #00ff88;"
        "   border: 2px solid #1b2838;"
        "   border-radius: 6px;"
        "}");
    layout->addWidget(lcd);
    lcdDisplays.append(lcd);

    auto dirLabel = new QLabel("● 停止", this);
    dirLabel->setAlignment(Qt::AlignCenter);
    dirLabel->setStyleSheet(
        "font-size: 18px; font-weight: bold; padding: 4px;"
        "color: #95a5a6;");
    layout->addWidget(dirLabel);
    directionIndicators.append(dirLabel);

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
                "}").arg(color.name()));
    inputLayout->addWidget(lineEdit);
    floorInputs.append(lineEdit);

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

    // 预留接口：确认按钮和回车触发楼层请求信号
    connect(confirmBtn, &QPushButton::clicked, this, [this, elevatorIndex]() {
        bool ok = false;
        int floor = floorInputs[elevatorIndex]->text().toInt(&ok);
        if (ok && floor >= 1 && floor <= HEIGHT) {
            emit floorRequested(elevatorIndex, floor);
            floorInputs[elevatorIndex]->clear();
        }
    });

    connect(lineEdit, &QLineEdit::returnPressed,
            confirmBtn, &QPushButton::click);

    groupBox->setLayout(layout);
    return groupBox;
}

void MainWindow::initInterface()
{
    // 初始化左侧楼层视图
    for (int i = 0; i < HEIGHT; ++i) {
        QLabel *label = new QLabel(this);
        label->setText(QString("第%1层").arg(HEIGHT - i));
        label->setAlignment(Qt::AlignCenter);
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_grid_layout->addWidget(label, i, 0);

        QGridLayout *layout = new QGridLayout();
        layout->setColumnStretch(0, 1);
        layout->setColumnStretch(1, 1);

        if (HEIGHT - i != HEIGHT) {
            QPushButton *up = new QPushButton(this);
            up->setIcon(QIcon(":/asset/up.png"));
            up->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            up->setCheckable(true);
            upButton.append(up);
            layout->addWidget(up, 0, 0);
            connect(up, &QPushButton::toggled, [i, this](bool /*checked*/) {
                buttonChecked[i][0] = !buttonChecked[i][0];
                emit upDownButtonChanged(i, 0);
            });
        }

        if (HEIGHT - i != 1) {
            QPushButton *down = new QPushButton(this);
            down->setIcon(QIcon(":/asset/down.png"));
            down->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            down->setCheckable(true);
            downButton.append(down);
            layout->addWidget(down, 0, 1);
            connect(down, &QPushButton::toggled, [i, this](bool /*checked*/) {
                buttonChecked[i][1] = !buttonChecked[i][1];
                emit upDownButtonChanged(i, 1);
            });
        }
        m_grid_layout->addLayout(layout, i, 1);
    }

    // 初始化可视化电梯
    for (int i = 0; i < ELEVATOR_NUM; ++i) {
        auto label = new QLabel(this);
        label->setText(QString("电梯%1").arg(i + 1));
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet(
            QString("background-color: %1; color: white; font-size: 14px; "
                    "font-weight: bold; border-radius: 4px;")
                .arg(kElevatorColors[i].name()));
        label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        elevatorLabels.append(label);
    }
    updateElevatorDisplay();
}

void MainWindow::updateElevatorDisplay()
{
    for (int i = 0; i < ELEVATOR_NUM; ++i) {
        m_grid_layout->removeWidget(elevatorLabels[i]);
    }
    for (int i = 0; i < ELEVATOR_NUM; ++i) {
        int row = HEIGHT - elevator[i];
        int col = 2 + i;
        m_grid_layout->addWidget(elevatorLabels[i], row, col);
        elevatorLabels[i]->show();

        if (i < lcdDisplays.size())
            lcdDisplays[i]->display(elevator[i]);

        if (i < directionIndicators.size()) {
            switch (elevatorDirection[i]) {
            case 1:
                directionIndicators[i]->setText("▲ 上行");
                directionIndicators[i]->setStyleSheet(
                    "font-size: 18px; font-weight: bold; padding: 4px;"
                    "color: #27ae60;");
                break;
            case -1:
                directionIndicators[i]->setText("▼ 下行");
                directionIndicators[i]->setStyleSheet(
                    "font-size: 18px; font-weight: bold; padding: 4px;"
                    "color: #e74c3c;");
                break;
            default:
                directionIndicators[i]->setText("● 停止");
                directionIndicators[i]->setStyleSheet(
                    "font-size: 18px; font-weight: bold; padding: 4px;"
                    "color: #95a5a6;");
                break;
            }
        }
    }
}

MainWindow::~MainWindow() {}