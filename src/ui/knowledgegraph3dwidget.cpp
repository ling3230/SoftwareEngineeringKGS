#include "knowledgegraph3dwidget.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <QSet>
#include <QHash>
#include <QMap>
#include <QQueue>
#include <cmath>
#include <algorithm>

namespace {

const QString kAggregateNodeId = "__collapsed_other_tracks__";

bool containsTagIgnoreCase(const QList<QString>& tags, const QString& expected)
{
    for (const QString& tag : tags) {
        if (tag.compare(expected, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    return false;
}

bool isExpandedForTechStack(const KnowledgePoint& kp, const QString& activeTechStackId)
{
    if (activeTechStackId.trimmed().isEmpty()) {
        return true;
    }

    if (kp.getCategory().compare(activeTechStackId, Qt::CaseInsensitive) == 0) {
        return true;
    }

    if (containsTagIgnoreCase(kp.getTags(), activeTechStackId)) {
        return true;
    }

    if (containsTagIgnoreCase(kp.getTags(), "common")) {
        return true;
    }

    return false;
}

bool shouldStayOnTrunk(const KnowledgePoint& kp, const QSet<QString>& expandedIds)
{
    if (containsTagIgnoreCase(kp.getTags(), "foundation")
        || containsTagIgnoreCase(kp.getTags(), "common")) {
        return true;
    }

    for (const QString& preId : kp.getPrerequisiteIds()) {
        if (expandedIds.contains(preId)) {
            return false;
        }
    }

    return true;
}

QList<QString> selectRenderablePrerequisites(const KnowledgePoint& kp,
                                             const QHash<QString, bool>& trunkById,
                                             const QSet<QString>& expandedIds)
{
    QList<QString> selected;
    const QList<QString> prerequisites = kp.getPrerequisiteIds();
    if (prerequisites.isEmpty()) {
        return selected;
    }

    const QString primaryParentId = kp.getPrimaryParentId();
    if (!primaryParentId.isEmpty() && expandedIds.contains(primaryParentId)) {
        selected.append(primaryParentId);
    }

    for (const QString& preId : prerequisites) {
        if (!expandedIds.contains(preId) || selected.contains(preId)) {
            continue;
        }

        // 主干依赖优先保留，避免关键结构被削弱。
        if (trunkById.value(preId, false) || trunkById.value(kp.getId(), false)) {
            selected.append(preId);
            break;
        }
    }

    if (selected.isEmpty()) {
        for (const QString& preId : prerequisites) {
            if (expandedIds.contains(preId)) {
                selected.append(preId);
                break;
            }
        }
    }

    return selected;
}

void drawArrowLine(QPainter& painter,
                   const QPointF& from,
                   const QPointF& to,
                   const QPen& pen,
                   qreal arrowSize)
{
    painter.setPen(pen);
    painter.drawLine(from, to);

    const QLineF line(from, to);
    if (line.length() < 1.0) {
        return;
    }

    const qreal pi = 3.14159265358979323846;
    const qreal angle = std::atan2(-line.dy(), line.dx());
    const QPointF arrowP1 = to + QPointF(std::sin(angle - pi / 3.0) * arrowSize,
                                         std::cos(angle - pi / 3.0) * arrowSize);
    const QPointF arrowP2 = to + QPointF(std::sin(angle - pi + pi / 3.0) * arrowSize,
                                         std::cos(angle - pi + pi / 3.0) * arrowSize);

    QPainterPath path;
    path.moveTo(to);
    path.lineTo(arrowP1);
    path.lineTo(arrowP2);
    path.closeSubpath();
    painter.fillPath(path, pen.color());
}

}

KnowledgeGraph3DWidget::KnowledgeGraph3DWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
    setMouseTracking(true);
    yaw = -18.0f;
    pitch = -8.0f;
}

void KnowledgeGraph3DWidget::setActiveTechStack(const QString& techStackId)
{
    activeTechStackId = techStackId.trimmed();
    if (!cachedKnowledge.isEmpty()) {
        setKnowledgeData(cachedKnowledge);
    }
}

void KnowledgeGraph3DWidget::setKnowledgeData(const QList<KnowledgePoint>& knowledgeList)
{
    cachedKnowledge = knowledgeList;
    nodes.clear();
    edges.clear();
    idToIndex.clear();

    QList<KnowledgePoint> expandedKnowledge;
    int collapsedCount = 0;

    for (const KnowledgePoint& kp : knowledgeList) {
        if (isExpandedForTechStack(kp, activeTechStackId)) {
            expandedKnowledge.append(kp);
        } else {
            ++collapsedCount;
        }
    }

    QSet<QString> expandedIds;
    QHash<QString, bool> trunkById;
    for (const KnowledgePoint& kp : expandedKnowledge) {
        expandedIds.insert(kp.getId());
    }

    for (const KnowledgePoint& kp : expandedKnowledge) {
        NodeRenderData node;
        node.id = kp.getId();
        node.title = kp.getTitle();
        node.category = kp.getCategory();
        node.tags = kp.getTags();
        node.learned = kp.getIsLearned();
        node.mastery = kp.getMasteryLevel();
        node.expanded = true;
        node.trunk = shouldStayOnTrunk(kp, expandedIds);
        trunkById.insert(node.id, node.trunk);

        idToIndex.insert(node.id, nodes.size());
        nodes.append(node);
    }

    if (collapsedCount > 0) {
        NodeRenderData aggregateNode;
        aggregateNode.id = kAggregateNodeId;
        aggregateNode.title = QString("其他方向 (%1)").arg(collapsedCount);
        aggregateNode.aggregate = true;
        aggregateNode.expanded = false;
        aggregateNode.aggregateCount = collapsedCount;
        idToIndex.insert(aggregateNode.id, nodes.size());
        nodes.append(aggregateNode);
    }

    QSet<QString> edgeKeys;
    for (const KnowledgePoint& kp : expandedKnowledge) {
        const QString toId = kp.getId();
        const QList<QString> prerequisites = selectRenderablePrerequisites(kp, trunkById, expandedIds);
        for (const QString& preId : prerequisites) {
            if (!idToIndex.contains(preId)) {
                continue;
            }

            const QString key = preId + "->" + toId;
            if (!edgeKeys.contains(key)) {
                edges.append({preId, toId, false});
                edgeKeys.insert(key);
            }
        }
    }

    if (collapsedCount > 0 && !expandedKnowledge.isEmpty()) {
        QString anchorId;
        for (const KnowledgePoint& kp : expandedKnowledge) {
            if (shouldStayOnTrunk(kp, expandedIds)) {
                anchorId = kp.getId();
                break;
            }
        }

        if (anchorId.isEmpty()) {
            anchorId = expandedKnowledge.first().getId();
        }

        edges.append({anchorId, kAggregateNodeId, true});
    }

    rebuildGraphLayout();
    update();
}

void KnowledgeGraph3DWidget::setFocusKnowledge(const QString& knowledgeId)
{
    focusedKnowledgeId = knowledgeId;
    update();
}

void KnowledgeGraph3DWidget::setFocusKnowledge(int knowledgeId)
{
    setFocusKnowledge(QString::number(knowledgeId));
}

void KnowledgeGraph3DWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.06f, 0.09f, 0.14f, 1.0f);
}

