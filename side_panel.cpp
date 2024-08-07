
#include "side_panel.h"
#include "ui_side_panel.h"

SidePanel::SidePanel(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SidePanel)
{
    ui->setupUi(this);

    ui->comboBox_filter->addItem("All");
    ui->comboBox_filter->addItem("Developer");
    ui->comboBox_filter->addItem("Designer");
    connect(ui->comboBox_filter, &QComboBox::currentTextChanged, this, &SidePanel::onFilterChanged);
}

SidePanel::~SidePanel()
{
    delete ui;
}

void SidePanel::addEmployeeCard(EmployeeCard *card)
{
    m_employeeCards.append(card);
    ui->scrollAreaLayout->addWidget(card);
    card->show();
    connect(card, &EmployeeCard::removeClicked, this, [=](EmployeeCard *card) {
        removeEmployeeCard(card);
        addEmployeeCard(card);
    });
}

void SidePanel::removeEmployeeCard(EmployeeCard *card)
{
    m_employeeCards.removeOne(card);
    ui->scrollAreaLayout->removeWidget(card);
    card->hide();
}

void SidePanel::onFilterChanged(const QString &filter)
{
    updateDisplayedCards();
}

void SidePanel::updateDisplayedCards()
{
    QString filter = ui->comboBox_filter->currentText();
    for (EmployeeCard *card : qAsConst(m_employeeCards)) {
        if (filter == "All" || card->jobPosition() == filter) {
            card->show();
        } else {
            card->hide();
        }
    }
}
