// ui/CareerSelectionDialog.cpp
#include "careerselectiondialog.h"
#include "ui_careerselectiondialog.h"

#include <QDebug>
#include <QMessageBox>
#include <QMouseEvent>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QLayoutItem>
#include <QStyle>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QEasingCurve>

namespace {
constexpr int kCareerPageIndex = 0;
constexpr int kTechStackPageIndex = 1;
constexpr int kGridColumns = 2;

QGridLayout* ensureGridLayout(QWidget* container)
{
    if (!container) {
        return nullptr;
    }

    if (!container->layout()) {
        auto* gridLayout = new QGridLayout(container);
        gridLayout->setContentsMargins(12, 12, 12, 12);
        gridLayout->setHorizontalSpacing(12);
        gridLayout->setVerticalSpacing(12);
    }

    return qobject_cast<QGridLayout*>(container->layout());
}
}

CareerSelectionDialog::CareerSelectionDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CareerSelectionDialog)
{
    ui->setupUi(this);

    setWindowTitle("选择职业发展方向");
    setMinimumSize(920, 680);
    resize(980, 720);

    const QString uiStyle = styleSheet();
    const QString dynamicStyle = R"(
        QFrame#careerItem {
            background-color: white;
            border: 2px solid #e0e0e0;
            border-radius: 10px;
            padding: 5px;
        }

        QFrame#careerItem:hover {
            border-color: #2196F3;
            background-color: #F5F5F5;
        }

        QFrame#careerItem[selected="true"] {
            border-color: #4CAF50;
            border-width: 3px;
            background-color: #E8F5E9;
        }

        QFrame#careerItem[selected="true"] QLabel#iconLabel {
            background-color: #4CAF50;
            color: white;
        }

        QLabel#nameLabel {
            font-size: 16px;
            font-weight: bold;
            color: #333;
        }

        QLabel#descLabel {
            font-size: 12px;
            color: #666;
            margin-top: 5px;
        }

        QLabel[text*="★"] {
            color: #FF9800;
            font-weight: bold;
            font-size: 12px;
            background-color: #FFF3E0;
            padding: 2px 8px;
            border-radius: 10px;
        }
    )";
    setStyleSheet(uiStyle + dynamicStyle);

    connect(ui->confirmButton, &QPushButton::clicked,
            this, &CareerSelectionDialog::onConfirmClicked);
    connect(ui->skipButton, &QPushButton::clicked,
            this, &CareerSelectionDialog::onSkipClicked);
        connect(ui->previousButton, &QPushButton::clicked,
            this, &CareerSelectionDialog::onPreviousClicked);
    connect(ui->settingsButton, &QPushButton::clicked,
            this, &CareerSelectionDialog::onSettingsButtonClicked);

    if (ui->contentStack) {
        ui->contentStack->setCurrentIndex(kCareerPageIndex);
    }
    showCareerStep();
}

CareerSelectionDialog::~CareerSelectionDialog()
{
    delete ui;
}

void CareerSelectionDialog::setAvailableCareers(const QList<CareerPath>& careerList)
{
    careers = careerList;
    selectedCareerIndex = -1;
    selectedTechStackIndex = -1;

    clearCareerItems();
    clearTechStackItems();
    populateCareerStep();
    if (ui->contentStack) {
        ui->contentStack->setCurrentIndex(kCareerPageIndex);
    }
    showCareerStep();
}

CareerPath CareerSelectionDialog::getSelectedCareer() const
{
    if (selectedCareerIndex >= 0 && selectedCareerIndex < careers.size()) {
        return careers[selectedCareerIndex];
    }

    return CareerPath();
}

TechStack CareerSelectionDialog::getSelectedTechStack() const
{
    const CareerPath selectedCareer = getSelectedCareer();
    const QList<TechStack> techStacks = selectedCareer.getTechStacks();
    if (selectedTechStackIndex >= 0 && selectedTechStackIndex < techStacks.size()) {
        return techStacks[selectedTechStackIndex];
    }

    return TechStack();
}

