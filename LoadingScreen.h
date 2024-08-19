#ifndef LOADINGSCREEN_H
#define LOADINGSCREEN_H

#include <QWidget>
#include <QMovie>
#include <QLabel>
#include <QVBoxLayout>
#include <QGraphicsBlurEffect>
#include <QPropertyAnimation>

class LoadingScreen : public QWidget {
    Q_OBJECT

public:
    explicit LoadingScreen(QWidget *parent = nullptr);
    void start();
    void stop();
    void setLoadingGif(const QString &gifPath);
    void setBlurLevel(int level);
    void bindToProcess(QObject *process, const char *startSignal, const char *stopSignal);

private:
    QLabel *backgroundLabel;
    QLabel *loadingLabel;
    QMovie *loadingMovie;
    QGraphicsBlurEffect *blurEffect;
    QVBoxLayout *layout;

    void setupUI();

signals:
    void loadingFinished();

public slots:
    void onProcessFinished();
};

#endif // LOADINGSCREEN_H