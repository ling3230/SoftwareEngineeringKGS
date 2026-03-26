// ui/CareerSelectionDialog.h
#ifndef CAREERSELECTIONDIALOG_H
#define CAREERSELECTIONDIALOG_H

#include <QDialog>
#include <QList>
#include <QFrame>  // 添加头文件
#include "core/entity/careerpath.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class CareerSelectionDialog;
}
QT_END_NAMESPACE

class CareerSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CareerSelectionDialog(QWidget *parent = nullptr);
    ~CareerSelectionDialog();
    //设置可选择的职业
    void setAvailableCareers(const QList<CareerPath>& careers);
    CareerPath getSelectedCareer() const;
    TechStack getSelectedTechStack() const;
    QString getSelectedTechStackId() const;
    bool isSkipped() const { return skipped; }

signals:
    void careerSelected(const CareerPath& career);
    void careerSelectionCompleted(const CareerPath& career, const TechStack& techStack);
    void selectionSkipped();

private slots:
    void onCareerItemClicked(int index);
    void onTechStackItemClicked(int index);
    void onPreviousClicked();
    void onConfirmClicked();
    void onSkipClicked();
    void onSettingsButtonClicked();

private:
    Ui::CareerSelectionDialog *ui;
    QList<CareerPath> careers;
    QList<QFrame*> careerFrames;
    QList<QFrame*> techStackFrames;
    int selectedCareerIndex = -1;
    int selectedTechStackIndex = -1;
    bool skipped = false;

    QFrame* createCareerItem(const CareerPath& career, int index);
    QFrame* createTechStackItem(const TechStack& techStack, int index);
    QWidget* getCareerContainer() const;
    QWidget* getTechStackContainer() const;
    void clearCareerItems();
    void clearTechStackItems();
    void populateCareerStep();
    void populateTechStackStep();
    void showCareerStep();
    void showTechStackStep();
    void slideToPage(int pageIndex, bool slideLeft);
    void updateCareerSelection();
    void updateTechStackSelection();
    void clearCareerSelections();
    void clearTechStackSelections();
    bool isCareerStep() const;
    bool eventFilter(QObject* watched, QEvent* event) override;
    bool isAnimating = false;
};

#endif // CAREERSELECTIONDIALOG_H