void KnowledgeGraph3DWidget::resizeGL(int w, int h)
{
    Q_UNUSED(w);
    Q_UNUSED(h);
}

void KnowledgeGraph3DWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QList<ProjectedNode> projected = projectNodes();

    QHash<QString, ProjectedNode> nodeMap;
    nodeMap.reserve(projected.size());
    for (const ProjectedNode& node : projected) {
        nodeMap.insert(node.id, node);
    }

    QHash<QString, int> outDegree;
    QHash<QString, int> inDegree;
    for (const ProjectedNode& node : projected) {
        if (node.aggregate) {
            continue;
        }
        outDegree.insert(node.id, 0);
        inDegree.insert(node.id, 0);
    }

    for (const EdgeRenderData& edge : edges) {
        if (edge.aggregate) {
            continue;
        }
        if (outDegree.contains(edge.from)) {
            outDegree[edge.from] = outDegree.value(edge.from) + 1;
        }
        if (inDegree.contains(edge.to)) {
            inDegree[edge.to] = inDegree.value(edge.to) + 1;
        }
    }

    QSet<QString> finalGoalIds;
    for (auto it = outDegree.constBegin(); it != outDegree.constEnd(); ++it) {
        if (it.value() == 0 && inDegree.value(it.key(), 0) > 0) {
            finalGoalIds.insert(it.key());
        }
    }

    QSet<QString> trunkVisibleNodeIds;
    QSet<QString> trunkEdgeKeys;
    QSet<QString> defaultVisibleEdgeKeys;
    QHash<QString, int> bestIncomingScore;
    QHash<QString, QString> bestIncomingEdgeKey;
    for (const ProjectedNode& node : projected) {
        if (!node.aggregate && node.trunk) {
            trunkVisibleNodeIds.insert(node.id);
        }
    }

    for (const EdgeRenderData& edge : edges) {
        if (edge.aggregate || !nodeMap.contains(edge.from) || !nodeMap.contains(edge.to)) {
            continue;
        }

        const ProjectedNode& fromNode = nodeMap[edge.from];
        const ProjectedNode& toNode = nodeMap[edge.to];
        if (fromNode.trunk && toNode.trunk) {
            const QString key = edge.from + "->" + edge.to;
            trunkEdgeKeys.insert(key);
            defaultVisibleEdgeKeys.insert(key);
            trunkVisibleNodeIds.insert(edge.from);
            trunkVisibleNodeIds.insert(edge.to);
        }

        if (finalGoalIds.contains(edge.to)) {
            defaultVisibleEdgeKeys.insert(edge.from + "->" + edge.to);
        }

        // 兜底：为每个非起点节点选一条最有代表性的入边，避免出现“没人指向”。
        int score = 0;
        if (fromNode.trunk) {
            score += 5;
        }
        if (fromNode.stage == toNode.stage - 1) {
            score += 4;
        } else if (fromNode.stage < toNode.stage) {
            score += 2;
        }
        if (fromNode.trunk && toNode.trunk) {
            score += 2;
        }

        const QString toId = edge.to;
        if (!bestIncomingScore.contains(toId) || score > bestIncomingScore.value(toId)) {
            bestIncomingScore[toId] = score;
            bestIncomingEdgeKey[toId] = edge.from + "->" + edge.to;
        }
    }

    for (auto it = bestIncomingEdgeKey.constBegin(); it != bestIncomingEdgeKey.constEnd(); ++it) {
        defaultVisibleEdgeKeys.insert(it.value());
    }

    QSet<QString> defaultVisibleNodeIds = trunkVisibleNodeIds;
    for (const QString& key : defaultVisibleEdgeKeys) {
        const QStringList parts = key.split("->");
        if (parts.size() == 2) {
            defaultVisibleNodeIds.insert(parts[0]);
            defaultVisibleNodeIds.insert(parts[1]);
        }
    }

    for (const QString& goalId : finalGoalIds) {
        defaultVisibleNodeIds.insert(goalId);
    }

    const bool hasFocus = !focusedKnowledgeId.trimmed().isEmpty();
    QSet<QString> focusNeighborhood;
    if (hasFocus) {
        focusNeighborhood.insert(focusedKnowledgeId);
        for (const EdgeRenderData& edge : edges) {
            if (edge.from == focusedKnowledgeId) {
                focusNeighborhood.insert(edge.to);
            }
            if (edge.to == focusedKnowledgeId) {
                focusNeighborhood.insert(edge.from);
            }
        }
    }

    for (const EdgeRenderData& edge : edges) {
        if (!nodeMap.contains(edge.from) || !nodeMap.contains(edge.to)) {
            continue;
        }

        const ProjectedNode& fromNode = nodeMap[edge.from];
        const ProjectedNode& toNode = nodeMap[edge.to];

        if (!edge.aggregate) {
            const QString edgeKey = edge.from + "->" + edge.to;
            const bool isTrunkEdge = trunkEdgeKeys.contains(edgeKey);
            const bool isBaselineEdge = defaultVisibleEdgeKeys.contains(edgeKey);
            if (hasFocus) {
                const bool isDirectRelation = (edge.from == focusedKnowledgeId || edge.to == focusedKnowledgeId);
                // 点击时保留基础关系，再额外展示当前节点全部直接关联。
                if (!isDirectRelation && !isTrunkEdge && !isBaselineEdge) {
                    continue;
                }
            } else {
                // 未点击时：保留主线连接 + 最终目标前置指向关系。
                if (!defaultVisibleEdgeKeys.contains(edgeKey)) {
                    continue;
                }
            }
        }

        if (edge.aggregate && hasFocus) {
            if (!focusNeighborhood.contains(edge.from) && !focusNeighborhood.contains(edge.to)) {
                continue;
            }
        }

        const float avgDepth = (fromNode.zDepth + toNode.zDepth) * 0.5f;
        int alpha = edge.aggregate ? 70 : qBound(35, static_cast<int>(180 - avgDepth * 0.22f), 145);
        if (hasFocus && !focusNeighborhood.contains(edge.from) && !focusNeighborhood.contains(edge.to)) {
            alpha = qBound(15, alpha / 3, 40);
        } else if (!hasFocus && !defaultVisibleNodeIds.contains(edge.from) && !defaultVisibleNodeIds.contains(edge.to)) {
            alpha = qBound(15, alpha / 3, 40);
        }
        QPen edgePen(edge.aggregate ? QColor(170, 170, 170, alpha) : QColor(120, 180, 240, alpha));
        edgePen.setWidthF(edge.aggregate ? 0.9 : 1.2);

        const QPointF dir = toNode.pos2d - fromNode.pos2d;
        const qreal len = std::hypot(dir.x(), dir.y());
        if (len < 1.0) {
            continue;
        }

        const QPointF unit(dir.x() / len, dir.y() / len);
        const QPointF start = fromNode.pos2d + unit * (fromNode.radius + 1.0);
        const QPointF end = toNode.pos2d - unit * (toNode.radius + 2.0);
        drawArrowLine(painter, start, end, edgePen, edge.aggregate ? 4.0 : 5.0);
    }

    std::sort(projected.begin(), projected.end(), [](const ProjectedNode& a, const ProjectedNode& b) {
        return a.zDepth > b.zDepth;
    });

    for (const ProjectedNode& node : projected) {
        QColor nodeColor = node.aggregate
            ? QColor(150, 150, 150)
            : (node.learned ? QColor(46, 204, 113) : QColor(52, 152, 219));

        if (!node.aggregate && node.mastery > 0 && node.mastery < 80) {
            nodeColor = QColor(52, 152, 219);
        }
        if (!node.aggregate && node.mastery >= 80) {
            nodeColor = QColor(39, 174, 96);
        }
        if (node.trunk) {
            nodeColor = QColor(231, 111, 81);
        }
        if (node.focused) {
            nodeColor = QColor(241, 196, 15);
        }

        if (hasFocus
            && !node.focused
            && !node.aggregate
            && !node.trunk
            && !focusNeighborhood.contains(node.id)) {
            nodeColor.setAlpha(85);
        } else if (!hasFocus
                   && !node.aggregate
                   && !node.trunk
                   && !defaultVisibleNodeIds.contains(node.id)) {
            nodeColor.setAlpha(75);
        }

        painter.setPen(Qt::NoPen);
        painter.setBrush(nodeColor);
        const float drawRadius = (hasFocus
                                  && !node.focused
                                  && !node.trunk
                                  && !focusNeighborhood.contains(node.id))
            ? node.radius * 0.82f
            : ((!hasFocus
                && !node.aggregate
                && !node.trunk
                && !defaultVisibleNodeIds.contains(node.id))
                   ? node.radius * 0.8f
                   : node.radius);
        painter.drawEllipse(node.pos2d, drawRadius, drawRadius);

        if (node.focused || node.aggregate || node.trunk) {
            QPen focusPen(QColor(255, 255, 255, 210));
            focusPen.setWidthF(1.5f);
            painter.setPen(focusPen);
            painter.setBrush(Qt::NoBrush);
            painter.drawEllipse(node.pos2d, node.radius + 4.0f, node.radius + 4.0f);

            painter.setPen(QColor(235, 245, 255));
            const QString stageLabel = node.aggregate
                ? node.title
                : QString("[阶段%1] %2").arg(node.stage + 1).arg(node.title);
            painter.drawText(node.pos2d + QPointF(10.0f, -8.0f), stageLabel);
        }
    }

    painter.setPen(QColor(180, 200, 220));
    painter.drawText(QRectF(14, 12, width() - 28, 40),
                     Qt::AlignLeft | Qt::AlignTop,
                     activeTechStackId.isEmpty()
                         ? "默认显示主线连接 + 最终目标前置指向 | 点击知识点显示该点全部直接关联"
                         : QString("当前技术栈: %1 | 默认主线+最终目标前置 | 点击节点查看该点全部直接关联")
                               .arg(activeTechStackId));
}

