#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>

void resizeColumnsToMaxContent(QTableView *tableView) {
    QAbstractItemModel *model = tableView->model();
    if (!model) return;

    QFontMetrics metrics(tableView->font());

    for (int col = 0; col < model->columnCount(); ++col) {
        int maxWidth = 0;

        // Проверяем ширину заголовка
        QString headerText = model->headerData(col, Qt::Horizontal).toString();
        maxWidth = metrics.horizontalAdvance(headerText) + 10;

        // Проверяем ширину данных в столбце
        for (int row = 0; row < model->rowCount(); ++row) {
            QString text = model->data(model->index(row, col)).toString();
            maxWidth = qMax(maxWidth, metrics.horizontalAdvance(text) + 10);
        }

        tableView->setColumnWidth(col, maxWidth);
    }
}