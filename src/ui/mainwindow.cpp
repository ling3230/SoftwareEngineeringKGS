#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "core/repository/knowledgerepository.h"
#include "core/entity/knowledgepoint.h"
#include "src/core/service/careerservice.h"
#include "src/core/service/loginservice.h"
#include "src/ui/careerselectiondialog.h"
#include "src/ui/logindialog.h"
#include <QApplication>
#include <QBrush>
#include <QColor>
#include <QDebug>
#include <QFont>
#include <QHeaderView>
#include <QMessageBox>
#include <QMenu>
#include <QQueue>
#include <QSet>
#include <QSize>
#include <QStyle>
#include <algorithm>

namespace {

const QString kDefaultTechStackId = "backend_java";
constexpr int kRoleIsCore = Qt::UserRole + 2;
constexpr int kRoleIsLearned = Qt::UserRole + 3;

QString buildLevelStatusText(int depth, bool hasChildren, bool isLearned)
{
    QString levelLabel;
    if (depth == 0) {
        levelLabel = QStringLiteral("主线");
    } else if (hasChildren) {
        levelLabel = QStringLiteral("模块");
    } else {
        levelLabel = QStringLiteral("技能");
    }

    return QStringLiteral("%1 · %2")
        .arg(levelLabel, isLearned ? QStringLiteral("已学") : QStringLiteral("待学"));
}

bool containsTagIgnoreCase(const QList<QString>& tags, const QString& expected)
{
    for (const QString& tag : tags) {
        if (tag.compare(expected, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

QString careerDirectionKey(CareerDirection direction)
{
    switch (direction) {
    case CareerDirection::DEFAULT_SE_COURSE:
        return QStringLiteral("se");
    case CareerDirection::FRONTEND_ENGINEER:
        return QStringLiteral("frontend");
    case CareerDirection::BACKEND_ENGINEER:
        return QStringLiteral("backend");
    case CareerDirection::FULLSTACK_ENGINEER:
        return QStringLiteral("fullstack");
    case CareerDirection::MOBILE_DEVELOPER:
        return QStringLiteral("mobile");
    case CareerDirection::DEVOPS_ENGINEER:
        return QStringLiteral("devops");
    case CareerDirection::TEST_ENGINEER:
        return QStringLiteral("test");
    case CareerDirection::PROJECT_MANAGER:
        return QStringLiteral("pm");
    case CareerDirection::DATA_SCIENTIST:
        return QStringLiteral("data");
    case CareerDirection::AI_ENGINEER:
        return QStringLiteral("ai");
    case CareerDirection::GAME_DEVELOPER:
        return QStringLiteral("game");
    case CareerDirection::CYBERSECURITY_ENGINEER:
        return QStringLiteral("security");
    }

    return QStringLiteral("se");
}

bool isCareerTrunkPoint(const KnowledgePoint& kp, CareerDirection direction)
{
    const QString directionTag = QStringLiteral("career:%1").arg(careerDirectionKey(direction));
    if (containsTagIgnoreCase(kp.getTags(), directionTag)
        || containsTagIgnoreCase(kp.getTags(), QStringLiteral("trunk"))
        || containsTagIgnoreCase(kp.getTags(), QStringLiteral("mainline"))) {
        return true;
    }

    return containsTagIgnoreCase(kp.getTags(), QStringLiteral("foundation"))
           || containsTagIgnoreCase(kp.getTags(), QStringLiteral("common"));
}

bool isTechStackBranchPoint(const KnowledgePoint& kp, const QString& techStackId)
{
    if (techStackId.trimmed().isEmpty()) {
        return false;
    }

    if (kp.getCategory().compare(techStackId, Qt::CaseInsensitive) == 0) {
        return true;
    }

    const QString stackTag = QStringLiteral("stack:%1").arg(techStackId.trimmed());
    return containsTagIgnoreCase(kp.getTags(), stackTag)
           || containsTagIgnoreCase(kp.getTags(), techStackId.trimmed());
}

bool isCrossStackLanguageNode(const KnowledgePoint& kp, const QString& activeStackId)
{
    const QString stack = activeStackId.trimmed().toLower();
    if (!stack.startsWith("backend_")) {
        return false;
    }

    const QString category = kp.getCategory().trimmed().toLower();
    if (!category.startsWith("backend_") || category == stack) {
        return false;
    }

    if (containsTagIgnoreCase(kp.getTags(), "foundation")
        || containsTagIgnoreCase(kp.getTags(), "common")) {
        return false;
    }

    // 其他技术栈语言标签节点在当前栈下隐藏，避免出现“C++ 学 Java 基础”的困惑。
    return containsTagIgnoreCase(kp.getTags(), "language")
           || containsTagIgnoreCase(kp.getTags(), "java_required")
           || containsTagIgnoreCase(kp.getTags(), "python")
           || containsTagIgnoreCase(kp.getTags(), "go")
           || containsTagIgnoreCase(kp.getTags(), "node_required")
           || containsTagIgnoreCase(kp.getTags(), "rust_required");
}

QString summarizeStageTitle(int stageIndex, const QList<KnowledgePoint>& stageKnowledge)
{
    bool hasFoundation = false;
    bool hasLanguageRuntime = false;
    bool hasFrameworkData = false;
    bool hasArchitecture = false;
    bool hasEngineering = false;

    for (const KnowledgePoint& kp : stageKnowledge) {
        if (containsTagIgnoreCase(kp.getTags(), "foundation")
            || containsTagIgnoreCase(kp.getTags(), "common")) {
            hasFoundation = true;
        }
        if (containsTagIgnoreCase(kp.getTags(), "language")
            || containsTagIgnoreCase(kp.getTags(), "runtime")
            || containsTagIgnoreCase(kp.getTags(), "branch")) {
            hasLanguageRuntime = true;
        }
        if (containsTagIgnoreCase(kp.getTags(), "framework")
            || containsTagIgnoreCase(kp.getTags(), "database")
            || containsTagIgnoreCase(kp.getTags(), "middleware")) {
            hasFrameworkData = true;
        }
        if (containsTagIgnoreCase(kp.getTags(), "architecture")
            || containsTagIgnoreCase(kp.getTags(), "network")
            || containsTagIgnoreCase(kp.getTags(), "concurrency")) {
            hasArchitecture = true;
        }
        if (containsTagIgnoreCase(kp.getTags(), "devops")) {
            hasEngineering = true;
        }
    }

    if (hasFoundation) {
        return QStringLiteral("基础能力准备");
    }
    if (hasLanguageRuntime) {
        return QStringLiteral("语言与运行时");
    }
    if (hasFrameworkData) {
        return QStringLiteral("框架与数据能力");
    }
    if (hasArchitecture) {
        return QStringLiteral("系统与架构能力");
    }
    if (hasEngineering) {
        return QStringLiteral("工程化与部署能力");
    }

    return QStringLiteral("阶段%1学习内容").arg(stageIndex + 1);
}

void addDependencyClosure(const QHash<QString, KnowledgePoint>& pointById, QSet<QString>& selectedIds)
{
    bool changed = true;
    while (changed) {
        changed = false;
        const QList<QString> snapshot = selectedIds.values();
        for (const QString& nodeId : snapshot) {
            if (!pointById.contains(nodeId)) {
                continue;
            }

            const KnowledgePoint kp = pointById.value(nodeId);
            for (const QString& preId : kp.getPrerequisiteIds()) {
                if (pointById.contains(preId) && !selectedIds.contains(preId)) {
                    selectedIds.insert(preId);
                    changed = true;
                }
            }

            const QString parentId = kp.getPrimaryParentId();
            if (!parentId.isEmpty() && pointById.contains(parentId) && !selectedIds.contains(parentId)) {
                selectedIds.insert(parentId);
                changed = true;
            }
        }
    }
}

QList<KnowledgePoint> topologicalSortByPrerequisite(const QList<KnowledgePoint>& points)
{
    QHash<QString, KnowledgePoint> pointById;
    QHash<QString, int> indegree;
    QHash<QString, QList<QString>> nextMap;
    for (const KnowledgePoint& kp : points) {
        pointById.insert(kp.getId(), kp);
    }

    for (const KnowledgePoint& kp : points) {
        indegree.insert(kp.getId(), 0);
    }

    for (const KnowledgePoint& kp : points) {
        for (const QString& preId : kp.getPrerequisiteIds()) {
            if (!pointById.contains(preId)) {
                continue;
            }
            nextMap[preId].append(kp.getId());
            indegree[kp.getId()] = indegree.value(kp.getId()) + 1;
        }
    }

    QList<QString> zeroList;
    for (auto it = indegree.constBegin(); it != indegree.constEnd(); ++it) {
        if (it.value() == 0) {
            zeroList.append(it.key());
        }
    }

    std::sort(zeroList.begin(), zeroList.end(), [&](const QString& lhs, const QString& rhs) {
        const KnowledgePoint a = pointById.value(lhs);
        const KnowledgePoint b = pointById.value(rhs);
        if (a.getImportance() != b.getImportance()) {
            return a.getImportance() > b.getImportance();
        }
        return a.getTitle().localeAwareCompare(b.getTitle()) < 0;
    });

    QQueue<QString> queue;
    for (const QString& nodeId : zeroList) {
        queue.enqueue(nodeId);
    }

    QList<KnowledgePoint> ordered;
    while (!queue.isEmpty()) {
        const QString currentId = queue.dequeue();
        ordered.append(pointById.value(currentId));

        QList<QString> nextIds = nextMap.value(currentId);
        std::sort(nextIds.begin(), nextIds.end(), [&](const QString& lhs, const QString& rhs) {
            const KnowledgePoint a = pointById.value(lhs);
            const KnowledgePoint b = pointById.value(rhs);
            if (a.getImportance() != b.getImportance()) {
                return a.getImportance() > b.getImportance();
            }
            return a.getTitle().localeAwareCompare(b.getTitle()) < 0;
        });

        for (const QString& nextId : nextIds) {
            const int newDegree = indegree.value(nextId) - 1;
            indegree[nextId] = newDegree;
            if (newDegree == 0) {
                queue.enqueue(nextId);
            }
        }
    }

    if (ordered.size() == points.size()) {
        return ordered;
    }

    // 若存在环路或脏数据，追加未入队节点，保证路径可展示。
    QSet<QString> added;
    for (const KnowledgePoint& kp : ordered) {
        added.insert(kp.getId());
    }

    QList<KnowledgePoint> remaining;
    for (const KnowledgePoint& kp : points) {
        if (!added.contains(kp.getId())) {
            remaining.append(kp);
        }
    }

    std::sort(remaining.begin(), remaining.end(), [](const KnowledgePoint& a, const KnowledgePoint& b) {
        if (a.getImportance() != b.getImportance()) {
            return a.getImportance() > b.getImportance();
        }
        return a.getTitle().localeAwareCompare(b.getTitle()) < 0;
    });

    ordered.append(remaining);
    return ordered;
}

void applyTreeItemStyle(QTreeWidgetItem* item, int depth, QStyle* style)
{
    if (!item) {
        return;
    }

    const bool hasChildren = item->childCount() > 0;
    const bool isCore = item->data(0, kRoleIsCore).toBool();
    const bool isLearned = item->data(0, kRoleIsLearned).toBool();

    QFont titleFont = item->font(0);
    titleFont.setBold(depth <= 1 || isCore);
    titleFont.setPointSize(depth == 0 ? 11 : (depth == 1 ? 10 : 9));
    item->setFont(0, titleFont);

    QFont statusFont = item->font(1);
    statusFont.setBold(depth <= 1);
    item->setFont(1, statusFont);

    const QColor titleColor = depth == 0
        ? QColor("#16324f")
        : (depth == 1 ? QColor("#24527a") : QColor("#334155"));
    const QColor statusColor = isLearned ? QColor("#1f7a4c") : QColor("#8a6d1d");
    const QColor rowColor = depth == 0
        ? QColor("#eaf4ff")
        : (hasChildren ? QColor("#f7fbff") : QColor("#ffffff"));

    item->setForeground(0, QBrush(titleColor));
    item->setForeground(1, QBrush(statusColor));
    item->setBackground(0, QBrush(rowColor));
    item->setBackground(1, QBrush(rowColor));
    item->setTextAlignment(1, Qt::AlignCenter);
    item->setSizeHint(0, QSize(0, depth == 0 ? 38 : (depth == 1 ? 34 : 30)));
    item->setText(1, buildLevelStatusText(depth, hasChildren, isLearned));

    if (style) {
        if (depth == 0) {
            item->setIcon(0, style->standardIcon(QStyle::SP_DirHomeIcon));
        } else if (hasChildren) {
            item->setIcon(0, style->standardIcon(QStyle::SP_DirIcon));
        } else {
            item->setIcon(0, style->standardIcon(QStyle::SP_FileIcon));
        }
    }

    for (int index = 0; index < item->childCount(); ++index) {
        applyTreeItemStyle(item->child(index), depth + 1, style);
    }
}

}

MainWindow::MainWindow(const UserProfile& user,QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , currentUser(user)
{
    ui->setupUi(this);
    this->setWindowTitle("软件工程知识图谱 - 个性化学习系统");

    // 获取服务
    knowledgeRepo = KnowledgeRepository::getInstance();
    CareerService* careerService = CareerService::getInstance();

    // 更新用户信息显示
    ui->userInfoLabel->setText(QString("👤 %1").arg(currentUser.getDisplayName()));

    // 获取当前职业并显示
    if (currentUser.getId() > 0) {
        CareerPath currentCareer = careerService->getCurrentUserCareer(currentUser.getId());
        ui->careerLabel->setText(QString("职业: %1").arg(currentCareer.getName()));
        TechStack currentTechStack = careerService->getCurrentUserTechStack(currentUser.getId());
        ui->currentCareerLabel->setText(currentTechStack.getName().isEmpty()
                                            ? currentCareer.getName()
                                            : QString("%1 · %2").arg(currentCareer.getName(), currentTechStack.getName()));
    } else {
        // 游客模式
        ui->careerLabel->setText("职业: 游客模式");
        ui->currentCareerLabel->setText("游客");
    }

    // 初始化界面
    initUI();

    // 加载知识点
    loadKnowledgeFromRepository();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initUI()
{
    detailStack = new QStackedWidget(this);
    graphWidget = new KnowledgeGraph3DWidget(this);

    // 使用堆叠容器在详情文本和三维图谱之间切换显示
    ui->detailLayout->removeWidget(ui->detailContent);
    detailStack->addWidget(ui->detailContent);
    detailStack->addWidget(graphWidget);
    detailStack->setCurrentWidget(ui->detailContent);
    ui->detailLayout->addWidget(detailStack);
    connect(ui->recommendToggleBtn, &QPushButton::clicked, this, [this]() {
        setRecommendationPanelVisible(!isRecommendationPanelVisible());
    });
    setRecommendationPanelVisible(true);

    // 设置树形控件
    ui->knowledgeTree->setHeaderLabels(QStringList() << "学习路径" << "层级 / 状态");
    ui->knowledgeTree->setRootIsDecorated(true);
    ui->knowledgeTree->setIndentation(28);
    ui->knowledgeTree->setUniformRowHeights(true);
    ui->knowledgeTree->setExpandsOnDoubleClick(true);
    ui->knowledgeTree->setAnimated(true);
    ui->knowledgeTree->setColumnWidth(1, 110);
    ui->knowledgeTree->header()->setStretchLastSection(false);
    ui->knowledgeTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->knowledgeTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);

    // 初始状态
    ui->detailContent->setPlainText("欢迎使用知识图谱系统！\n\n点击左侧知识点查看详情。");

    // 更新统计信息
    updateStats();
}

void MainWindow::loadKnowledgeFromRepository()
{
    if (!knowledgeRepo) {
        qWarning() << "Repository未初始化";
        ui->detailContent->setPlainText("错误：知识库未初始化");
        return;
    }

    // 清空树形控件
    ui->knowledgeTree->clear();

    // 从Repository获取所有知识点
    QList<KnowledgePoint> allKnowledge = knowledgeRepo->findAll();

    if (allKnowledge.isEmpty()) {
        qDebug() << "数据库为空，初始化测试数据";
        initTestData();
        // 重新加载
        allKnowledge = knowledgeRepo->findAll();

        if (allKnowledge.isEmpty()) {
            ui->detailContent->setPlainText(
                "知识库为空，无法加载数据。\n"
                "请确保数据库连接正确。"
                );
            return;
        }
    }

    const QList<KnowledgePoint> visibleKnowledge = filterKnowledgeForActiveTechStack(allKnowledge);
    qDebug() << "从Repository加载" << allKnowledge.size() << "个知识点，可见" << visibleKnowledge.size() << "个";
    updatePersonaCard(visibleKnowledge);
    updateRecommendationPanel(visibleKnowledge);

    // 按“学习阶段”构建树，避免分支过深导致的可读性问题。
    QHash<QString, KnowledgePoint> pointById;
    QHash<QString, int> indegree;
    QHash<QString, QList<QString>> adjacency;
    QHash<QString, int> stageById;

    for (const KnowledgePoint& kp : visibleKnowledge) {
        pointById.insert(kp.getId(), kp);
        indegree.insert(kp.getId(), 0);
        stageById.insert(kp.getId(), 0);
    }

    for (const KnowledgePoint& kp : visibleKnowledge) {
        for (const QString& preId : kp.getPrerequisiteIds()) {
            if (!pointById.contains(preId)) {
                continue;
            }
            adjacency[preId].append(kp.getId());
            indegree[kp.getId()] = indegree.value(kp.getId()) + 1;
        }
    }

    QList<QString> zeroInDegreeIds;
    for (auto it = indegree.constBegin(); it != indegree.constEnd(); ++it) {
        if (it.value() == 0) {
            zeroInDegreeIds.append(it.key());
        }
    }

    std::sort(zeroInDegreeIds.begin(), zeroInDegreeIds.end(), [&](const QString& lhs, const QString& rhs) {
        const KnowledgePoint a = pointById.value(lhs);
        const KnowledgePoint b = pointById.value(rhs);
        if (a.getImportance() != b.getImportance()) {
            return a.getImportance() > b.getImportance();
        }
        return a.getTitle().localeAwareCompare(b.getTitle()) < 0;
    });

    QQueue<QString> queue;
    for (const QString& id : zeroInDegreeIds) {
        queue.enqueue(id);
    }

    QSet<QString> visited;
    while (!queue.isEmpty()) {
        const QString currentId = queue.dequeue();
        visited.insert(currentId);
        const int currentStage = stageById.value(currentId, 0);

        QList<QString> nextIds = adjacency.value(currentId);
        std::sort(nextIds.begin(), nextIds.end(), [&](const QString& lhs, const QString& rhs) {
            const KnowledgePoint a = pointById.value(lhs);
            const KnowledgePoint b = pointById.value(rhs);
            if (a.getImportance() != b.getImportance()) {
                return a.getImportance() > b.getImportance();
            }
            return a.getTitle().localeAwareCompare(b.getTitle()) < 0;
        });

        for (const QString& nextId : nextIds) {
            stageById[nextId] = qMax(stageById.value(nextId, 0), currentStage + 1);
            const int newDegree = indegree.value(nextId) - 1;
            indegree[nextId] = newDegree;
            if (newDegree == 0) {
                queue.enqueue(nextId);
            }
        }
    }

    for (const KnowledgePoint& kp : visibleKnowledge) {
        if (visited.contains(kp.getId())) {
            continue;
        }

        int fallbackStage = 0;
        for (const QString& preId : kp.getPrerequisiteIds()) {
            if (stageById.contains(preId)) {
                fallbackStage = qMax(fallbackStage, stageById.value(preId) + 1);
            }
        }
        stageById[kp.getId()] = fallbackStage;
    }

    QMap<int, QList<KnowledgePoint>> knowledgeByStage;
    for (const KnowledgePoint& kp : visibleKnowledge) {
        knowledgeByStage[stageById.value(kp.getId(), 0)].append(kp);
    }

    for (auto it = knowledgeByStage.begin(); it != knowledgeByStage.end(); ++it) {
        QList<KnowledgePoint> stageKnowledge = it.value();
        std::sort(stageKnowledge.begin(), stageKnowledge.end(), [](const KnowledgePoint& a, const KnowledgePoint& b) {
            if (a.getImportance() != b.getImportance()) {
                return a.getImportance() > b.getImportance();
            }
            return a.getTitle().localeAwareCompare(b.getTitle()) < 0;
        });

        QTreeWidgetItem* stageItem = new QTreeWidgetItem();
        const QString stageTitle = summarizeStageTitle(it.key(), stageKnowledge);
        stageItem->setText(0, QString("第 %1 阶段 · %2").arg(it.key() + 1).arg(stageTitle));
        stageItem->setText(1, QString("%1 个知识点").arg(stageKnowledge.size()));
        stageItem->setData(0, kRoleIsCore, true);
        stageItem->setData(0, kRoleIsLearned, false);
        stageItem->setToolTip(0, QString("建议按阶段顺序学习\n主题：%1\n当前阶段共 %2 个知识点")
                      .arg(stageTitle)
                      .arg(stageKnowledge.size()));

        for (const KnowledgePoint& kp : stageKnowledge) {
            QTreeWidgetItem* item = new QTreeWidgetItem();
            item->setText(0, kp.getTitle());
            item->setData(0, Qt::UserRole, kp.getId());
            item->setData(0, kRoleIsCore, kp.getIsCore());
            item->setData(0, kRoleIsLearned, kp.getIsLearned());
            item->setToolTip(0, QString("%1\n预计学习时间：%2 分钟")
                                  .arg(kp.getDescription().isEmpty() ? kp.getTitle() : kp.getDescription())
                                  .arg(kp.getEstimatedTime()));
            stageItem->addChild(item);
        }

        ui->knowledgeTree->addTopLevelItem(stageItem);
    }

    for (int index = 0; index < ui->knowledgeTree->topLevelItemCount(); ++index) {
        QTreeWidgetItem* topItem = ui->knowledgeTree->topLevelItem(index);
        applyTreeItemStyle(topItem, 0, style());
        topItem->setText(1, QString("%1 个知识点").arg(topItem->childCount()));
    }

    ui->knowledgeTree->collapseAll();
    ui->knowledgeTree->expandToDepth(1);

    // 更新统计信息
    ui->totalNodesValue->setText(QString::number(visibleKnowledge.size()));
    updateStats();

    if (graphWidget) {
        graphWidget->setActiveTechStack(QString());
        graphWidget->setKnowledgeData(visibleKnowledge);
    }

    // 显示欢迎信息
    ui->detailContent->setPlainText(
        QString("知识库加载完成！\n\n"
                "欢迎 %1 使用知识图谱系统\n"
                "当前职业: %2\n\n"
                "学习路径生成规则：职业主线 + 技术栈分支\n"
                "树形视图按学习阶段分组展示\n"
                "点击左侧知识点查看详细信息。")
            .arg(currentUser.getDisplayName())
            .arg(ui->currentCareerLabel->text())
        );
}

QString MainWindow::resolveActiveTechStackId() const
{
    const QString techStackId = currentUser.getTechStackId().trimmed();

    if (techStackId.compare("backend_java", Qt::CaseInsensitive) == 0
        || techStackId.compare("backend_python", Qt::CaseInsensitive) == 0
        || techStackId.compare("backend_go", Qt::CaseInsensitive) == 0) {
        return techStackId;
    }

    if (techStackId.compare("default_0", Qt::CaseInsensitive) == 0
        || techStackId.compare("default_2", Qt::CaseInsensitive) == 0
        || techStackId.isEmpty()) {
        return kDefaultTechStackId;
    }

    return techStackId;
}

QList<KnowledgePoint> MainWindow::filterKnowledgeForActiveTechStack(const QList<KnowledgePoint>& allKnowledge) const
{
    if (allKnowledge.isEmpty()) {
        return allKnowledge;
    }

    QHash<QString, KnowledgePoint> pointById;
    for (const KnowledgePoint& kp : allKnowledge) {
        pointById.insert(kp.getId(), kp);
    }

    const CareerDirection direction = currentUser.getCareerDirection();
    const QString activeStackId = resolveActiveTechStackId();

    QSet<QString> selectedIds;
    for (const KnowledgePoint& kp : allKnowledge) {
        if (isCareerTrunkPoint(kp, direction)) {
            selectedIds.insert(kp.getId());
        }
        if (isTechStackBranchPoint(kp, activeStackId)) {
            selectedIds.insert(kp.getId());
        }
    }

    if (selectedIds.isEmpty()) {
        QList<KnowledgePoint> fallback;
        fallback.reserve(allKnowledge.size());
        for (const KnowledgePoint& kp : allKnowledge) {
            if (activeStackId.isEmpty()
                || kp.getCategory().compare(activeStackId, Qt::CaseInsensitive) == 0) {
                fallback.append(kp);
            }
        }
        return topologicalSortByPrerequisite(fallback);
    }

    addDependencyClosure(pointById, selectedIds);

    QList<KnowledgePoint> mergedPath;
    mergedPath.reserve(selectedIds.size());
    for (const QString& nodeId : selectedIds) {
        if (pointById.contains(nodeId)) {
            const KnowledgePoint kp = pointById.value(nodeId);
            if (!isCrossStackLanguageNode(kp, activeStackId)) {
                mergedPath.append(kp);
            }
        }
    }

    return topologicalSortByPrerequisite(mergedPath);
}

void MainWindow::updatePersonaCard(const QList<KnowledgePoint>& visibleKnowledge)
{
    const QString displayName = currentUser.getDisplayName().trimmed().isEmpty()
        ? QStringLiteral("学习者")
        : currentUser.getDisplayName().trimmed();

    const QString activeStackId = resolveActiveTechStackId();
    QString stackLabel = activeStackId;
    if (activeStackId.compare(QStringLiteral("backend_java"), Qt::CaseInsensitive) == 0) {
        stackLabel = QStringLiteral("Java 技术栈");
    } else if (activeStackId.compare(QStringLiteral("backend_python"), Qt::CaseInsensitive) == 0) {
        stackLabel = QStringLiteral("Python 技术栈");
    } else if (activeStackId.compare(QStringLiteral("backend_go"), Qt::CaseInsensitive) == 0) {
        stackLabel = QStringLiteral("Go 技术栈");
    } else if (activeStackId.compare(QStringLiteral("backend_cpp"), Qt::CaseInsensitive) == 0) {
        stackLabel = QStringLiteral("C++ 技术栈");
    } else if (activeStackId.compare(QStringLiteral("backend_nodejs"), Qt::CaseInsensitive) == 0) {
        stackLabel = QStringLiteral("Node.js 技术栈");
    } else if (activeStackId.compare(QStringLiteral("backend_rust"), Qt::CaseInsensitive) == 0) {
        stackLabel = QStringLiteral("Rust 技术栈");
    } else if (activeStackId.startsWith(QStringLiteral("default_"))) {
        stackLabel = QStringLiteral("默认学习路径");
    }

    const int totalCount = visibleKnowledge.size();
    int learnedCount = 0;
    for (const KnowledgePoint& kp : visibleKnowledge) {
        if (kp.getIsLearned()) {
            ++learnedCount;
        }
    }

    int stage = 1;
    if (totalCount > 0) {
        stage = qMin(5, (learnedCount * 5) / qMax(1, totalCount) + 1);
    }

    ui->personaSummaryLabel->setText(
        QStringLiteral("%1 · %2").arg(ui->currentCareerLabel->text(), stackLabel));
    ui->personaFocusLabel->setText(
        QStringLiteral("%1，当前已完成 %2/%3 个知识点，推荐聚焦第 %4 阶段能力。")
            .arg(displayName)
            .arg(learnedCount)
            .arg(totalCount)
            .arg(stage));
}

void MainWindow::updateRecommendationPanel(const QList<KnowledgePoint>& visibleKnowledge)
{
    if (visibleKnowledge.isEmpty()) {
        ui->recommendationContent->setPlainText(QStringLiteral("当前学习路径为空，暂无推荐内容。"));
        return;
    }

    QHash<QString, KnowledgePoint> pointById;
    for (const KnowledgePoint& kp : visibleKnowledge) {
        pointById.insert(kp.getId(), kp);
    }

    struct RecommendationItem {
        KnowledgePoint point;
        int score = 0;
        QString reason;
    };

    QList<RecommendationItem> candidates;
    for (const KnowledgePoint& kp : visibleKnowledge) {
        if (kp.getIsLearned()) {
            continue;
        }

        bool learnable = true;
        int satisfiedPrerequisiteCount = 0;
        for (const QString& preId : kp.getPrerequisiteIds()) {
            if (!pointById.contains(preId)) {
                continue;
            }

            if (!pointById.value(preId).getIsLearned()) {
                learnable = false;
                break;
            }

            ++satisfiedPrerequisiteCount;
        }

        if (!learnable) {
            continue;
        }

        int score = 0;
        score += kp.getImportance() * 12;
        score += kp.getIsCore() ? 35 : 0;
        score += qMax(0, 100 - kp.getMasteryLevel()) / 4;
        score += (kp.getEstimatedTime() >= 20 && kp.getEstimatedTime() <= 90) ? 10 : 0;
        score -= qMax(0, kp.getEstimatedTime() - 120) / 4;

        const int difficultyLevel = static_cast<int>(kp.getDifficulty());
        score += qMax(0, 10 - qAbs(difficultyLevel - 3) * 3);

        QString reason = QStringLiteral("前置知识已满足");
        if (kp.getIsCore()) {
            reason = QStringLiteral("主线核心节点，建议优先学习");
        } else if (satisfiedPrerequisiteCount > 0) {
            reason = QStringLiteral("已衔接 %1 个前置知识，可立即推进").arg(satisfiedPrerequisiteCount);
        }

        candidates.append({kp, score, reason});
    }

    std::sort(candidates.begin(), candidates.end(), [](const RecommendationItem& a, const RecommendationItem& b) {
        if (a.score != b.score) {
            return a.score > b.score;
        }
        return a.point.getImportance() > b.point.getImportance();
    });

    if (candidates.isEmpty()) {
        ui->recommendationContent->setPlainText(
            QStringLiteral("当前没有可立即学习的新知识点。\n建议先完成左侧路径中未掌握的前置节点，再刷新推荐。"));
        return;
    }

    QStringList lines;
    lines.append(QStringLiteral("今日个性化推荐（Top 3）"));
    lines.append(QStringLiteral(""));

    const int topN = qMin(3, candidates.size());
    for (int index = 0; index < topN; ++index) {
        const RecommendationItem& item = candidates[index];
        lines.append(QStringLiteral("%1. %2").arg(index + 1).arg(item.point.getTitle()));
        lines.append(QStringLiteral("   预计时长：%1 分钟").arg(item.point.getEstimatedTime()));
        lines.append(QStringLiteral("   推荐理由：%1").arg(item.reason));
        lines.append(QStringLiteral("   重要性：%1，当前掌握度：%2%%")
                         .arg(item.point.getImportance())
                         .arg(item.point.getMasteryLevel()));
        lines.append(QStringLiteral(""));
    }

    ui->recommendationContent->setPlainText(lines.join(QStringLiteral("\n")));
}

bool MainWindow::isRecommendationPanelVisible() const
{
    return ui->recommendationContent->isVisible();
}

void MainWindow::setRecommendationPanelVisible(bool visible)
{
    ui->recommendationContent->setVisible(visible);
    ui->recommendToggleBtn->setText(visible ? QStringLiteral("隐藏推荐") : QStringLiteral("显示推荐"));
    if (detailStack) {
        detailStack->updateGeometry();
    }
}

void MainWindow::on_knowledgeTree_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    if (!item || !knowledgeRepo) return;

    // 获取知识点ID
    QString knowledgeId = item->data(0, Qt::UserRole).toString();

    if (!knowledgeId.isEmpty()) {
        if (graphWidget) {
            graphWidget->setFocusKnowledge(knowledgeId);
        }
        showKnowledgeDetails(knowledgeId);
    }
}

void MainWindow::on_graphBtn_clicked()
{
    if (!detailStack || !graphWidget) {
        return;
    }

    bool showingGraph = detailStack->currentWidget() == graphWidget;
    if (showingGraph) {
        detailStack->setCurrentWidget(ui->detailContent);
        ui->detailTitle->setText("知识点详情");
        ui->graphBtn->setText("浏览知识图谱");
        return;
    }

    QList<KnowledgePoint> allKnowledge = knowledgeRepo ? knowledgeRepo->findAll() : QList<KnowledgePoint>();
    const QList<KnowledgePoint> visibleKnowledge = filterKnowledgeForActiveTechStack(allKnowledge);
    graphWidget->setActiveTechStack(QString());
    graphWidget->setKnowledgeData(visibleKnowledge);
    detailStack->setCurrentWidget(graphWidget);
    ui->detailTitle->setText("三维知识图谱");
    ui->graphBtn->setText("返回知识详情");
}

void MainWindow::showKnowledgeDetails(const QString& knowledgeId)
{
    KnowledgePoint kp;
    bool ok = false;
    const int numericId = knowledgeId.toInt(&ok);
    if (ok && numericId > 0) {
        kp = knowledgeRepo->findById(numericId);
    } else {
        const QList<KnowledgePoint> allKnowledge = knowledgeRepo->findAll();
        for (const KnowledgePoint& candidate : allKnowledge) {
            if (candidate.getId() == knowledgeId) {
                kp = candidate;
                break;
            }
        }
    }

    if (kp.getId().isEmpty()) {
        ui->detailContent->setPlainText("错误：未找到知识点");
        return;
    }

    QString details = QString(
                          "【%1】\n\n"
                          "━━━━━━━━━━━━━━━━━━━━━━\n\n"
                          "📝 描述：\n%2\n\n"
                          "━━━━━━━━━━━━━━━━━━━━━━\n\n"
                          "📊 详细信息：\n"
                          "• 知识点ID：%3\n"
                          "• 类　　型：%4\n"
                          "• 难　　度：%5\n"
                          "• 学习时间：%6 分钟\n\n"
                          "📈 学习状态：\n"
                          "• 当前状态：%7\n"
                          "• 掌握程度：%8%%\n\n"
                          "🔗 关联知识点：\n"
                          "• 前置知识：%9\n"
                          "• 后续知识：%10"
                          ).arg(kp.getTitle())
                          .arg(kp.getDescription().isEmpty() ? "（暂无描述）" : kp.getDescription())
                          .arg(kp.getId())
                          .arg(kp.getTypeString())
                          .arg(kp.getDifficultyString())
                          .arg(kp.getEstimatedTime())
                          .arg(kp.getIsLearned() ? "✅ 已学习" : "⭕ 未学习")
                          .arg(kp.getMasteryLevel())
                          .arg(kp.getPrerequisiteIds().isEmpty() ? "无" : QString("%1 个").arg(kp.getPrerequisiteIds().size()))
                          .arg(kp.getChildIds().isEmpty() ? "无" : QString("%1 个").arg(kp.getChildIds().size()));

    ui->detailContent->setPlainText(details);
}

void MainWindow::on_refreshBtn_clicked()
{
    qDebug() << "刷新知识点";
    loadKnowledgeFromRepository();
    QMessageBox::information(this, "刷新完成", "知识库已刷新！");
}

void MainWindow::on_studyBtn_clicked()
{
    on_graphBtn_clicked();
}

void MainWindow::on_settingsBtn_clicked()
{
    QMenu menu(this);
    QAction* switchCareerAction = menu.addAction("切换学习方向");
    QAction* switchAccountAction = menu.addAction("切换账号");
    menu.addSeparator();
    QAction* exitAction = menu.addAction("退出");

    QAction* selectedAction = menu.exec(ui->settingsBtn->mapToGlobal(QPoint(0, ui->settingsBtn->height())));

    if (selectedAction == switchCareerAction) {
        switchCareerDirection();
    } else if (selectedAction == switchAccountAction) {
        switchAccount();
    } else if (selectedAction == exitAction) {
        exitApplication();
    }
}

void MainWindow::switchCareerDirection()
{
    CareerService* careerService = CareerService::getInstance();

    CareerSelectionDialog careerDialog(this);
    careerDialog.setAvailableCareers(careerService->getAllCareerPaths());

    CareerPath selectedCareer;
    TechStack selectedTechStack;
    bool hasSelection = false;

    connect(&careerDialog, &CareerSelectionDialog::careerSelectionCompleted,
            this, [&](const CareerPath& career, const TechStack& techStack) {
                selectedCareer = career;
                selectedTechStack = techStack;
                hasSelection = true;
            });

    if (careerDialog.exec() != QDialog::Accepted || !hasSelection) {
        return;
    }

    if (currentUser.getId() > 0) {
        if (!careerService->setUserCareerSelection(currentUser.getId(),
                                                   selectedCareer.getDirection(),
                                                   selectedTechStack.getId())) {
            QMessageBox::warning(this, "切换失败", "职业方向保存失败，请稍后重试。");
            return;
        }

        currentUser.setCareerDirection(selectedCareer.getDirection());
        currentUser.setTechStackId(selectedTechStack.getId());
    }

    ui->careerLabel->setText(QString("职业: %1").arg(selectedCareer.getName()));
    ui->currentCareerLabel->setText(selectedTechStack.getName().isEmpty()
                                        ? selectedCareer.getName()
                                        : QString("%1 · %2").arg(selectedCareer.getName(), selectedTechStack.getName()));

    loadKnowledgeFromRepository();

    QMessageBox::information(this, "切换成功",
                             selectedTechStack.getName().isEmpty()
                                 ? QString("当前学习方向已切换为：%1").arg(selectedCareer.getName())
                                 : QString("当前学习方向已切换为：%1\n技术栈：%2")
                                       .arg(selectedCareer.getName(), selectedTechStack.getName()));
}

void MainWindow::switchAccount()
{
    LoginService* loginService = LoginService::getInstance();
    CareerService* careerService = CareerService::getInstance();

    LoginDialog loginDialog(this);
    UserProfile newUser;
    bool loginSuccess = false;

    connect(&loginDialog, &LoginDialog::loginRequested,
            this, [&](const QString& username, const QString& password) {
                if (!loginService->login(username, password)) {
                    loginDialog.showErrorMessage("用户名或密码错误");
                    loginDialog.clearPassword();
                    return;
                }

                newUser = loginService->getCurrentUser();
                loginSuccess = true;
                loginDialog.accept();
            });

    connect(&loginDialog, &LoginDialog::guestLoginRequested,
            this, [&]() {
                newUser = loginService->loginAsGuest();
                loginSuccess = true;
                loginDialog.accept();
            });

    if (loginDialog.exec() != QDialog::Accepted || !loginSuccess) {
        return;
    }

    loginService->logout();

    if (newUser.getId() > 0 && careerService->shouldSelectCareer(newUser.getId())) {
        CareerSelectionDialog careerDialog(this);
        careerDialog.setAvailableCareers(careerService->getAllCareerPaths());

        CareerPath selectedCareer;
        TechStack selectedTechStack;
        bool hasSelection = false;

        connect(&careerDialog, &CareerSelectionDialog::careerSelectionCompleted,
                this, [&](const CareerPath& career, const TechStack& techStack) {
                    selectedCareer = career;
                    selectedTechStack = techStack;
                    hasSelection = true;
                });

        if (careerDialog.exec() == QDialog::Accepted && hasSelection) {
            careerService->setUserCareerSelection(newUser.getId(),
                                                  selectedCareer.getDirection(),
                                                  selectedTechStack.getId());
            newUser.setCareerDirection(selectedCareer.getDirection());
            newUser.setTechStackId(selectedTechStack.getId());
        }
    }

    MainWindow* newMainWindow = new MainWindow(newUser);
    newMainWindow->show();
    close();
}

void MainWindow::exitApplication()
{
    qApp->quit();
}

void MainWindow::updateStats()
{
    // 计算已学习的节点数
    int learnedCount = 0;
    int totalMastery = 0;
    int nodeCount = 0;

    const QList<KnowledgePoint> visibleKnowledge = filterKnowledgeForActiveTechStack(knowledgeRepo->findAll());
    for (const KnowledgePoint& kp : visibleKnowledge) {
        if (kp.getIsLearned()) {
            learnedCount++;
        }
        totalMastery += kp.getMasteryLevel();
        nodeCount++;
    }

    // 更新UI
    ui->learnedNodesValue->setText(QString::number(learnedCount));

    int avgMastery = nodeCount > 0 ? totalMastery / nodeCount : 0;
    ui->masteryValue->setText(QString("%1%").arg(avgMastery));

    // 更新进度条
    if (nodeCount > 0) {
        int progress = (learnedCount * 100) / nodeCount;
        ui->overallProgress->setValue(progress);
        ui->progressPercentLabel->setText(
            QString("%1% (已完成 %2/%3 个知识点)")
                .arg(progress)
                .arg(learnedCount)
                .arg(nodeCount)
            );
    }
}

void MainWindow::initTestData()
{
    qDebug() << "初始化测试数据...";

    if (!knowledgeRepo) return;

    // 创建一些测试知识点
    QList<KnowledgePoint> testNodes;

    // 前端基础
    KnowledgePoint html;
    html.setTitle("HTML5基础");
    html.setDescription("学习HTML5标签、语义化、表单等基础知识");
    //html.setType(KnowledgePoint::Type::THEORY);
    //html.setDifficulty(KnowledgePoint::Difficulty::EASY);
    html.setEstimatedTime(120);
    html.setMasteryLevel(0);
    html.setIsLearned(false);
    testNodes.append(html);

    KnowledgePoint css;
    css.setTitle("CSS3样式");
    css.setDescription("学习CSS选择器、盒模型、布局、动画等");
    //css.setType(KnowledgePoint::Type::THEORY);
    //css.setDifficulty(KnowledgePoint::Difficulty::EASY);
    css.setEstimatedTime(150);
    css.setMasteryLevel(0);
    css.setIsLearned(false);
    testNodes.append(css);

    KnowledgePoint js;
    js.setTitle("JavaScript核心");
    js.setDescription("学习JS语法、DOM操作、事件、异步编程");
    //js.setType(KnowledgePoint::Type::THEORY);
    //js.setDifficulty(KnowledgePoint::Difficulty::MEDIUM);
    js.setEstimatedTime(300);
    js.setMasteryLevel(0);
    js.setIsLearned(false);
    testNodes.append(js);

    // 保存到数据库
    for (KnowledgePoint& kp : testNodes) {
        knowledgeRepo->saveKnowledge(kp);
    }

    qDebug() << "测试数据初始化完成";
}