void KnowledgeGraph3DWidget::mousePressEvent(QMouseEvent* event)
{
    lastMousePos = event->pos();

    if (event->button() == Qt::LeftButton) {
        rotatingWithLeftButton = true;
        setCursor(Qt::ClosedHandCursor);
        event->accept();
        return;
    }

    QOpenGLWidget::mousePressEvent(event);
}

void KnowledgeGraph3DWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        rotatingWithLeftButton = false;
        unsetCursor();
        event->accept();
        return;
    }

    QOpenGLWidget::mouseReleaseEvent(event);
}

void KnowledgeGraph3DWidget::mouseMoveEvent(QMouseEvent* event)
{
    QPoint delta = event->pos() - lastMousePos;
    lastMousePos = event->pos();

    if (!rotatingWithLeftButton || !(event->buttons() & Qt::LeftButton)) {
        QOpenGLWidget::mouseMoveEvent(event);
        return;
    }

    yaw += delta.x() * 0.4f;
    pitch += delta.y() * 0.3f;
    pitch = qBound(-85.0f, pitch, 85.0f);

    update();
    event->accept();
}

void KnowledgeGraph3DWidget::wheelEvent(QWheelEvent* event)
{
    const QPoint degrees = event->angleDelta() / 8;
    if (!degrees.isNull()) {
        cameraDistance -= degrees.y() * 0.7f;
        cameraDistance = qBound(220.0f, cameraDistance, 900.0f);
        update();
    }

    QOpenGLWidget::wheelEvent(event);
}

