#include "fractalgeneratorview.h"
#include "vec2_qt.hpp"

#include <QGraphicsItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsObject>
#include <QMouseEvent>

#include <functional>

namespace {

struct Line
{
    Vec2d p1;
    Vec2d p2;
};

auto distanceToLine(const Vec2d& pos, const Line& line)
    -> double
{
    auto d1 = pos - line.p1;
    auto dr = line.p2 - line.p1;
    auto length = dr.norm();
    if (length == 0)
        return d1.norm();

    auto dir = dr / length;
    auto x = dir * d1;
    if (x < 0)
        return d1.norm();

    if (x > length)
        return (pos - line.p2).norm();

    return fabs(d1 % dir);
}

struct HandleLines
{
    QGraphicsLineItem* before{};
    QGraphicsLineItem* after{};
};

class HandleItem : public QGraphicsRectItem
{
public:
    HandleItem(const Vec2d& pos,
               HandleLines lines,
               std::function<void()> onPosChanged,
               QGraphicsItem* parent = nullptr) :
        QGraphicsRectItem( rectFromCenter(pos), parent ),
        lines_{ lines },
        onPosChanged_{ onPosChanged }
    {
        setPen(QPen(Qt::black));
        setBrush(Qt::gray);
        setFlags(
            QGraphicsItem::ItemIsMovable /*|
            QGraphicsItem::ItemIsFocusable |
            QGraphicsItem::ItemIsSelectable*/ );
        setZValue(1);
    }

    auto lines() const
        -> HandleLines
    { return lines_; }

    auto setLines(const HandleLines& lines)
        -> void
    { lines_ = lines; }

    auto setLineBefore(QGraphicsLineItem* line)
        -> void
    { lines_.before = line; }

    auto setLineAfter(QGraphicsLineItem* line)
        -> void
    { lines_.after = line; }

protected:
    auto mouseMoveEvent(QGraphicsSceneMouseEvent *event)
        -> void override
    {
        QGraphicsRectItem::mouseMoveEvent(event);

        auto p = rect().center() + pos();

        if (lines_.before)
        {
            auto line = lines_.before->line();
            line.setP2(p);
            lines_.before->setLine(line);
        }

        if (lines_.after)
        {
            auto line = lines_.after->line();
            line.setP1(p);
            lines_.after->setLine(line);
        }

        if (onPosChanged_)
            onPosChanged_();
    }

private:
    static constexpr auto size = 8.;

    static auto rectFromCenter(const Vec2d& pos)
        -> QRectF
    {
        constexpr auto dr = Vec2d{size/2., size/2.};
        return {toQPointF(pos-dr), QSizeF{size, size}};
    }

    HandleLines lines_;
    std::function<void()> onPosChanged_;
};

} // anonymous namespace



struct FractalGeneratorView::Impl
{
    explicit Impl(
            FractalGeneratorView* view,
            FractalGeneratorObject* fractalGeneratorObject) :
        view_{ view },
        fractalGeneratorObject_{ fractalGeneratorObject }
    {
        makeScene();
        view->setScene( &scene_ );
        view->setRenderHints(QPainter::Antialiasing);
        view->setMouseTracking(true);
        view->setMinimumWidth(300);
    }

    static auto makeLineItem(const Vec2d& v1, const Vec2d& v2)
        -> QGraphicsLineItem*
    {
        auto* item = new QGraphicsLineItem(
            QLineF{toQPointF(v1), toQPointF(v2)});
        item->setPen(QPen(Qt::black, 2));
        item->setFlags({});
        return item;
    }

    auto makeHandleItem(const Vec2d& v, const HandleLines& lines)
        -> HandleItem*
    { return new HandleItem{ v, lines, [&]{ updateGeneratorObject(); } }; }

    auto makeScene()
        -> void
    {
        auto handleSize = 8;
        auto handleSize2 = QSizeF(handleSize, handleSize);
        auto dr = Vec2d{handleSize/2., handleSize/2.};
        scene_.clear();
        lines_.clear();
        handles_.clear();

        const auto& fg = fractalGeneratorObject_->fractalGenerator();

        for (size_t index=1, n=fg.size(); index<n; ++index)
        {
            auto* item = makeLineItem(fg[index-1], fg[index]);
            scene_.addItem(item);
            lines_.push_back(item);
        }

        for (size_t index=0, n=fg.size(); index<n; ++index)
        {
            auto* item = makeHandleItem(
                fg[index],
                {
                    .before = index > 0 ? lines_[index-1]: nullptr,
                    .after = index+1 < n ? lines_[index]: nullptr
                });
            handles_.push_back(item);
            scene_.addItem(item);
        }
    }

    auto hoverHandle(HandleItem* h)
        -> void
    {
        for (auto* h2: handles_)
            h2->setBrush(h2 == h? Qt::red: Qt::gray);
    }

    auto hoverLine(QGraphicsLineItem* line)
        -> void
    {
        for (auto* line2: lines_)
            line2->setPen(line2 == line? QPen(Qt::red, 2): QPen(Qt::black, 2));
    }

    auto trackClosestObject(const QPointF& pos)
        -> void
    {
        if (trackHoveredHandle(pos))
            hoverLine(nullptr);
        else
            trackClosestLine(pos);
    }

