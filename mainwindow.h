#pragma once

#include <QMainWindow>

#define HEIGHT 20
#define ELEVATOR_NUM 5

class QGridLayout;
class QPushButton;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

signals:
    void upDownButtonChanged(int index, int direction);

private:
    // 初始化界面
    void initInterface();

    // 电梯位置，起始位置为1
    int elevator[ELEVATOR_NUM];
    // 某一层的上下按钮是否被按下，0列表示上，1列表示下
    bool buttonChecked[HEIGHT][2];
    // 保存按钮指针
    QVector<QPushButton *> upButton;
    QVector<QPushButton *> downButton;

    // 布局
    QGridLayout *m_grid_layout;
};
