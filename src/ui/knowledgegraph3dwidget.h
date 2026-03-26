#ifndef KNOWLEDGEGRAPH3DWIDGET_H
#define KNOWLEDGEGRAPH3DWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QList>
#include <QHash>
#include <QVector3D>
#include <QPoint>

#include "src/core/entity/knowledgepoint.h"

class KnowledgeGraph3DWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit KnowledgeGraph3DWidget(QWidget* parent = nullptr);

    void setActiveTechStack(const QString& techStackId);//设置使用的技术栈
    void setKnowledgeData(const QList<KnowledgePoint>& knowledgeList);//设置知识点数据
    void setFocusKnowledge(const QString& knowledgeId);//
    void setFocusKnowledge(int knowledgeId);

protected:
    //openGL方法
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    struct NodeRenderData {//节点数据
        QString id;
        QString title;
        QString category;
        QList<QString> tags;
        QVector3D pos;
        int stage = 0;
        bool learned = false;
        int mastery = 0;
        bool expanded = true;
        bool trunk = false;
        bool aggregate = false;
        int aggregateCount = 0;
    };

    struct EdgeRenderData {//
        QString from;
        QString to;
        bool aggregate = false;
    };

    struct ProjectedNode {
        QString id;
        QString title;
        QPointF pos2d;
        float zDepth = 0.0f;
        float radius = 8.0f;
        int stage = 0;
        bool learned = false;
        int mastery = 0;
        bool focused = false;
        bool expanded = true;
        bool trunk = false;
        bool aggregate = false;
        int aggregateCount = 0;
    };

    QList<NodeRenderData> nodes;
    QList<EdgeRenderData> edges;
    QHash<QString, int> idToIndex;
    QList<KnowledgePoint> cachedKnowledge;

    float yaw = 0.0f;
    float pitch = 0.0f;
    float cameraDistance = 500.0f;
    QPoint lastMousePos;
    bool rotatingWithLeftButton = false;
    QString focusedKnowledgeId;
    QString activeTechStackId;

    void rebuildGraphLayout();
    QList<ProjectedNode> projectNodes() const;
    QPointF projectPoint(const QVector3D& point, float* outDepth) const;
};

#endif // KNOWLEDGEGRAPH3DWIDGET_H