QString CareerSelectionDialog::getSelectedTechStackId() const
{
    return getSelectedTechStack().getId();
}

QFrame* CareerSelectionDialog::createCareerItem(const CareerPath& career, int index)
{
    QFrame* frame = new QFrame();
    frame->setObjectName("careerItem");
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setMinimumSize(360, 150);
    frame->setMaximumHeight(150);
    frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    frame->setCursor(Qt::PointingHandCursor);

    auto* layout = new QVBoxLayout(frame);
    layout->setSpacing(10);
    layout->setContentsMargins(15, 15, 15, 15);

    auto* topLayout = new QHBoxLayout();

    auto* iconLabel = new QLabel();
    iconLabel->setObjectName("iconLabel");
    iconLabel->setText(career.getIcon());
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFixedSize(50, 50);

    auto* textLayout = new QVBoxLayout();
    textLayout->setSpacing(5);

    auto* nameLayout = new QHBoxLayout();
    auto* nameLabel = new QLabel(career.getName());
    nameLabel->setObjectName("nameLabel");
    nameLayout->addWidget(nameLabel);

    if (career.isRecommended()) {
        auto* recommendLabel = new QLabel("★ 推荐");
        nameLayout->addWidget(recommendLabel);
    }
    nameLayout->addStretch();

    auto* descLabel = new QLabel(career.getDescription());
    descLabel->setObjectName("descLabel");
    descLabel->setWordWrap(true);
    descLabel->setMaximumHeight(48);

    textLayout->addLayout(nameLayout);
    textLayout->addWidget(descLabel);
    topLayout->addWidget(iconLabel);
    topLayout->addLayout(textLayout, 1);

    layout->addLayout(topLayout);
    layout->addStretch();

    frame->installEventFilter(this);
    frame->setProperty("careerIndex", index);
    frame->setProperty("itemType", "career");
    return frame;
}

QFrame* CareerSelectionDialog::createTechStackItem(const TechStack& techStack, int index)
{
    QFrame* frame = new QFrame();
    frame->setObjectName("careerItem");
    frame->setFrameShape(QFrame::StyledPanel);
    frame->setMinimumSize(360, 150);
    frame->setMaximumHeight(150);
    frame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    frame->setCursor(Qt::PointingHandCursor);

    auto* layout = new QVBoxLayout(frame);
    layout->setSpacing(10);
    layout->setContentsMargins(15, 15, 15, 15);

    auto* topLayout = new QHBoxLayout();

    auto* iconLabel = new QLabel();
    iconLabel->setObjectName("iconLabel");
    iconLabel->setText(techStack.getIcon().isEmpty() ? QString("🧩") : techStack.getIcon());
    iconLabel->setAlignment(Qt::AlignCenter);
    iconLabel->setFixedSize(50, 50);

    auto* textLayout = new QVBoxLayout();
    textLayout->setSpacing(5);

    auto* nameLayout = new QHBoxLayout();
    auto* nameLabel = new QLabel(techStack.getName());
    nameLabel->setObjectName("nameLabel");
    nameLayout->addWidget(nameLabel);

    if (techStack.isRecommended()) {
        auto* recommendLabel = new QLabel("★ 推荐");
        nameLayout->addWidget(recommendLabel);
    }
    nameLayout->addStretch();

    auto* descLabel = new QLabel(techStack.getDescription());
    descLabel->setObjectName("descLabel");
    descLabel->setWordWrap(true);
    descLabel->setMaximumHeight(48);

    textLayout->addLayout(nameLayout);
    textLayout->addWidget(descLabel);
    topLayout->addWidget(iconLabel);
    topLayout->addLayout(textLayout, 1);

    layout->addLayout(topLayout);
    layout->addStretch();

    frame->installEventFilter(this);
    frame->setProperty("techStackIndex", index);
    frame->setProperty("itemType", "techStack");
    return frame;
}

