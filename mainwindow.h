#pragma once

#include <QMainWindow>

class QGridLayout;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private:
    // 初始化界面
    void initInterface();

    // 布局
    QGridLayout *m_grid_layout;
};
