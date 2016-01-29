#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    bool eventFilter(QObject *object, QEvent *event);

private:
    Ui::MainWindow *ui;


private slots:
    void showFps(int);

};

#endif // MAINWINDOW_H