void CareerSelectionDialog::populateCareerStep()
{
    clearCareerItems();
    careerFrames.clear();

    QGridLayout* gridLayout = ensureGridLayout(getCareerContainer());
    if (!gridLayout) {
        qWarning() << "无法创建主职业布局";
        return;
    }

    int row = 0;
    int col = 0;
    for (int i = 0; i < careers.size(); ++i) {
        QFrame* itemFrame = createCareerItem(careers[i], i);
        careerFrames.append(itemFrame);
        gridLayout->addWidget(itemFrame, row, col);

        if (careers[i].isRecommended() && selectedCareerIndex == -1) {
            selectedCareerIndex = i;
        }

        ++col;
        if (col >= kGridColumns) {
            col = 0;
            ++row;
        }
    }

    if (selectedCareerIndex != -1) {
        updateCareerSelection();
    }

    gridLayout->setRowStretch(row + 1, 1);
}

void CareerSelectionDialog::populateTechStackStep()
{
    clearTechStackItems();
    techStackFrames.clear();
    selectedTechStackIndex = -1;

    QGridLayout* gridLayout = ensureGridLayout(getTechStackContainer());
    if (!gridLayout) {
        qWarning() << "无法创建技术栈布局";
        return;
    }

    const QList<TechStack> techStacks = getSelectedCareer().getTechStacks();
    int row = 0;
    int col = 0;
    for (int i = 0; i < techStacks.size(); ++i) {
        QFrame* itemFrame = createTechStackItem(techStacks[i], i);
        techStackFrames.append(itemFrame);
        gridLayout->addWidget(itemFrame, row, col);

        if (techStacks[i].isRecommended() && selectedTechStackIndex == -1) {
            selectedTechStackIndex = i;
        }

        ++col;
        if (col >= kGridColumns) {
            col = 0;
            ++row;
        }
    }

    if (selectedTechStackIndex != -1) {
        updateTechStackSelection();
    }

    gridLayout->setRowStretch(row + 1, 1);
}

void CareerSelectionDialog::showCareerStep()
{
    ui->titleLabel->setText("第一步：选择主职业方向");
    ui->descriptionLabel->setText("先选择您的主职业方向，下一步再从该方向中选择具体技术栈。\n双击卡片可直接进入下一步。");
    ui->confirmButton->setText("下一步");
    ui->confirmButton->setEnabled(selectedCareerIndex >= 0);
    ui->previousButton->setEnabled(false);
    ui->settingsButton->setText("以后再说");
}

void CareerSelectionDialog::showTechStackStep()
{
    const CareerPath selectedCareer = getSelectedCareer();
    ui->titleLabel->setText(QString("第二步：选择 %1 技术栈").arg(selectedCareer.getName()));
    ui->descriptionLabel->setText(QString("已选择主职业：%1。请继续选择一个最符合目标的技术栈。\n双击卡片可直接完成选择。").arg(selectedCareer.getName()));
    ui->confirmButton->setText("确认选择");
    ui->confirmButton->setEnabled(selectedTechStackIndex >= 0);
    ui->previousButton->setEnabled(true);
    ui->settingsButton->setText("以后再说");
}

