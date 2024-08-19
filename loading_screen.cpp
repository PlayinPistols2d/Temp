#include "LoadingScreen.h"

LoadingScreen::LoadingScreen(QWidget *parent) : QWidget(parent)
{
    // Make sure the loading screen covers the entire parent window
    setGeometry(parent->rect());

    // Set the blur effect
    backgroundLabel = new QLabel(this);
    backgroundLabel->setGeometry(this->rect());
    backgroundLabel->setStyleSheet("background-color: rgba(0, 0, 0, 100);"); // Semi-transparent background

    QGraphicsBlurEffect *blurEffect = new QGraphicsBlurEffect(this);
    blurEffect->setBlurRadius(10);
    backgroundLabel->setGraphicsEffect(blurEffect);

    // Set up the loading SVG
    loadingSvg = new QSvgWidget(":/path/to/your/loading.svg", this);
    loadingSvg->setFixedSize(100, 100); // Set desired size
    loadingSvg->move((width() - loadingSvg->width()) / 2, (height() - loadingSvg->height()) / 2);

    // Set up the layout (optional)
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(backgroundLabel);
    layout->addWidget(loadingSvg, 0, Qt::AlignCenter);

    // Initially hidden
    setVisible(false);
}

void LoadingScreen::start()
{
    setVisible(true);
    loadingSvg->show();
}

void LoadingScreen::stop()
{
    loadingSvg->hide();
    setVisible(false);
    emit loadingFinished();
}