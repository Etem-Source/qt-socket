#include <QtWidgets>
#include <QtNetwork>
#include "client.h"

Client::Client(QWidget *parent)
    : QDialog(parent)
    , hostCombo(new QComboBox)
    , portLineEdit(new QLineEdit)
    , connectButton(new QPushButton(tr("Connexion")))
    , sendMessageButton(new QPushButton(tr("Envoyer le message")))
    , sendMessage(new QLineEdit)
    , messageDisplay(new QTextEdit)
    , tcpSocket(new QTcpSocket(this))
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    in.setDevice(tcpSocket);
    in.setVersion(QDataStream::Qt_5_0);

    // Interface utilisateur
    setupUi();

    connect(tcpSocket, &QIODevice::readyRead, this, &Client::readMessage);
    connect(tcpSocket, &QTcpSocket::connected, this, &Client::onConnected);
    connect(tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &Client::displayError);
    connect(connectButton, &QPushButton::clicked, this, &Client::requestConnection);
    connect(sendMessageButton, &QPushButton::clicked, this, &Client::sendMessageToServer);

    setWindowTitle(tr("Client Chat"));
    portLineEdit->setValidator(new QIntValidator(1, 65535, this));
    connectButton->setDefault(true);
    connectButton->setEnabled(false);

    QStringList hosts;
    hosts << "localhost" << "192.168.0.143";
    hostCombo->addItems(hosts);
    hostCombo->setEditable(true);

    connect(portLineEdit, &QLineEdit::textChanged, this, &Client::enableConnectButton);
    connect(hostCombo, &QComboBox::editTextChanged, this, &Client::enableConnectButton);
}

void Client::setupUi() {
    auto mainLayout = new QVBoxLayout(this);

    auto hostLabel = new QLabel(tr("&Nom du serveur:"));
    hostLabel->setBuddy(hostCombo);
    auto portLabel = new QLabel(tr("&Port:"));
    portLabel->setBuddy(portLineEdit);

    auto formLayout = new QFormLayout;
    formLayout->addRow(hostLabel, hostCombo);
    formLayout->addRow(portLabel, portLineEdit);

    mainLayout->addLayout(formLayout);
    mainLayout->addWidget(new QLabel(tr("Messages :")));
    mainLayout->addWidget(messageDisplay);
    mainLayout->addWidget(new QLabel(tr("Envoyer un message :")));
    mainLayout->addWidget(sendMessage);
    mainLayout->addWidget(sendMessageButton);
    mainLayout->addWidget(connectButton);

    messageDisplay->setReadOnly(true);
    setLayout(mainLayout);

    // Augmenter la taille de la fenêtre
    resize(600, 400);
}

void Client::requestConnection() {
    connectButton->setEnabled(false);
    tcpSocket->abort();
    tcpSocket->connectToHost(hostCombo->currentText(), portLineEdit->text().toInt());
}

void Client::displayError(QAbstractSocket::SocketError socketError) {
    switch (socketError) {
        case QAbstractSocket::RemoteHostClosedError:
            break;
        case QAbstractSocket::HostNotFoundError:
            QMessageBox::information(this, tr("Client Chat"),
                                     tr("Le serveur n'a pas été trouvé. Vérifiez le "
                                        "nom du serveur et les paramètres du port."));
            break;
        case QAbstractSocket::ConnectionRefusedError:
            QMessageBox::information(this, tr("Client Chat"),
                                     tr("La connexion a été refusée par le serveur. "
                                        "Assurez-vous que le serveur Fortune fonctionne, "
                                        "et vérifiez que le nom du serveur et les paramètres "
                                        "du port sont corrects."));
            break;
        default:
            QMessageBox::information(this, tr("Client Chat"),
                                     tr("L'erreur suivante s'est produite : %1.")
                                     .arg(tcpSocket->errorString()));
    }
    connectButton->setEnabled(true);
}

void Client::readMessage() {
    in.startTransaction();
    QString message;
    in >> message;
    if (!in.commitTransaction())
        return;

    // Afficher le message reçu du serveur
    displayMessage("Serveur", message);
}

void Client::sendMessageToServer() {
    QString message = sendMessage->text();
    if (message.isEmpty()) {
        return;
    }
    qDebug() << "Message envoyé (Client) : " << message;
    displayMessage("Vous", message);
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_5_0);
    out << message;
    tcpSocket->write(block);
    sendMessage->clear();
}

void Client::displayMessage(const QString &sender, const QString &message) {
    QString timeStamp = QDateTime::currentDateTime().toString("dd/MM/yyyy | hh:mm:ss");
    messageDisplay->append(QString("[%1] %2 : %3").arg(timeStamp, sender, message));
}

void Client::enableConnectButton() {
    connectButton->setEnabled(!hostCombo->currentText().isEmpty() &&
                              !portLineEdit->text().isEmpty());
}

void Client::sessionOpened() {
    enableConnectButton();
}

void Client::onConnected() {
    QString serverIp = tcpSocket->peerAddress().toString();
    displayMessage("<LOGS>", tr("Connexion réussie sur le serveur (%1)").arg(serverIp));
}
