#ifndef LOADINGSCREEN_H
#define LOADINGSCREEN_H

#include <QWidget>
#include <QSvgRenderer>
#include <QLabel>
#include <QVBoxLayout>
#include <QGraphicsBlurEffect>
#include <QTimer>

class LoadingScreen : public QWidget {
    Q_OBJECT

public:
    explicit LoadingScreen(QWidget *parent = nullptr);
    void start();
    void stop();
    void setLoadingSvg(const QString &svgPath);
    void setBlurLevel(int level);
    void bindToProcess(QObject *process, const char *startSignal, const char *stopSignal);

private:
    QLabel *backgroundLabel;
    QLabel *loadingLabel;
    QSvgRenderer *svgRenderer;
    QTimer *updateTimer;
    QGraphicsBlurEffect *blurEffect;
    QVBoxLayout *layout;

    void setupUI();
    void updateSvg();

signals:
    void loadingFinished();

public slots:
    void onProcessFinished();
};

#endif // LOADINGSCREEN_H