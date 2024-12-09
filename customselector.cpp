// CustomSelector.cpp
#include "CustomSelector.h"
#include <QStandardItem>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QStyle>

CustomSelector::CustomSelector(QWidget *parent)
    : QWidget(parent),
      m_lineEdit(new QLineEdit(this)),
      m_popupFrame(new QFrame(this, Qt::Popup)),
      m_listView(new QListView(m_popupFrame)),
      m_model(new QStandardItemModel(this)),
      m_proxyModel(new QSortFilterProxyModel(this))
{
    // Setup layout
    auto layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->addWidget(m_lineEdit);
    setLayout(layout);

    // Popup setup
    m_popupFrame->setFrameStyle(QFrame::Box | QFrame::Raised);
    m_popupFrame->setLineWidth(1);
    m_popupFrame->resize(width(), 150); // arbitrary initial size
    m_popupFrame->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);

    // Model and proxy model
    m_proxyModel->setSourceModel(m_model);
    m_proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_proxyModel->setFilterKeyColumn(0); // assuming first column holds the main text

    m_listView->setModel(m_proxyModel);
    m_listView->setEditTriggers(QListView::NoEditTriggers);

    // Connect signals
    connect(m_lineEdit, &QLineEdit::textChanged, this, &CustomSelector::onTextChanged);
    connect(m_listView, &QListView::clicked, this, &CustomSelector::onItemClicked);
    connect(m_listView->selectionModel(), &QItemSelectionModel::currentChanged,
            this, &CustomSelector::onCurrentChanged);

    // Event filter to catch hover events in listView
    m_listView->viewport()->installEventFilter(this);
}

void CustomSelector::addItem(const QString &text, const QString &hint)
{
    QStandardItem *item = new QStandardItem(text);
    item->setData(hint, Qt::UserRole);
    m_model->appendRow(item);
}

void CustomSelector::setItems(const QList<QPair<QString, QString>> &items)
{
    m_model->clear();
    for (auto &pair : items) {
        addItem(pair.first, pair.second);
    }
}

void CustomSelector::clearItems()
{
    m_model->clear();
}

void CustomSelector::setFilterCaseSensitivity(Qt::CaseSensitivity caseSensitivity)
{
    m_proxyModel->setFilterCaseSensitivity(caseSensitivity);
}

QString CustomSelector::currentText() const
{
    QModelIndex current = m_listView->currentIndex();
    if (!current.isValid())
        return QString();
    return current.data(Qt::DisplayRole).toString();
}

QString CustomSelector::currentHint() const
{
    QModelIndex current = m_listView->currentIndex();
    if (!current.isValid())
        return QString();
    return hintForIndex(current);
}

bool CustomSelector::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_listView->viewport()) {
        if (event->type() == QEvent::MouseMove) {
            // Show hint for hovered item
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            QModelIndex idx = m_listView->indexAt(me->pos());
            if (idx.isValid()) {
                updateHintDisplay(idx);
            }
        }
    }
    return QWidget::eventFilter(obj, event);
}

void CustomSelector::onTextChanged(const QString &text)
{
    // Filter the model
    m_proxyModel->setFilterFixedString(text);

    if (!text.isEmpty() && m_proxyModel->rowCount() > 0) {
        showPopup();
    } else {
        hidePopup();
    }
}

void CustomSelector::onItemClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    QString selectedText = index.data(Qt::DisplayRole).toString();
    QString selectedHint = hintForIndex(index);
    m_lineEdit->setText(selectedText);
    hidePopup();

    emit itemSelected(selectedText, selectedHint);
}

void CustomSelector::onCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous);
    if (current.isValid()) {
        updateHintDisplay(current);
    }
}

QString CustomSelector::hintForIndex(const QModelIndex &index) const
{
    QModelIndex sourceIndex = m_proxyModel->mapToSource(index);
    return sourceIndex.data(Qt::UserRole).toString();
}

void CustomSelector::showPopup()
{
    if (m_popupFrame->isVisible()) return;

    // Position the popup below the line edit
    QPoint below = mapToGlobal(QPoint(0, m_lineEdit->height()));
    m_popupFrame->move(below);
    m_popupFrame->resize(width(), 150);
    m_popupFrame->show();
}

void CustomSelector::hidePopup()
{
    if (m_popupFrame->isVisible()) {
        m_popupFrame->hide();
    }
}

void CustomSelector::updateHintDisplay(const QModelIndex &index)
{
    if (!index.isValid()) return;

    QString hintText = hintForIndex(index);
    if (!hintText.isEmpty()) {
        // Using a tooltip here for simplicity, but you can create a custom label instead.
        // Alternatively, you could show a separate hint widget that updates its text dynamically.
        QToolTip::showText(QCursor::pos(), hintText, m_listView);
    } else {
        QToolTip::hideText();
    }
}