void CareerSelectionDialog::slideToPage(int pageIndex, bool slideLeft)
{
    if (!ui->contentStack) {
        return;
    }

    if (isAnimating || ui->contentStack->currentIndex() == pageIndex) {
        ui->contentStack->setCurrentIndex(pageIndex);
        return;
    }

    QWidget* currentPage = ui->contentStack->currentWidget();
    QWidget* nextPage = ui->contentStack->widget(pageIndex);
    if (!currentPage || !nextPage) {
        ui->contentStack->setCurrentIndex(pageIndex);
        return;
    }

    const QRect stackRect = ui->contentStack->rect();
    const QPoint origin = stackRect.topLeft();
    const int offset = stackRect.width();
    const QPoint currentEnd = origin + QPoint(slideLeft ? -offset : offset, 0);
    const QPoint nextStart = origin + QPoint(slideLeft ? offset : -offset, 0);

    nextPage->setGeometry(stackRect);
    nextPage->move(nextStart);
    nextPage->show();
    nextPage->raise();

    currentPage->setGeometry(stackRect);
    currentPage->move(origin);
    currentPage->show();

    auto* currentAnimation = new QPropertyAnimation(currentPage, "pos");
    currentAnimation->setDuration(260);
    currentAnimation->setStartValue(origin);
    currentAnimation->setEndValue(currentEnd);
    currentAnimation->setEasingCurve(QEasingCurve::InOutCubic);

    auto* nextAnimation = new QPropertyAnimation(nextPage, "pos");
    nextAnimation->setDuration(260);
    nextAnimation->setStartValue(nextStart);
    nextAnimation->setEndValue(origin);
    nextAnimation->setEasingCurve(QEasingCurve::InOutCubic);

    auto* animationGroup = new QParallelAnimationGroup(this);
    animationGroup->addAnimation(currentAnimation);
    animationGroup->addAnimation(nextAnimation);

    isAnimating = true;
    connect(animationGroup, &QParallelAnimationGroup::finished, this,
            [this, animationGroup, pageIndex, currentPage, nextPage, origin]() {
                ui->contentStack->setCurrentIndex(pageIndex);
                currentPage->move(origin);
                nextPage->move(origin);
                isAnimating = false;
                animationGroup->deleteLater();
            });

    animationGroup->start();
}

void CareerSelectionDialog::updateCareerSelection()
{
    clearCareerSelections();
    if (selectedCareerIndex >= 0 && selectedCareerIndex < careerFrames.size()) {
        QFrame* selectedFrame = careerFrames[selectedCareerIndex];
        selectedFrame->setProperty("selected", true);
        selectedFrame->style()->unpolish(selectedFrame);
        selectedFrame->style()->polish(selectedFrame);
    }
}

void CareerSelectionDialog::updateTechStackSelection()
{
    clearTechStackSelections();
    if (selectedTechStackIndex >= 0 && selectedTechStackIndex < techStackFrames.size()) {
        QFrame* selectedFrame = techStackFrames[selectedTechStackIndex];
        selectedFrame->setProperty("selected", true);
        selectedFrame->style()->unpolish(selectedFrame);
        selectedFrame->style()->polish(selectedFrame);
    }
}

void CareerSelectionDialog::clearCareerSelections()
{
    for (QFrame* frame : careerFrames) {
        frame->setProperty("selected", false);
        frame->style()->unpolish(frame);
        frame->style()->polish(frame);
    }
}

void CareerSelectionDialog::clearTechStackSelections()
{
    for (QFrame* frame : techStackFrames) {
        frame->setProperty("selected", false);
        frame->style()->unpolish(frame);
        frame->style()->polish(frame);
    }
}

bool CareerSelectionDialog::isCareerStep() const
{
    return !ui->contentStack || ui->contentStack->currentIndex() == kCareerPageIndex;
}

bool CareerSelectionDialog::eventFilter(QObject* watched, QEvent* event)
{
    QFrame* frame = qobject_cast<QFrame*>(watched);
    if (!frame) {
        return QDialog::eventFilter(watched, event);
    }

    const QString itemType = frame->property("itemType").toString();
    if (event->type() == QEvent::MouseButtonPress) {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            if (itemType == "career") {
                onCareerItemClicked(frame->property("careerIndex").toInt());
                return true;
            }
            if (itemType == "techStack") {
                onTechStackItemClicked(frame->property("techStackIndex").toInt());
                return true;
            }
        }
    } else if (event->type() == QEvent::MouseButtonDblClick) {
        auto* mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            if (itemType == "career") {
                onCareerItemClicked(frame->property("careerIndex").toInt());
                onConfirmClicked();
                return true;
            }
            if (itemType == "techStack") {
                onTechStackItemClicked(frame->property("techStackIndex").toInt());
                onConfirmClicked();
                return true;
            }
        }
    }

    return QDialog::eventFilter(watched, event);
}

