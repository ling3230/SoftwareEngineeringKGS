#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QCloseEvent>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();

    // 获取输入数据
    QString getUsername() const;
    QString getPassword() const;
    bool isRememberMeChecked() const;

signals:
    void loginRequested(const QString &username, const QString &password);
    void guestLoginRequested();
    void dialogClosed();

public slots:
    void setLoginEnabled(bool enabled);
    void showErrorMessage(const QString &message);
    void clearErrorMessage();
    void clearPassword();

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void on_showPasswordCheck_toggled(bool checked);
    void validateInput();
    void on_loginButton_clicked();
    void on_guestButton_clicked();
    void on_forgotButton_clicked();

private:
    Ui::LoginDialog *ui;
    void setupConnections();
    bool validateUsername() const;
    bool validatePassword() const;
};

#endif // LOGINDIALOG_H
