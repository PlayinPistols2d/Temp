// main.cpp or some usage point

#include "CustomSelector.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    CustomSelector selector;
    QList<QPair<QString, QString>> items = {
        { "Apple", "A sweet red fruit." },
        { "Apricot", "A small, yellow-orange stone fruit." },
        { "Avocado", "A creamy green fruit used in guacamole." },
        { "Banana", "A long curved fruit with a yellow skin." }
    };
    selector.setItems(items);

    QObject::connect(&selector, &CustomSelector::itemSelected, [](const QString &text, const QString &hint){
        qDebug() << "Selected:" << text << "Hint:" << hint;
    });

    selector.resize(200, 40);
    selector.show();

    return app.exec();
}