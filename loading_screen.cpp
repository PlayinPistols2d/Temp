#include "LoadingScreen.h"
#include <QPainter>

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
    loadingLabel->setAlignment(Qt::AlignCenter);

    svgRenderer = new QSvgRenderer(this);

    blurEffect = new QGraphicsBlurEffect(this);
    blurEffect->setBlurRadius(10);
    backgroundLabel->setGraphicsEffect(blurEffect);

    layout = new QVBoxLayout(this);
    layout->addWidget(loadingLabel, 0, Qt::AlignCenter);

    setLayout(layout);
    setFixedSize(parentWidget()->size());

    // Timer to update the SVG animation (if needed)
    updateTimer = new QTimer(this);
    connect(updateTimer, &QTimer::timeout, this, &LoadingScreen::updateSvg);
}

void LoadingScreen::start() {
    updateTimer->start(16); // Approximately 60 FPS for smooth animation
    show();
}

void LoadingScreen::stop() {
    updateTimer->stop();
    hide();
    emit loadingFinished();
}

void LoadingScreen::setLoadingSvg(const QString &svgPath) {
    svgRenderer->load(svgPath);
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

void LoadingScreen::updateSvg() {
    QPixmap pixmap(loadingLabel->size());
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    svgRenderer->render(&painter);
    loadingLabel->setPixmap(pixmap);
}