    auto trackHoveredHandle(const QPointF& pos)
        -> bool
    {
        for (auto it=handles_.rbegin(); it!=handles_.rend(); ++it)
        {
            auto* h = *it;
            if (h->contains(h->mapFromScene(pos)))
            {
                hoverHandle(h);
                return true;
            }
        }

        hoverHandle(nullptr);
        return false;
    }

    auto closestLine(const Vec2d& v)
        -> size_t
    {
        const auto& fg = fractalGeneratorObject_->fractalGenerator();

        size_t closestIndex = ~0;
        auto closestDist = 0.;
        for (size_t index=0, n=fg.size(); index+1<n; ++index)
        {
            auto d = distanceToLine(v, { fg[index], fg[index+1] });
            if (closestIndex == ~0 || d < closestDist)
            {
                closestIndex = index;
                closestDist = d;
            }
        }

        return closestIndex;
    }

    auto trackClosestLine(const QPointF& pos)
        -> void
    {
        auto closestIndex = closestLine(toVec2d(pos));
        hoverLine( closestIndex == ~0? nullptr: lines_[closestIndex] );
    }

    auto handleMouseMoveEvent(QMouseEvent* event)
        -> void
    {
        view_->QGraphicsView::mouseMoveEvent(event);
        trackClosestObject(view_->mapToScene(event->pos()));
    }

    auto handleMousePressEvent(QMouseEvent* event)
        -> void
    {
        auto scenePos = view_->mapToScene(event->pos());
        auto* handleItem =
            dynamic_cast<HandleItem*>(scene_.itemAt(scenePos, {}));
        if (handleItem)
        {
            if (event->modifiers() & Qt::AltModifier)
                removeVertex(handleItem);
            else
                view_->QGraphicsView::mousePressEvent(event);
        }
        else
            addVertex(toVec2d(scenePos));
    }

    auto addVertex(const Vec2d& v)
        -> void
    {
        auto lineIndex = closestLine(v);
        if (lineIndex == ~0)
            return;

        auto fg = fractalGeneratorObject_->fractalGenerator();

        auto* lineItemBefore = lines_[lineIndex];
        auto lineBefore = lineItemBefore->line();
        lineBefore.setP2(toQPointF(v));
        lineItemBefore->setLine(lineBefore);

        auto* lineItemAfter = makeLineItem(v, fg[lineIndex+1]);
        scene_.addItem(lineItemAfter);
        lines_.insert(lines_.begin() + lineIndex+1, lineItemAfter);

        handles_[lineIndex+1]->setLineBefore(lineItemAfter);

        auto* newHandle = makeHandleItem(v, { lineItemBefore, lineItemAfter });
        scene_.addItem(newHandle);
        handles_.insert(handles_.begin() + lineIndex+1, newHandle);

        fg.insert(fg.begin() + lineIndex+1, v);
        fractalGeneratorObject_->setFractalGenerator(fg);
    }

    auto removeVertex(HandleItem* handleItem)
        -> void
    {
        auto index = std::find(handles_.begin(), handles_.end(), handleItem) -
                     handles_.begin();
        if (index == 0 || index+1 == handles_.size())
            return;
        Q_ASSERT(index > 0 && index+1 < handles_.size());

        auto fg = fractalGeneratorObject_->fractalGenerator();
        fg.erase(fg.begin() + index);

        scene_.removeItem(handleItem);
        handles_.erase(handles_.begin() + index);

        scene_.removeItem(lines_[index]);
        lines_.erase(lines_.begin() + index);

        auto* lineItem = lines_[index-1];
        auto line = lineItem->line();
        line.setP2(toQPointF(fg[index]));
        lineItem->setLine(line);

        handles_[index]->setLineBefore(lineItem);

        fractalGeneratorObject_->setFractalGenerator(fg);
    }

    auto updateGeneratorObject()
        -> void
    {
        auto fg = fractalGeneratorObject_->fractalGenerator();
        Q_ASSERT(fg.size() == handles_.size());
        for (size_t index=0, n=fg.size(); index<n; ++index)
        {
            auto* h = handles_[index];
            fg[index] = toVec2d(h->rect().center() + h->pos());
        }
        fractalGeneratorObject_->setFractalGenerator(fg);
    }

    FractalGeneratorView* view_;
    FractalGeneratorObject* fractalGeneratorObject_;
    QGraphicsScene scene_;

    std::vector<QGraphicsLineItem*> lines_;
    std::vector<HandleItem*> handles_;
};

FractalGeneratorView::~FractalGeneratorView() = default;

FractalGeneratorView::FractalGeneratorView(
        FractalGeneratorObject* fractalGeneratorObject,
        QWidget* parent) :
    QGraphicsView{ parent },
    impl_{ std::make_unique<Impl>(this, fractalGeneratorObject) }
{}

auto FractalGeneratorView::mouseMoveEvent(QMouseEvent *event)
    -> void
{ impl_->handleMouseMoveEvent(event); }

auto FractalGeneratorView::mousePressEvent(QMouseEvent* event)
    -> void
{ impl_->handleMousePressEvent(event); }
