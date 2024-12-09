#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QDebug>
#include "CustomLineEdit.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QWidget w;
    QVBoxLayout layout(&w);

    CustomLineEdit *lineEdit = new CustomLineEdit;
    layout.addWidget(lineEdit);

    QList<QPair<QString, QString>> items = {
        { "Apple", "A sweet red fruit." },
        { "Apricot", "A small, yellow-orange stone fruit." },
        { "Avocado", "A creamy green fruit used in guacamole." },
        { "Banana", "A long curved fruit with a yellow skin." },
        { "Blueberry", "A small sweet blue fruit." }
    };
    lineEdit->setItems(items);

    QObject::connect(lineEdit, &CustomLineEdit::itemSelected, [](const QString &text, const QString &hint){
        qDebug() << "Selected:" << text << "Hint:" << hint;
    });

    w.resize(300, 100);
    w.show();

    return app.exec();
}