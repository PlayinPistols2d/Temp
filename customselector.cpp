#include "CustomLineEdit.h"
#include <QFocusEvent>
#include <QMouseEvent>
#include <QApplication>
#include <QAbstractItemView>

CustomLineEdit::CustomLineEdit(QWidget *parent)
    : QLineEdit(parent),
      m_completer(new QCompleter(this)),
      m_model(new QStandardItemModel(this))
{
    // Setup model and completer
    m_completer->setModel(m_model);
    m_completer->setCompletionMode(QCompleter::PopupCompletion);
    m_completer->setCaseSensitivity(Qt::CaseInsensitive);
    m_completer->setFilterMode(Qt::MatchContains); 
    // MatchContains or MatchStartsWith depending on preferred filtering

    // Set the completer on the line edit
    setCompleter(m_completer);

    // Connect signals
    connect(m_completer, QOverload<const QString &>::of(&QCompleter::activated),
            this, &CustomLineEdit::onCompletionSelected);
    connect(m_completer, QOverload<const QString &>::of(&QCompleter::highlighted),
            this, &CustomLineEdit::onHighlighted);

    // Event filter to detect hover on completer popup
    if (m_completer->popup()) {
        m_completer->popup()->viewport()->installEventFilter(this);
    }
}

void CustomLineEdit::addItem(const QString &text, const QString &hint)
{
    QStandardItem *item = new QStandardItem(text);
    item->setData(hint, Qt::UserRole);
    m_model->appendRow(item);
}

void CustomLineEdit::setItems(const QList<QPair<QString, QString>> &items)
{
    m_model->clear();
    for (auto &pair : items) {
        addItem(pair.first, pair.second);
    }
}

void CustomLineEdit::clearItems()
{
    m_model->clear();
}

void CustomLineEdit::setFilterCaseSensitivity(Qt::CaseSensitivity cs)
{
    m_completer->setCaseSensitivity(cs);
}

void CustomLineEdit::focusInEvent(QFocusEvent *e)
{
    QLineEdit::focusInEvent(e);
    // When we focus in, show the completer if there are items
    if (m_model->rowCount() > 0) {
        showCompleterPopup();
    }
}

void CustomLineEdit::mousePressEvent(QMouseEvent *e)
{
    QLineEdit::mousePressEvent(e);
    // On click, also show the completer popup
    if (m_model->rowCount() > 0) {
        showCompleterPopup();
    }
}

bool CustomLineEdit::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_completer->popup()->viewport()) {
        if (event->type() == QEvent::MouseMove) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            QModelIndex index = m_completer->popup()->indexAt(me->pos());
            if (index.isValid()) {
                showHintForIndex(index);
            } else {
                QToolTip::hideText();
            }
        }
    }
    return QLineEdit::eventFilter(obj, event);
}

void CustomLineEdit::onCompletionSelected(const QString &completion)
{
    // Find the item in the model that matches the completion
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex idx = m_model->index(i, 0);
        if (idx.data(Qt::DisplayRole).toString() == completion) {
            QString hint = hintForIndex(idx);
            emit itemSelected(completion, hint);
            break;
        }
    }
}

void CustomLineEdit::onHighlighted(const QString &highlightedText)
{
    // On keyboard navigation, highlighted item changes
    // Show hint for that item if found
    for (int i = 0; i < m_model->rowCount(); ++i) {
        QModelIndex idx = m_model->index(i, 0);
        if (idx.data(Qt::DisplayRole).toString() == highlightedText) {
            showHintForIndex(idx);
            break;
        }
    }
}

void CustomLineEdit::showCompleterPopup()
{
    // Force the completer to show the popup under current text
    m_completer->complete();
}

QString CustomLineEdit::hintForIndex(const QModelIndex &index) const
{
    return index.data(Qt::UserRole).toString();
}

void CustomLineEdit::showHintForIndex(const QModelIndex &index)
{
    QString hint = hintForIndex(index);
    if (!hint.isEmpty()) {
        // Show tooltip at cursor position or near popup
        QPoint globalPos = m_completer->popup()->viewport()->mapToGlobal(
            m_completer->popup()->viewport()->rect().topLeft()
        );
        // Adjust position based on item rect
        QRect rect = m_completer->popup()->visualRect(index);
        QPoint hintPos = globalPos + QPoint(rect.right() + 10, rect.top());
        QToolTip::showText(hintPos, hint, m_completer->popup());
    } else {
        QToolTip::hideText();
    }
}