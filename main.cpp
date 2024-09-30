#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QCommandLineParser>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "ros-copilot_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    QCommandLineParser parser;
    parser.setApplicationDescription("Qt GUI Application");
    parser.addHelpOption();

    QCommandLineOption resumeOption(QStringList() << "r" << "resume",
                                    "Resume using the specified config file.", "file");
    parser.addOption(resumeOption);

    parser.process(a);
    QString resumeFile = parser.value(resumeOption);

    MainWindow w(resumeFile);
    w.show();
    return a.exec();
}
