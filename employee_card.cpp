#include "employee_card.h"
#include "ui_employee_card.h"

EmployeeCard::EmployeeCard(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EmployeeCard)
{
    ui->setupUi(this);
}

EmployeeCard::~EmployeeCard()
{
    delete ui;
}

void EmployeeCard::setEmployeeID(const QString &id)
{
    ui->label_id->setText(id);
}

void EmployeeCard::setEmployeePicture(const QPixmap &picture)
{
    ui->label_picture->setPixmap(picture);
}

void EmployeeCard::setEmployeeName(const QString &name)
{
    ui->label_name->setText(name);
}

void EmployeeCard::setEmployeePosition(const QString &position)
{
    ui->label_position->setText(position);
}

void EmployeeCard::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragStartPosition = event->pos();
    }
    QWidget::mousePressEvent(event);
}

void EmployeeCard::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;
    if ((event->pos() - dragStartPosition).manhattanLength() < QApplication::startDragDistance())
        return;

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    mimeData->setText("EmployeeCard");
    drag->setMimeData(mimeData);
    drag->exec(Qt::MoveAction);
}
