#include <QApplication>
#include "src/ui/mainwindow.h"
#include "src/ui/logindialog.h"
#include "src/ui/careerselectiondialog.h"  // 添加职业选择对话框
#include "src/core/service/loginservice.h"
#include "src/core/service/careerservice.h"     // 添加职业服务
#include "src/core/database/databasemanager.h"
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 设置应用信息
    app.setApplicationName("SoftwareEngineeringKGS");
    app.setOrganizationName("SE_Course");

    // ========== 重要：先连接数据库 ==========
    DatabaseManager* dbManager = DatabaseManager::getInstance();
    if (!dbManager->connect("data.db")) {  // 连接数据库，数据库文件名为 data.db
        qWarning() << "数据库连接失败，程序无法启动！";
        QMessageBox::critical(nullptr, "错误",
                              "无法连接到数据库，程序将退出。\n"
                              "请确保应用程序有写入权限。");
        return -1;  // 退出程序
    }
    qDebug() << "数据库连接成功，开始初始化服务...";
    // ======================================

    // 创建登录对话框
    LoginDialog loginDialog;

    // 获取服务单例
    LoginService* loginService = LoginService::getInstance();
    CareerService* careerService = CareerService::getInstance();

    // 连接登录信号
    QObject::connect(&loginDialog, &LoginDialog::loginRequested, [&](const QString &username, const QString &password) {
                         if (loginService->login(username, password)) {
                             QMessageBox::information(&loginDialog, "登录成功",QString("欢迎回来，%1！").arg(username));
                             loginDialog.close();

                             // 获取当前用户
                             UserProfile currentUser = loginService->getCurrentUser();

                             // 检查是否需要选择职业方向
                             if (careerService->shouldSelectCareer(currentUser.getId())) {
                                 // 显示职业选择对话框
                                 CareerSelectionDialog careerDialog;
                                 careerDialog.setAvailableCareers(careerService->getAllCareerPaths());

                                 // 获取所有可用的职业方向
                                 QList<CareerPath> careers = careerService->getAllCareerPaths();
                                 careerDialog.setAvailableCareers(careers);

                                 // 连接职业选择信号
                                 QObject::connect(&careerDialog,
                                                  &CareerSelectionDialog::careerSelectionCompleted,
                                                  [&](const CareerPath& career, const TechStack& techStack) {
                                                      careerService->setUserCareerSelection(currentUser.getId(),
                                                                                            career.getDirection(),
                                                                                            techStack.getId());
                                                      currentUser.setCareerDirection(career.getDirection());
                                                      currentUser.setTechStackId(techStack.getId());
                                                      QMessageBox::information(nullptr,
                                                                               "选择成功",
                                                                               techStack.getName().isEmpty()
                                                                                   ? QString("您已选择: %1\n系统将为您展示相应的学习路径。")
                                                                                         .arg(career.getName())
                                                                                   : QString("您已选择: %1\n技术栈: %2\n系统将为您展示相应的学习路径。")
                                                                                         .arg(career.getName(), techStack.getName()));

                                                      // ===== 修改：打开主窗口 =====
                                                      MainWindow* mainWindow = new MainWindow(currentUser);
                                                      mainWindow->show();
                                                  });

                                 // 连接跳过选择信号
                                 QObject::connect(&careerDialog, &CareerSelectionDialog::selectionSkipped,[&]() {
                                                      QMessageBox::information(nullptr, "使用默认设置",
                                                                               "将使用软件工程必修课程作为您的学习方向。");

                                                      // TODO: 打开主窗口，使用默认职业
                                                      QMessageBox::information(nullptr, "系统提示",
                                                                               "即将进入知识图谱主界面\n使用默认职业方向: 软件工程必修课");
                                                      app.quit();
                                                  });

                                 careerDialog.exec();
                             } else {
                                 // 已经有职业方向，直接进入主界面
                                 CareerPath currentCareer = careerService->getCurrentUserCareer(currentUser.getId());
                                 TechStack currentTechStack = careerService->getCurrentUserTechStack(currentUser.getId());
                                 QMessageBox::information(nullptr, "欢迎回来",
                                                         currentTechStack.getName().isEmpty()
                                                             ? QString("欢迎回来，%1！\n当前职业方向: %2")
                                                                   .arg(username)
                                                                   .arg(currentCareer.getName())
                                                             : QString("欢迎回来，%1！\n当前职业方向: %2\n当前技术栈: %3")
                                                                   .arg(username)
                                                                   .arg(currentCareer.getName(), currentTechStack.getName()));

                                 // ===== 修改：打开主窗口 =====
                                 MainWindow* mainWindow = new MainWindow(currentUser);
                                 mainWindow->show();
                             }
                         } else {
                             loginDialog.showErrorMessage("用户名或密码错误");
                             loginDialog.clearPassword();
                         }
                     });

    // 连接游客登录信号
    QObject::connect(&loginDialog, &LoginDialog::guestLoginRequested, [&]() {
        UserProfile guestUser = loginService->loginAsGuest();
        QMessageBox::information(&loginDialog, "游客模式",
                                 "您已进入游客体验模式。\n请注意：学习进度将不会保存。");
        loginDialog.close();

        // 游客模式也需要选择职业方向（但不保存）
        CareerSelectionDialog careerDialog;
        careerDialog.setAvailableCareers(careerService->getAllCareerPaths());

        QObject::connect(&careerDialog, &CareerSelectionDialog::careerSelectionCompleted,
                         [&](const CareerPath& career, const TechStack& techStack) {
                             QMessageBox::information(nullptr, "游客模式",
                                                      techStack.getName().isEmpty()
                                                          ? QString("您选择了: %1\n在游客模式下，您的选择不会保存。")
                                                                .arg(career.getName())
                                                          : QString("您选择了: %1\n技术栈: %2\n在游客模式下，您的选择不会保存。")
                                                                .arg(career.getName(), techStack.getName()));

                             // TODO: 打开主窗口（游客模式）
                             QMessageBox::information(nullptr, "系统提示",
                                                      techStack.getName().isEmpty()
                                                          ? QString("游客模式进入主界面\n职业方向: %1\n（功能受限，进度不保存）")
                                                                .arg(career.getName())
                                                          : QString("游客模式进入主界面\n职业方向: %1\n技术栈: %2\n（功能受限，进度不保存）")
                                                                .arg(career.getName(), techStack.getName()));
                             app.quit();
                         });

        QObject::connect(&careerDialog, &CareerSelectionDialog::selectionSkipped,
                         [&]() {
                             QMessageBox::information(nullptr, "游客模式",
                                                      "您跳过了职业选择，将使用默认设置。");

                             // TODO: 打开主窗口（游客模式）
                             QMessageBox::information(nullptr, "系统提示",
                                                      "游客模式进入主界面\n使用默认职业方向\n（功能受限，进度不保存）");
                             app.quit();
                         });

        careerDialog.exec();
    });

    // 显示登录对话框
    loginDialog.show();

    return app.exec();
}
