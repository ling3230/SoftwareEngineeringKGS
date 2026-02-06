#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlQuery>
#include <QSharedPointer>
#include <QTreeWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class KnowledgeRepository;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onTreeItemClicked(QTreeWidgetItem *item, int column);

private:
    Ui::MainWindow *ui;
    KnowledgeRepository* knowledgeRepo;

    void initUI();
    void loadKnowledgeFromRepository();  // 新方法：从Repository加载
    void showKnowledgeDetails(int knowledgeId);

    // 测试数据初始化（开发阶段用）
    void initTestData();

};
#endif // MAINWINDOW_H
