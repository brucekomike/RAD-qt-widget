#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(const QString &configFilePath, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_InstallButton_clicked();

    void on_selectButton_clicked();

private:
    Ui::MainWindow *ui;
    QString fileName;
    bool configFilePathProvided = false;
    bool fileSelected = false;
    bool refreshenv = false;

};
#endif // MAINWINDOW_H
