#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QProcess>
#include <QString>
//#include <iostream>
//#include <CLI/CLI.hpp>
#include <cstdlib>
#include <unistd.h>
#include <string>
#include <fstream>
#include <filesystem>
#include "configparser.h"
#define VERSION "0.1.0"

//platform specific settings
//default platform
//empty if not set
#if defined(__MINGW32__) || defined(__MSYS__)
#include <windows.h>
#define OS_DEFUALT "nt"
#elif defined(__APPLE__) || defined(__MACH__)
#define OS_DEFUALT "darwin"
#elif defined(__linux__)
#include <sys/utsname.h>
#define OS_DEFUALT getLinuxDistro()
std::string getLinuxDistro() {
    std::ifstream releaseFile("/etc/os-release");
    std::string line;
    while (std::getline(releaseFile, line)) {
        if (line.find("ID=") == 0) {
            return line.substr(3);
        }
    }
    return "unknown";
}
#else
#define OS_DEFUALT ""
#endif

//architecture specific settings
//default architectures
//empty if not set
#if defined(__x86_64__) || defined(_M_X64)
#define DEFAULT_ARCH "x86_64"
#elif defined(__aarch64__)
#define DEFAULT_ARCH "arm64"
#elif defined(__arm__) || defined(_M_ARM)
#define DEFAULT_ARCH "arm";
#else
#define DEFAULT_ARCH "";
#endif


std::string os_type = OS_DEFUALT;

void restartApplicationWithArgs(const QString &fileLocation) {
    QString appPath = QCoreApplication::applicationFilePath();
    QStringList args;
    args << "--resume" << fileLocation;

    QProcess::startDetached(appPath, args);
}

MainWindow::MainWindow(const QString &configFilePath, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , configFilePathProvided(!configFilePath.isEmpty())
{
    ui->setupUi(this);
    ui->version->setText("Version: " VERSION);

    QString os_default = QString::fromStdString(os_type) ;
    ui->system->setText("System: " + os_default);
    ui->arch->setText("Arch: " DEFAULT_ARCH);
    if(configFilePathProvided){
        fileName = configFilePath;
        fileSelected = true;
        on_InstallButton_clicked();
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}



void MainWindow::on_selectButton_clicked()
{
    fileName = QFileDialog::getOpenFileName(this, "Open File", "", "YAML Files (*.yaml)");

    if (!fileName.isEmpty()) {
        // Do something with the selected file
        QMessageBox::information(this, "File Selected", "You selected: " + fileName);
        statusBar()->showMessage("file loaded");
        ui->fileNameLabel->setText(fileName);

        QFileInfo fileInfo(fileName);
        QString directory = fileInfo.absolutePath();
        ui->work_dir->setText("Word Dir: "+ directory);
        QProcess process;
        process.setWorkingDirectory(directory);

        fileSelected = true;



    } else {
        // Handle the case where no file is selected
        QMessageBox::warning(this, "No File Selected", "Please select a file.");
        ui->statusbar->showMessage("No File Selected");
    }
}


void MainWindow::on_InstallButton_clicked()
{
    if (fileSelected == true){
    statusBar()->showMessage(fileName);
    std::string std_filename = fileName.toStdString();
    ConfigParser parser(std_filename);
    parser.parseConfig();
    std::string platform = os_type;
    PlatformConfig platformConfig = parser.getPlatformConfig(platform, DEFAULT_ARCH);

    // Execute checks
    if (!configFilePathProvided){
        for (const auto& check : platformConfig.checks) {
            statusBar()->showMessage(QString("Running check: %1").arg(check.command.c_str()));
            int result = system(check.command.c_str());
            if (result != 0) {
                for (const auto& failCmd : check.onFailCommands) {
                    statusBar()->showMessage(QString("Running on_fail command: %1").arg(failCmd.c_str()));
                    system(failCmd.c_str());
                    refreshenv = true;
                }
                if (refreshenv){
                    restartApplicationWithArgs(fileName);
                    exit(0);
                }
            }
        }
    }


    // Place files
    for (const auto& file : platformConfig.filesToCopy) {
        statusBar()->showMessage(QString("Copying file from %1 to %2").arg(file.source.c_str(), file.destination.c_str()));
        std::filesystem::copy(file.source, file.destination, std::filesystem::copy_options::overwrite_existing);
    }

    // Install packages
    for (const auto& cmd : platformConfig.installCommands) {
        statusBar()->showMessage(QString("Running install command: %1").arg(cmd.c_str()));
        system(cmd.c_str());
    }

    // Clone git repositories
    for (const auto& repo : platformConfig.gitRepos) {
        statusBar()->showMessage(QString("Cloning git repository: %1").arg(repo.c_str()));
        std::string gitCmd = "git clone " + repo;
        system(gitCmd.c_str());
    }
     QMessageBox::information(this, "Instuall Finished!", "instructions of " + fileName+" installed.");
    }
    else{
        statusBar()->showMessage("no file seleted, please select a file.");
    }
}
