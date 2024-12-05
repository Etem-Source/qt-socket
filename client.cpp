#include <QtWidgets>
#include <QtNetwork>
#include "client.h"

Client::Client(QWidget *parent)
    : QDialog(parent)
    , hostCombo(new QComboBox)
    , portLineEdit(new QLineEdit)
    , getFortuneButton(new QPushButton(tr("Get Fortune")))
    , sendMessageButton(new QPushButton(tr("Send Message")))
    , sendMessage(new QLineEdit)
    , receivedMessage(new QLabel)
    , tcpSocket(new QTcpSocket(this))
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    in.setDevice(tcpSocket);
    in.setVersion(QDataStream::Qt_5_0);

    // Configuration de l'interface utilisateur
    setupUi();

    connect(tcpSocket, &QIODevice::readyRead, this, &Client::readFortune);
    connect(tcpSocket, QOverload<QAbstractSocket::SocketError>::of
            (&QAbstractSocket::error), this, &Client::displayError);
    connect(getFortuneButton, &QPushButton::clicked, this, &Client::requestNewFortune);
    connect(sendMessageButton, &QPushButton::clicked, this, &Client::sendMessageToServer);

    setWindowTitle(tr("Fortune Client"));
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));
    getFortuneButton->setDefault(true);
    getFortuneButton->setEnabled(false);

    QStringList hosts;
    hosts << "localhost";
    hostCombo->addItems(hosts);
    QStringList hosts2;
    hosts << "172.16.13.11";
    hostCombo->addItems(hosts);


    connect(portLineEdit, &QLineEdit::textChanged, this, &Client::enableGetFortuneButton);
}

void Client::setupUi() {
    auto mainLayout = new QVBoxLayout(this);

    auto hostLabel = new QLabel(tr("&Server name:"));
    hostLabel->setBuddy(hostCombo);
    auto portLabel = new QLabel(tr("&Port:"));
    portLabel->setBuddy(portLineEdit);

    auto formLayout = new QFormLayout;
    formLayout->addRow(hostLabel, hostCombo);
    formLayout->addRow(portLabel, portLineEdit);

    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(new QLabel(tr("Received Messages:")));
    mainLayout->addWidget(receivedMessage);
    mainLayout->addWidget(new QLabel(tr("Send Message:")));
    mainLayout->addWidget(sendMessage);
    mainLayout->addWidget(sendMessageButton);
    mainLayout->addWidget(getFortuneButton);

    setLayout(mainLayout);
}

void Client::requestNewFortune() {
    getFortuneButton->setEnabled(false);
    tcpSocket->abort();
    tcpSocket->connectToHost(hostCombo->currentText(), portLineEdit->text().toInt());
}

void Client::displayError(QAbstractSocket::SocketError socketError) {
    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
            break;
        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this, tr("Fortune Client"),
                                     tr("The host was not found. Please check the "
                                        "host name and port settings."));
            break;
        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this, tr("Fortune Client"),
                                     tr("The connection was refused by the peer. "
                                        "Make sure the fortune server is running, "
                                        "and check that the host name and port "
                                        "settings are correct."));
            break;
        default:
            QMessageBox::information(this, tr("Fortune Client"),
                                     tr("The following error occurred: %1.")
                                     .arg(tcpSocket->errorString()));
    }
    getFortuneButton->setEnabled(true);
}

void Client::readFortune() {
    in.startTransaction();
    QString nextFortune;
    in >> nextFortune;
    if (!in.commitTransaction())
        return;
    if (nextFortune == currentFortune) {
        QTimer::singleShot(0, this, &Client::requestNewFortune);
        return;
    }
    currentFortune = nextFortune;
    receivedMessage->setText(currentFortune);
    getFortuneButton->setEnabled(true);
}

void Client::enableGetFortuneButton() {
    getFortuneButton->setEnabled(!hostCombo->currentText().isEmpty() &&
                                 !portLineEdit->text().isEmpty());
}

void Client::sessionOpened() {
    enableGetFortuneButton();
}

void Client::sendMessageToServer() {
    QString message = sendMessage->text();
    if (message.isEmpty()) {
        return;
    }
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_0);
    out << message;
    tcpSocket->write(block);
    sendMessage->clear();
}
