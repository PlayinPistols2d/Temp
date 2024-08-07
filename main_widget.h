#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include "task_panel.h"
#include "side_panel.h"

namespace Ui {
class MainWidget;
}

class MainWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MainWidget(QWidget *parent = nullptr);
    ~MainWidget();

private:
    Ui::MainWidget *ui;
    void setupTestData();
};

#endif // MAINWIDGET_H
