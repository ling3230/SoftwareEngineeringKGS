#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "core/repository/knowledgerepository.h"
#include "core/entity/knowledgepoint.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle("软件工程知识图谱 - 个性化学习系统");

    knowledgeRepo = KnowledgeRepository::getInstance();

    // 初始化界面
    initUI();
    loadKnowledgeFromRepository();  // 使用Repository加载数据
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initUI()
{
    // 设置树形控件
    ui->treeWidget->setHeaderLabel("知识图谱");
    ui->treeWidget->setColumnCount(2);
    ui->treeWidget->setHeaderLabels(QStringList() << "知识点" << "状态");

    // 连接信号槽
    connect(ui->treeWidget, &QTreeWidget::itemClicked,
            this, &MainWindow::onTreeItemClicked);
}

void MainWindow::loadKnowledgeFromRepository()
{
    if (!knowledgeRepo) {
        qWarning() << "Repository未初始化";
        ui->plainTextEdit->setPlainText("错误：知识库未初始化");
        return;
    }

    // 清空树形控件
    ui->treeWidget->clear();

    // 从Repository获取所有知识点
    QList<KnowledgePoint> allKnowledge = knowledgeRepo->findAll();

    if (allKnowledge.isEmpty()) {
        qDebug() << "数据库为空，显示测试数据";
        ui->plainTextEdit->setPlainText(
            "知识库为空，点击右侧'初始化数据'按钮添加测试数据"
            );
        return;
    }

    qDebug() << "从Repository加载" << allKnowledge.size() << "个知识点";

    // 创建ID到树节点的映射
    QMap<int, QTreeWidgetItem*> itemMap;

    // 第一遍：创建所有节点
    for (const KnowledgePoint& kp : allKnowledge) {
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(0, kp.getTitle());
        item->setText(1, kp.getIsLearned() ? "✓ 已学" : "○ 未学");

        // 保存知识点ID到item的数据中
        item->setData(0, Qt::UserRole, kp.getId());

        // 根据掌握程度设置颜色
        if (kp.getMasteryLevel() >= 80) {
            item->setForeground(1, QBrush(Qt::darkGreen));
        } else if (kp.getMasteryLevel() > 0) {
            item->setForeground(1, QBrush(Qt::darkBlue));
        }

        itemMap[kp.getId()] = item;
    }

    // 第二遍：建立父子关系
    int rootCount = 0;
    for (const KnowledgePoint& kp : allKnowledge) {
        QTreeWidgetItem* item = itemMap[kp.getId()];
        int parentId = kp.getParentId();

        if (parentId > 0 && itemMap.contains(parentId)) {
            // 有父节点，添加到父节点下
            itemMap[parentId]->addChild(item);
        } else {
            // 根节点，添加到树控件
            ui->treeWidget->addTopLevelItem(item);
            rootCount++;
        }
    }

    // 展开所有节点
    ui->treeWidget->expandAll();

    // 显示统计信息
    ui->plainTextEdit->setPlainText(
        QString("知识库加载完成！\n\n"
                "统计信息：\n"
                "• 总知识点数：%1\n"
                "• 根节点数：%2\n"
                "• 点击左侧节点查看详情")
            .arg(allKnowledge.size())
            .arg(rootCount)
        );
}

void MainWindow::onTreeItemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    if (!item || !knowledgeRepo) return;

    // 获取知识点ID
    int knowledgeId = item->data(0, Qt::UserRole).toInt();

    if (knowledgeId > 0) {
        showKnowledgeDetails(knowledgeId);
    } else {
        ui->plainTextEdit->setPlainText(
            "点击了测试节点（无数据库ID）\n"
            "等数据初始化后会有详细信息"
            );
    }
}

void MainWindow::showKnowledgeDetails(int knowledgeId)
{
    KnowledgePoint kp = knowledgeRepo->findById(knowledgeId);

    if (kp.getId() <= 0) {
        ui->plainTextEdit->setPlainText("错误：未找到知识点");
        return;
    }

    QString details = QString(
                          "=== %1 ===\n\n"
                          "ID: %2\n"
                          "描述: %3\n\n"
                          "分类信息：\n"
                          "• 类型: %4\n"
                          "• 难度: %5\n"
                          "• 预计学习时间: %6分钟\n"
                          "• 父节点ID: %7\n\n"
                          "学习状态：\n"
                          "• 状态: %8\n"
                          "• 掌握程度: %9/100\n\n"
                          "先修知识点: %10\n"
                          "子知识点: %11"
                          ).arg(kp.getTitle())
                          .arg(kp.getId())
                          .arg(kp.getDescription().isEmpty() ? "（无描述）" : kp.getDescription())
                          .arg(kp.getTypeString())
                          .arg(kp.getDifficultyString())
                          .arg(kp.getEstimatedTime())
                          .arg(kp.getParentId() > 0 ? QString::number(kp.getParentId()) : "无")
                          .arg(kp.getIsLearned() ? "已学习" : "未学习")
                          .arg(kp.getMasteryLevel())
                          .arg(kp.getPrerequisiteIds().isEmpty() ? "无" :
                                   QString("%1个").arg(kp.getPrerequisiteIds().size()))
                          .arg(kp.getChildIds().isEmpty() ? "无" :
                                   QString("%1个").arg(kp.getChildIds().size()));

    ui->plainTextEdit->setPlainText(details);
}
