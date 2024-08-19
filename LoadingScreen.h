#ifndef LOADINGSCREEN_H
#define LOADINGSCREEN_H

#include <QWidget>
#include <QGraphicsBlurEffect>
#include <QLabel>
#include <QVBoxLayout>
#include <QSvgWidget>

class LoadingScreen : public QWidget
{
    Q_OBJECT
public:
    explicit LoadingScreen(QWidget *parent = nullptr);

    void start();
    void stop();

signals:
    void loadingFinished();

private:
    QLabel *backgroundLabel;
    QSvgWidget *loadingSvg;
};

#endif // LOADINGSCREEN_H