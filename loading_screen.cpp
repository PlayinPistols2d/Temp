#include "LoadingScreen.h"

LoadingScreen::LoadingScreen(QWidget *parent) : QWidget(parent)
{
    // Set the widget to be transparent and frameless
    setAttribute(Qt::WA_TransparentForMouseEvents); // Allows clicks to pass through the loading screen
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent, false);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::SubWindow);

    // Make sure the loading screen covers the entire parent widget
    setGeometry(parent->rect());

    // Set the blur effect
    backgroundLabel = new QLabel(this);
    backgroundLabel->setStyleSheet("background-color: rgba(0, 0, 0, 100);"); // Semi-transparent background

    blurEffect = new QGraphicsBlurEffect(this);
    blurEffect->setBlurRadius(10);
    backgroundLabel->setGraphicsEffect(blurEffect);

    // Set up the loading SVG
    loadingSvg = new QSvgWidget(":/path/to/your/loading.svg", this);
    loadingSvg->setFixedSize(100, 100); // Set desired size

    // Set up the layout (optional)
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(backgroundLabel);
    layout->setContentsMargins(0, 0, 0, 0); // No margins, fill the entire widget
    layout->setSpacing(0);

    // Initially hidden
    setVisible(false);
}

void LoadingScreen::start()
{
    // Move the loading spinner to the center of the parent
    loadingSvg->move((width() - loadingSvg->width()) / 2, (height() - loadingSvg->height()) / 2);
    setVisible(true);
}

void LoadingScreen::stop()
{
    setVisible(false);
    emit loadingFinished();
}

void LoadingScreen::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    // Resize and reposition the background and loading indicator to match the parent widget
    setGeometry(parentWidget()->rect());
    backgroundLabel->setGeometry(this->rect());
    loadingSvg->move((width() - loadingSvg->width()) / 2, (height() - loadingSvg->height()) / 2);
}