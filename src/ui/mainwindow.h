#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlQuery>
#include <QSharedPointer>
#include <QTreeWidgetItem>
#include <QStackedWidget>
#include "src/core/repository/knowledgerepository.h"
#include "src/core/entity/userprofile.h"
#include "src/ui/knowledgegraph3dwidget.h"

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
    MainWindow(const UserProfile& user, QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_knowledgeTree_itemClicked(QTreeWidgetItem *item, int column);
    void on_refreshBtn_clicked();
    void on_studyBtn_clicked();
    void on_settingsBtn_clicked();
    void on_graphBtn_clicked();
    //void updateProgress();

private:
    void initUI();
    void loadKnowledgeFromRepository();
    QString resolveActiveTechStackId() const;
    QList<KnowledgePoint> filterKnowledgeForActiveTechStack(const QList<KnowledgePoint>& allKnowledge) const;
    void updatePersonaCard(const QList<KnowledgePoint>& visibleKnowledge);
    void updateRecommendationPanel(const QList<KnowledgePoint>& visibleKnowledge);
    bool isRecommendationPanelVisible() const;
    void setRecommendationPanelVisible(bool visible);
    void showKnowledgeDetails(const QString& knowledgeId);
    void updateStats();
    void switchCareerDirection();
    void switchAccount();
    void exitApplication();


    void initTestData();


private:

    Ui::MainWindow *ui;
    KnowledgeRepository* knowledgeRepo;
    UserProfile currentUser;
    QStackedWidget* detailStack = nullptr;
    KnowledgeGraph3DWidget* graphWidget = nullptr;
    QMap<QString, bool> learnedNodes;

};
#endif // MAINWINDOW_H
