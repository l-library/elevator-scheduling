#pragma once

#include <QMainWindow>

#define HEIGHT 20
#define ELEVATOR_NUM 5

class QGridLayout;
class QLabel;
class QPushButton;
class QLCDNumber;
class QLineEdit;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

signals:
    void upDownButtonChanged(int index, int direction);
    // 电梯内部请求信号：电梯编号(0-4)，目标楼层(1-20)
    void floorRequested(int elevatorIndex, int targetFloor);

private:
    // 初始化界面
    void initInterface();
    void updateElevatorDisplay();
    QWidget *createElevatorPanel(int elevatorIndex);

    // 电梯位置，起始位置为1
    int elevator[ELEVATOR_NUM];
    // 电梯方向：-1=下行，0=停止，1=上行
    int elevatorDirection[ELEVATOR_NUM];
    // 某一层的上下按钮是否被按下，0列表示上，1列表示下
    bool buttonChecked[HEIGHT][2];
    // 保存按钮指针
    QVector<QPushButton *> upButton;
    QVector<QPushButton *> downButton;
    QVector<QLabel *> elevatorLabels;

    QVector<QLCDNumber *> lcdDisplays;
    QVector<QLabel *> directionIndicators;
    QVector<QLineEdit *> floorInputs;

    // 布局
    QGridLayout *m_grid_layout;
};