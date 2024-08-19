#include "LoadingScreen.h"

LoadingScreen::LoadingScreen(QWidget *parent) : QWidget(parent) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    setupUI();
}

void LoadingScreen::setupUI() {
    backgroundLabel = new QLabel(this);
    backgroundLabel->setStyleSheet("background-color: rgba(0, 0, 0, 150);"); // Semi-transparent black

    loadingLabel = new QLabel(this);
    loadingMovie = new QMovie(this);
    loadingLabel->setMovie(loadingMovie);

    blurEffect = new QGraphicsBlurEffect(this);
    blurEffect->setBlurRadius(10);
    backgroundLabel->setGraphicsEffect(blurEffect);

    layout = new QVBoxLayout(this);
    layout->addWidget(loadingLabel, 0, Qt::AlignCenter);

    setLayout(layout);
    setFixedSize(parentWidget()->size());
}

void LoadingScreen::start() {
    loadingMovie->start();
    show();
}

void LoadingScreen::stop() {
    loadingMovie->stop();
    hide();
    emit loadingFinished();
}

void LoadingScreen::setLoadingGif(const QString &gifPath) {
    loadingMovie->setFileName(gifPath);
}

void LoadingScreen::setBlurLevel(int level) {
    blurEffect->setBlurRadius(level);
}

void LoadingScreen::bindToProcess(QObject *process, const char *startSignal, const char *stopSignal) {
    connect(process, startSignal, this, SLOT(start()));
    connect(process, stopSignal, this, SLOT(stop()));
    connect(process, stopSignal, this, SLOT(onProcessFinished()));
}

void LoadingScreen::onProcessFinished() {
    stop();
    emit loadingFinished();
}