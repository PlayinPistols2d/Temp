#ifndef EMPLOYEECARD_H
#define EMPLOYEECARD_H

#include <QWidget>
#include <QLabel>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>

namespace Ui {
class EmployeeCard;
}

class EmployeeCard : public QWidget
{
    Q_OBJECT

public:
    explicit EmployeeCard(QWidget *parent = nullptr);
    ~EmployeeCard();

    void setEmployeeID(const QString &id);
    void setEmployeePicture(const QPixmap &picture);
    void setEmployeeName(const QString &name);
    void setEmployeePosition(const QString &position);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    Ui::EmployeeCard *ui;
    QPoint dragStartPosition;
};

#endif // EMPLOYEECARD_H
