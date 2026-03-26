#include "logindialog.h"
#include "ui_logindialog.h"

#include <QMessageBox>
#include <QDebug>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    // 设置窗口属性
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedSize(size());

    // 设置焦点策略
    ui->usernameEdit->setFocus();

    // 连接信号槽
    setupConnections();

    // 初始验证
    validateInput();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::setupConnections()
{
    // 输入变化时验证
    connect(ui->usernameEdit, &QLineEdit::textChanged,
            this, &LoginDialog::validateInput);
    connect(ui->passwordEdit, &QLineEdit::textChanged,
            this, &LoginDialog::validateInput);

    // 连接UI自动生成的槽（命名规则：on_控件名_信号名）
    // 这些连接会在setupUi中自动建立，不需要手动连接

    // 回车键登录
    connect(ui->usernameEdit, &QLineEdit::returnPressed,
            ui->loginButton, &QPushButton::click);
    connect(ui->passwordEdit, &QLineEdit::returnPressed,
            ui->loginButton, &QPushButton::click);
}

QString LoginDialog::getUsername() const
{
    return ui->usernameEdit->text().trimmed();
}

QString LoginDialog::getPassword() const
{
    return ui->passwordEdit->text();
}

bool LoginDialog::isRememberMeChecked() const
{
    return ui->rememberMeCheck->isChecked();
}

void LoginDialog::setLoginEnabled(bool enabled)
{
    ui->loginButton->setEnabled(enabled);
}

void LoginDialog::showErrorMessage(const QString &message)
{
    ui->statusLabel->setText(message);
    ui->statusLabel->setStyleSheet("color: #e74c3c;");

    // 标记输入框错误状态
    QString errorStyle = "border-color: #e74c3c; background-color: #fff5f5;";
    if (!validateUsername()) {
        ui->usernameEdit->setStyleSheet(errorStyle);
    }
    if (!validatePassword()) {
        ui->passwordEdit->setStyleSheet(errorStyle);
    }
}

void LoginDialog::clearErrorMessage()
{
    ui->statusLabel->clear();

    // 恢复输入框样式
    QString normalStyle = "";
    ui->usernameEdit->setStyleSheet(normalStyle);
    ui->passwordEdit->setStyleSheet(normalStyle);
}

void LoginDialog::clearPassword()
{
    ui->passwordEdit->clear();
    ui->passwordEdit->setFocus();
}

bool LoginDialog::validateUsername() const
{
    QString username = getUsername();
    return !username.isEmpty() && username.length() >= 3;
}

bool LoginDialog::validatePassword() const
{
    return !ui->passwordEdit->text().isEmpty();
}

void LoginDialog::validateInput()
{
    bool isValid = validateUsername() && validatePassword();
    setLoginEnabled(isValid);

    if (isValid) {
        clearErrorMessage();
    }
}

void LoginDialog::on_showPasswordCheck_toggled(bool checked)
{
    ui->passwordEdit->setEchoMode(
        checked ? QLineEdit::Normal : QLineEdit::Password
        );
}

void LoginDialog::on_loginButton_clicked()
{
    QString username = getUsername();
    QString password = getPassword();

    if (!validateUsername() || !validatePassword()) {
        showErrorMessage("请输入有效的用户名和密码");
        return;
    }

    qDebug() << "登录请求: 用户名=" << username;
    emit loginRequested(username, password);
}

void LoginDialog::on_guestButton_clicked()
{
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "游客模式",
        "您将以游客身份进入系统，部分功能可能受限。\n是否继续？",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        qDebug() << "游客登录请求";
        emit guestLoginRequested();
    }
}

void LoginDialog::on_forgotButton_clicked()
{
    QMessageBox::information(
        this,
        "忘记密码",
        "请联系系统管理员重置密码。\n\n"
        "演示账号：\n"
        "• admin / admin123\n"
        "• student / student123\n"
        "• test / test123"
        );
}

void LoginDialog::closeEvent(QCloseEvent *event)
{
    emit dialogClosed();
    event->accept();
}
