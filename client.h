#ifndef CLIENT_H
#define CLIENT_H

#include <QDataStream>
#include <QDialog>
#include <QTcpSocket>
#include <QTextEdit>

QT_BEGIN_NAMESPACE
class QComboBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QNetworkSession;
QT_END_NAMESPACE

class Client : public QDialog
{
    Q_OBJECT

public:
    explicit Client(QWidget *parent = nullptr);

private slots:
    void displayError(QAbstractSocket::SocketError socketError);
    void enableConnectButton();
    void sessionOpened();
    void setupUi();
    void sendMessageToServer();
    void displayMessage(const QString &sender, const QString &message);
    void requestConnection();
    void readMessage();
    void onConnected();

private:
    QComboBox *hostCombo = nullptr;
    QLineEdit *portLineEdit = nullptr;
    QLabel *statusLabel = nullptr;
    QPushButton *connectButton = nullptr;
    QTcpSocket *tcpSocket = nullptr;
    QDataStream in;
    QString currentFortune;
    QNetworkSession *networkSession = nullptr;
    QTextEdit *messageDisplay;
    QLineEdit *sendMessage;
    QPushButton *sendMessageButton;
};

#endif