void KnowledgeGraph3DWidget::rebuildGraphLayout()
{
    const int count = nodes.size();
    if (count == 0) {
        return;
    }

    QHash<QString, int> levels;
    QHash<QString, int> indegree;
    QHash<QString, QList<QString>> adjacency;
    QList<QString> expandedIds;
    QString aggregateId;

    for (const NodeRenderData& node : nodes) {
        if (node.aggregate) {
            aggregateId = node.id;
            continue;
        }

        expandedIds.append(node.id);
        indegree.insert(node.id, 0);
    }

    for (const EdgeRenderData& edge : edges) {
        if (edge.aggregate || !indegree.contains(edge.from) || !indegree.contains(edge.to)) {
            continue;
        }

        adjacency[edge.from].append(edge.to);
        indegree[edge.to] = indegree.value(edge.to) + 1;
    }

    QQueue<QString> queue;
    for (auto it = indegree.begin(); it != indegree.end(); ++it) {
        if (it.value() == 0) {
            queue.enqueue(it.key());
            levels.insert(it.key(), 0);
        }
    }

    while (!queue.isEmpty()) {
        const QString current = queue.dequeue();
        const int currentLevel = levels.value(current, 0);

        for (const QString& next : adjacency.value(current)) {
            levels[next] = qMax(levels.value(next, 0), currentLevel + 1);
            indegree[next] = indegree.value(next) - 1;
            if (indegree.value(next) == 0) {
                queue.enqueue(next);
            }
        }
    }

    for (const QString& nodeId : expandedIds) {
        if (!levels.contains(nodeId)) {
            levels.insert(nodeId, 0);
        }
    }

    int minLevel = 0;
    int maxLevel = 0;
    bool firstLevel = true;
    QMap<int, QList<QString>> idsByLevel;

    for (const QString& nodeId : expandedIds) {
        const int lv = levels.value(nodeId, 0);
        if (firstLevel) {
            minLevel = lv;
            maxLevel = lv;
            firstLevel = false;
        } else {
            minLevel = qMin(minLevel, lv);
            maxLevel = qMax(maxLevel, lv);
        }

        idsByLevel[lv].append(nodeId);
    }

    const int levelCount = qMax(1, maxLevel - minLevel + 1);
    const float trunkGapY = 84.0f;
    const float trunkLineX = 0.0f;
    const float trunkLineZ = 0.0f;
    const float branchOffsetX = 84.0f;
    const float branchGapY = 28.0f;
    const float branchGapZ = 22.0f;

    QHash<QString, QVector3D> positions;
    QList<QString> trunkOrder;
    QHash<QString, int> trunkIndexById;

    for (int lv = minLevel; lv <= maxLevel; ++lv) {
        QList<QString> ids = idsByLevel.value(lv);
        std::sort(ids.begin(), ids.end(), [&](const QString& left, const QString& right) {
            const bool lTrunk = idToIndex.contains(left) && nodes[idToIndex.value(left)].trunk;
            const bool rTrunk = idToIndex.contains(right) && nodes[idToIndex.value(right)].trunk;
            if (lTrunk != rTrunk) {
                return lTrunk;
            }
            return left < right;
        });

        for (const QString& id : ids) {
            const bool isTrunk = idToIndex.contains(id) && nodes[idToIndex.value(id)].trunk;
            if (isTrunk) {
                trunkIndexById.insert(id, trunkOrder.size());
                trunkOrder.append(id);
            }
        }
    }

    const float trunkStartY = -0.5f * trunkGapY * qMax(0, trunkOrder.size() - 1);
    for (int i = 0; i < trunkOrder.size(); ++i) {
        const QString& trunkId = trunkOrder[i];
        const float x = trunkLineX;
        const float y = trunkStartY + i * trunkGapY;
        const float z = trunkLineZ;
        positions.insert(trunkId, QVector3D(x, y, z));
    }

    QHash<QString, QList<QString>> branchesByAnchor;
    for (const QString& nodeId : expandedIds) {
        if (trunkIndexById.contains(nodeId)) {
            continue;
        }

        QString anchorId;
        int anchorRank = std::numeric_limits<int>::max();
        for (const EdgeRenderData& edge : edges) {
            if (edge.aggregate || edge.to != nodeId) {
                continue;
            }
            if (!trunkIndexById.contains(edge.from)) {
                continue;
            }

            const int rank = trunkIndexById.value(edge.from);
            if (rank < anchorRank) {
                anchorRank = rank;
                anchorId = edge.from;
            }
        }

        if (anchorId.isEmpty()) {
            const int lv = levels.value(nodeId, minLevel);
            for (int candidateLevel = lv; candidateLevel >= minLevel && anchorId.isEmpty(); --candidateLevel) {
                QList<QString> ids = idsByLevel.value(candidateLevel);
                for (const QString& id : ids) {
                    if (trunkIndexById.contains(id)) {
                        anchorId = id;
                        break;
                    }
                }
            }
        }

        if (anchorId.isEmpty() && !trunkOrder.isEmpty()) {
            anchorId = trunkOrder.first();
        }
        if (!anchorId.isEmpty()) {
            branchesByAnchor[anchorId].append(nodeId);
        }
    }

    for (const QString& anchorId : trunkOrder) {
        QList<QString> branchIds = branchesByAnchor.value(anchorId);
        if (branchIds.isEmpty()) {
            continue;
        }

        std::sort(branchIds.begin(), branchIds.end());
        const QVector3D anchorPos = positions.value(anchorId);
        const float side = (trunkIndexById.value(anchorId, 0) % 2 == 0) ? 1.0f : -1.0f;
        const float startY = anchorPos.y() - 0.5f * branchGapY * qMax(0, branchIds.size() - 1);

        for (int i = 0; i < branchIds.size(); ++i) {
            const float x = anchorPos.x() + side * branchOffsetX;
            const float y = startY + i * branchGapY;
            const float z = anchorPos.z() + (i % 2 == 0 ? 1.0f : -1.0f) * (branchGapZ * (1 + i / 2));
            positions.insert(branchIds[i], QVector3D(x, y, z));
        }
    }

    if (!aggregateId.isEmpty()) {
        const float centerY = trunkOrder.isEmpty() ? 0.0f : (positions.value(trunkOrder.first()).y() + positions.value(trunkOrder.last()).y()) * 0.5f;
        const float farZ = trunkOrder.isEmpty() ? 0.0f : positions.value(trunkOrder.last()).z() + 80.0f;
        positions.insert(aggregateId, QVector3D(0.0f, centerY, farZ));
    }

    for (NodeRenderData& node : nodes) {
        node.pos = positions.value(node.id, QVector3D(0.0f, 0.0f, 0.0f));
        node.stage = levels.value(node.id, 0);
    }
}