void CareerSelectionDialog::onCareerItemClicked(int index)
{
    if (isAnimating || index < 0 || index >= careers.size()) {
        return;
    }

    selectedCareerIndex = index;
    selectedTechStackIndex = -1;
    updateCareerSelection();
    clearTechStackSelections();

    if (isCareerStep()) {
        const CareerPath selectedCareer = getSelectedCareer();
        ui->confirmButton->setEnabled(true);

        if (selectedCareer.hasTechStacks()) {
            populateTechStackStep();
            showTechStackStep();
            slideToPage(kTechStackPageIndex, true);
        }
    }
}

void CareerSelectionDialog::onTechStackItemClicked(int index)
{
    if (isAnimating) {
        return;
    }

    const QList<TechStack> techStacks = getSelectedCareer().getTechStacks();
    if (index < 0 || index >= techStacks.size()) {
        return;
    }

    selectedTechStackIndex = index;
    updateTechStackSelection();
    ui->confirmButton->setEnabled(true);
}

void CareerSelectionDialog::onPreviousClicked()
{
    if (isAnimating || isCareerStep()) {
        return;
    }

    selectedTechStackIndex = -1;
    clearTechStackSelections();
    showCareerStep();
    slideToPage(kCareerPageIndex, false);
}

QWidget* CareerSelectionDialog::getCareerContainer() const
{
    return ui->scrollAreaWidgetContents;
}

QWidget* CareerSelectionDialog::getTechStackContainer() const
{
    if (ui->scrollAreaWidgetContents_2) {
        return ui->scrollAreaWidgetContents_2;
    }
    return ui->scrollAreaWidgetContents_3;
}

void CareerSelectionDialog::clearCareerItems()
{
    QWidget* container = getCareerContainer();
    if (!container || !container->layout()) {
        return;
    }

    QLayoutItem* child = nullptr;
    while ((child = container->layout()->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }
    careerFrames.clear();
}

void CareerSelectionDialog::clearTechStackItems()
{
    QWidget* container = getTechStackContainer();
    if (!container || !container->layout()) {
        return;
    }

    QLayoutItem* child = nullptr;
    while ((child = container->layout()->takeAt(0)) != nullptr) {
        if (child->widget()) {
            delete child->widget();
        }
        delete child;
    }
    techStackFrames.clear();
}

void CareerSelectionDialog::onConfirmClicked()
{
    if (isAnimating) {
        return;
    }

    if (isCareerStep()) {
        const CareerPath selectedCareer = getSelectedCareer();
        if (selectedCareer.getName().isEmpty()) {
            return;
        }

        if (!selectedCareer.hasTechStacks()) {
            emit careerSelected(selectedCareer);
            emit careerSelectionCompleted(selectedCareer, TechStack());
            accept();
            return;
        }

        populateTechStackStep();
        showTechStackStep();
        slideToPage(kTechStackPageIndex, true);
        return;
    }

    const CareerPath selectedCareer = getSelectedCareer();
    const TechStack selectedTechStack = getSelectedTechStack();
    if (selectedCareer.getName().isEmpty() || selectedTechStack.getId().isEmpty()) {
        return;
    }

    emit careerSelected(selectedCareer);
    emit careerSelectionCompleted(selectedCareer, selectedTechStack);
    accept();
}

void CareerSelectionDialog::onSkipClicked()
{
    const int result = QMessageBox::question(
        this,
        "跳过选择",
        "跳过选择将使用默认的软件工程必修课程。\n您以后可以在设置中更改职业方向和技术栈。\n\n确定跳过吗？",
        QMessageBox::Yes | QMessageBox::No);

    if (result == QMessageBox::Yes) {
        skipped = true;
        emit selectionSkipped();
        accept();
    }
}

void CareerSelectionDialog::onSettingsButtonClicked()
{
    skipped = true;
    emit selectionSkipped();
    accept();
}
