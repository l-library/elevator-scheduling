#pragma once

#include <QMainWindow>
#include <QSet>
#include <climits>

#define HEIGHT 20
#define ELEVATOR_NUM 5

class QGridLayout;
class QLabel;
class QPushButton;
class QLCDNumber;
class QLineEdit;

enum Direction
{
    up,
    down,
    stop
};

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
    // 调度算法
    void onExternalRequest(int floorIdx, int direction, bool checked); // 有新的外部请求
    int findBestElevator(int floor, int reqDir);                       // 最佳电梯选择

    // 电梯位置，起始位置为1
    int elevator[ELEVATOR_NUM];
    // 电梯方向
    Direction elevatorDirection[ELEVATOR_NUM];
    // 某一层的上下按钮是否被按下，0列表示上，1列表示下
    bool buttonChecked[HEIGHT + 1][2];
    // 每部电梯的待服务目标楼层（内呼 + 分配的外呼）
    QSet<int> elevatorTargets[ELEVATOR_NUM];
    // 外呼记录：externalAssignment[楼层][方向] 表示负责该请求的电梯编号
    // 楼层使用实际楼层 1~20，-1表示未分配
    int externalAssignment[HEIGHT + 1][2];
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