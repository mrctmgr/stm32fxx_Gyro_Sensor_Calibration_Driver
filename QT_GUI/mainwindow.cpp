#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <QTextBrowser>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonParseError>
#include "QMessageBox"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->comboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onComboBoxIndexChanged);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateTextBrowser()
{
    QString dosyaAdi = "C:\\Users\\batma\\OneDrive\\Masaüstü\\exam\\deneme.txt";

    QFile dosya(dosyaAdi);
    if (dosya.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream okuyucu(&dosya);
        QString sonMetin = ui->textBrowser->toPlainText();

        while (!okuyucu.atEnd()) {
            QString satir = okuyucu.readLine();
            if (!sonMetin.contains(satir)) {
                ui->textBrowser->append(satir);

                // JSON verisini ayrıştır ve x, y, z değerlerini al
                QJsonParseError parseError;
                QJsonDocument jsonDoc = QJsonDocument::fromJson(satir.toUtf8(), &parseError);
                if (parseError.error == QJsonParseError::NoError && jsonDoc.isObject()) {
                    QJsonObject jsonObj = jsonDoc.object();
                    if (jsonObj.contains("Angle_X")) {
                        double angleX = jsonObj.value("Angle_X").toDouble();
                        ui->x_value->display(angleX);
                    }
                    if (jsonObj.contains("Angle_Y")) {
                        double angleY = jsonObj.value("Angle_Y").toDouble();
                        ui->y_value->display(angleY);
                    }
                    if (jsonObj.contains("Angle_Z")) {
                        double angleZ = jsonObj.value("Angle_Z").toDouble();
                        ui->z_value->display(angleZ);
                    }
                }
            }
        }

        dosya.close();
    } else {
        QMessageBox::warning(this, "Başlık", "Dosya bulunamadi.");
    }
}

void MainWindow::on_pushButton_clicked()
{
    ui->textBrowser->clear();
}

void MainWindow::on_pushButton_2_clicked()
{
    // Dosyanın değişikliklerini takip etmek için QTimer kullanalım
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTextBrowser);
    timer->start(1000);
}

void MainWindow::onComboBoxIndexChanged(int index)
{

}

void writeDataToFile(const QString& fileName, const QString& data)
{

    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {

        QTextStream out(&file);
        out << data;


        file.close();
    } else {
        qDebug() << "File Error: " << file.errorString();
    }
}

void MainWindow::on_pushButton_4_clicked()
{
    ui->textBrowser->append(ui->comboBox->currentText());
    if (ui->comboBox->currentText() == "Periodic Mode")
    {
        writeDataToFile("C:\\Users\\batma\\OneDrive\\Belgeler\\QT\\file_read_deneme_001\\foe.txt", "0");

    }
    if (ui->comboBox->currentText() == "Capture Mode")
    {
        writeDataToFile("C:\\Users\\batma\\OneDrive\\Belgeler\\QT\\file_read_deneme_001\\foe.txt", "1");
    }
    QMessageBox::warning(this, "Error", "Fİle is not exist.");
}

