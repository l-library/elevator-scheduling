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
class QListWidget;

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
    // 定时更新
    void updateElevators();

    // 电梯位置，起始位置为1
    int m_elevator[ELEVATOR_NUM];
    // 电梯方向
    Direction m_elevator_direction[ELEVATOR_NUM];
    // 某一层的上下按钮是否被按下，0列表示上，1列表示下
    bool m_button_checked[HEIGHT + 1][2];
    // 每部电梯的待服务目标楼层（内呼 + 分配的外呼）
    QSet<int> m_elevator_targets[ELEVATOR_NUM];
    // 外呼记录：externalAssignment[楼层][方向] 表示负责该请求的电梯编号
    // 楼层使用实际楼层 1~20，-1表示未分配
    int m_external_assignment[HEIGHT + 1][2];
    // 保存按钮指针
    QVector<QPushButton *> m_up_button;
    QVector<QPushButton *> m_down_button;
    QVector<QLabel *> m_elevator_labels;

    QVector<QLCDNumber *> m_lcd_displays;
    QVector<QLabel *> m_direction_indicators;
    QVector<QLineEdit *> m_floor_inputs;
    QVector<QListWidget *> m_task_lists;

    // 布局
    QGridLayout *m_grid_layout;
};