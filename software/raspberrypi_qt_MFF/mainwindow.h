#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>

class QLabel;
class LeptonThread;
class QGridLayout;

class MainWindow : public QMainWindow {
    Q_OBJECT

	static const int SCALE = 6;

    static int snapshotCount;
    static int imageWidth;
    static int imageHeight;

    QLabel *imageLabel;
    LeptonThread *thread;
    QGridLayout *layout;

    unsigned short rawMin, rawMax;
    QVector<unsigned short> rawData;
    QImage rgbImage;

public:
    explicit MainWindow(QWidget *parent = 0);

public slots:
    void saveSnapshot();
    void ffcManual();
    void updateImage(unsigned short *, int, int);
};

#endif // MAINWINDOW_H