QList<KnowledgeGraph3DWidget::ProjectedNode> KnowledgeGraph3DWidget::projectNodes() const
{
    QList<ProjectedNode> result;
    result.reserve(nodes.size());

    for (const NodeRenderData& node : nodes) {
        float depth = 0.0f;
        QPointF p = projectPoint(node.pos, &depth);

        ProjectedNode projected;
        projected.id = node.id;
        projected.title = node.title;
        projected.pos2d = p;
        projected.zDepth = depth;
        projected.stage = node.stage;
        projected.learned = node.learned;
        projected.mastery = node.mastery;
        projected.focused = (node.id == focusedKnowledgeId);
        projected.expanded = node.expanded;
        projected.trunk = node.trunk;
        projected.aggregate = node.aggregate;
        projected.aggregateCount = node.aggregateCount;

        float depthScale = qBound(0.65f, 1.2f - depth * 0.0013f, 1.35f);
        projected.radius = node.aggregate ? 10.0f * depthScale : (node.trunk ? 8.8f * depthScale : 7.0f * depthScale);

        result.append(projected);
    }

    return result;
}

QPointF KnowledgeGraph3DWidget::projectPoint(const QVector3D& point, float* outDepth) const
{
    const float yawRad = qDegreesToRadians(yaw);
    const float pitchRad = qDegreesToRadians(pitch);

    float x1 = point.x() * qCos(yawRad) + point.z() * qSin(yawRad);
    float z1 = -point.x() * qSin(yawRad) + point.z() * qCos(yawRad);

    float y2 = point.y() * qCos(pitchRad) - z1 * qSin(pitchRad);
    float z2 = point.y() * qSin(pitchRad) + z1 * qCos(pitchRad);

    float zCamera = z2 + cameraDistance;
    float perspective = 380.0f / qMax(50.0f, zCamera);

    float xScreen = x1 * perspective + width() * 0.5f;
    float yScreen = y2 * perspective + height() * 0.5f;

    if (outDepth) {
        *outDepth = zCamera;
    }

    return QPointF(xScreen, yScreen);
}
