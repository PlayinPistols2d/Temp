#ifndef SIDEPANEL_H
#define SIDEPANEL_H

#include <QWidget>
#include <QList>
#include "employee_card.h"

namespace Ui {
class SidePanel;
}

class SidePanel : public QWidget
{
    Q_OBJECT

public:
    explicit SidePanel(QWidget *parent = nullptr);
    ~SidePanel();

    void addEmployeeCard(EmployeeCard *card);
    void removeEmployeeCard(EmployeeCard *card);

private slots:
    void onFilterChanged(const QString &filter);

private:
    Ui::SidePanel *ui;
    QList<EmployeeCard*> m_employeeCards;

    void updateDisplayedCards();
};

#endif // SIDEPANEL_H
