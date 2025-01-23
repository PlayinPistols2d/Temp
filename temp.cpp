#ifndef CUSTOMVERTICALHEADER_H
#define CUSTOMVERTICALHEADER_H

#include <QHeaderView>
#include <QPainter>

class CustomVerticalHeader : public QHeaderView
{
    Q_OBJECT
public:
    explicit CustomVerticalHeader(Qt::Orientation orientation, QWidget *parent = nullptr)
        : QHeaderView(orientation, parent) {}

protected:
    void paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const override
    {
        if (orientation() == Qt::Vertical) {
            // Рисуем фоновую часть
            painter->save();
            painter->fillRect(rect, this->palette().base());
            painter->restore();

            // Устанавливаем шрифт и цвет
            painter->setFont(this->font());
            painter->setPen(this->palette().text().color());

            // Нумерация строк с 0
            QString text = QString::number(logicalIndex);

            // Центрируем текст
            painter->drawText(rect, Qt::AlignCenter, text);
        } else {
            QHeaderView::paintSection(painter, rect, logicalIndex);
        }
    }
};

#endif // CUSTOMVERTICALHEADER_H



#include "CustomVerticalHeader.h"
#include <QApplication>
#include <QTableView>
#include <QStandardItemModel>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QTableView tableView;
    QStandardItemModel model(10, 5); // Таблица 10x5

    // Заполняем таблицу данными
    for (int row = 0; row < model.rowCount(); ++row) {
        for (int col = 0; col < model.columnCount(); ++col) {
            model.setItem(row, col, new QStandardItem(QString::number(row * col)));
        }
    }

    // Устанавливаем модель
    tableView.setModel(&model);

    // Устанавливаем кастомный вертикальный заголовок
    auto *header = new CustomVerticalHeader(Qt::Vertical, &tableView);
    tableView.setVerticalHeader(header);

    // Отображаем таблицу
    tableView.show();

    return app.exec();
}