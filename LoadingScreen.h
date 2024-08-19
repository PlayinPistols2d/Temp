#ifndef LOADINGSCREEN_H
#define LOADINGSCREEN_H

#include <QWidget>
#include <QGraphicsBlurEffect>
#include <QLabel>
#include <QSvgWidget>
#include <QVBoxLayout>

class LoadingScreen : public QWidget
{
    Q_OBJECT
public:
    explicit LoadingScreen(QWidget *parent = nullptr);

    void start();
    void stop();

protected:
    void resizeEvent(QResizeEvent *event) override;

signals:
    void loadingFinished();

private:
    QLabel *backgroundLabel;
    QSvgWidget *loadingSvg;
    QGraphicsBlurEffect *blurEffect;
};

#endif // LOADINGSCREEN_H