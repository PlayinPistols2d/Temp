// MyWidget.cpp
#include "MyWidget.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QListWidget>
#include <QRegularExpression>
#include <algorithm>
#include <limits>

MyWidget::MyWidget(QWidget *parent)
    : QWidget(parent)
{
    // Инициализация UI
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_listWidget = new QListWidget(this);
    m_sortButton = new QPushButton("Sort", this);

    layout->addWidget(m_listWidget);
    layout->addWidget(m_sortButton);

    // Пример заполнения списка MyObject
    m_list << MyObject("agagsga[0-1]")
           << MyObject("shshgsbsb[2-12]")
           << MyObject("example[1-1]")
           << MyObject("test[0-9]");

    populateListWidget();

    connect(m_sortButton, &QPushButton::clicked, this, &MyWidget::sortList);
}

void MyWidget::sortList()
{
    std::sort(m_list.begin(), m_list.end(), [](const MyObject &a, const MyObject &b){
        auto aPair = extractNumbers(a.value());
        auto bPair = extractNumbers(b.value());

        // Сортируем сначала по x, при равенстве по y
        if (aPair.first == bPair.first)
            return aPair.second < bPair.second;
        return aPair.first < bPair.first;
    });

    populateListWidget();
}

QPair<int,int> MyWidget::extractNumbers(const QString &str)
{
    static QRegularExpression re("\\[(\\d+)-(\\d+)\\]$");
    QRegularExpressionMatch match = re.match(str);
    if (match.hasMatch()) {
        bool ok1 = false, ok2 = false;
        int x = match.captured(1).toInt(&ok1);
        int y = match.captured(2).toInt(&ok2);
        if (ok1 && ok2) {
            return qMakePair(x, y);
        }
    }
    return qMakePair(std::numeric_limits<int>::max(), std::numeric_limits<int>::max());
}

void MyWidget::populateListWidget()
{
    m_listWidget->clear();
    for (const auto &obj : m_list) {
        m_listWidget->addItem(obj.value());
    }
}