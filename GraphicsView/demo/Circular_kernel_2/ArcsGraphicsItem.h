#ifndef CGAL_QT_ARCS_GRAPHICS_ITEM_H
#define CGAL_QT_ARCS_GRAPHICS_ITEM_H

#include <CGAL/Bbox_2.h>
#include <CGAL/Qt/PainterOstream.h>
#include <CGAL/Qt/GraphicsItem.h>
#include <CGAL/Qt/Converter.h>

#include <QGraphicsScene>
#include <QPainter>
#include <QStyleOption>

namespace CGAL {
namespace Qt {

template <typename CK>
class ArcsGraphicsItem : public GraphicsItem
{
  typedef typename CK::Circle_2 Circle_2;
  typedef typename CK::Circular_arc_2 Circular_arc_2;
  typedef typename CK::Circular_arc_point_2 Circular_arc_point_2;
  typedef typename CK::Line_arc_2 Line_arc_2;
  typedef std::variant<std::pair<Circular_arc_point_2,unsigned>, Circular_arc_2, Line_arc_2 > Inter_variant;
  typedef std::variant<Circular_arc_2, Line_arc_2 > Arc_variant;

  std::vector<Arc_variant>& arcs;
  std::vector<Inter_variant>& intersections;

public:
  ArcsGraphicsItem(std::vector<Arc_variant>& arcs_, std::vector<Inter_variant>& intersections_);

  void modelChanged();

public:
  QRectF boundingRect() const;

  void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);


  const QPen& intersectionsPen() const
  {
    return intersections_pen;
  }

  const QPen& inputPen() const
  {
    return input_pen;
  }

  void setIntersectionsPen(const QPen& pen)
  {
    intersections_pen = pen;
  }

  void setInputPen(const QPen& pen)
  {
    input_pen = pen;
  }

protected:
  void updateBoundingBox();

  QPainter* m_painter;
  PainterOstream<CK> painterostream;

  QRectF bounding_rect;

  QPen intersections_pen;
  QPen input_pen;
  Converter<CK> convert;
};


template <typename CK>
ArcsGraphicsItem<CK>::ArcsGraphicsItem(std::vector<Arc_variant>& arcs_, std::vector<Inter_variant>& intersections_)
  :  arcs(arcs_), intersections(intersections_), painterostream(nullptr)
{
  setIntersectionsPen(QPen(::Qt::red, 3.));
  if(arcs.empty()){
    this->hide();
  }
  updateBoundingBox();
  setZValue(3);
}

template <typename CK>
QRectF
ArcsGraphicsItem<CK>::boundingRect() const
{
  return bounding_rect;
}



template <typename CK>
void
ArcsGraphicsItem<CK>::paint(QPainter *painter,
                                    const QStyleOptionGraphicsItem *,
                                    QWidget * )
{
  painter->setPen(this->inputPen());
  painterostream = PainterOstream<CK>(painter);

  for(typename std::vector<Arc_variant>::iterator it = arcs.begin(); it != arcs.end(); ++it){
    Circular_arc_2 ca;
    Line_arc_2 la;
    if(auto ca = std::get_if<Circular_arc_2>(&(*it))){
      painterostream << *ca;
    } else if(auto la = std::get_if<Line_arc_2>(&(*it))){
      painterostream << *la;
    }
  }


  painter->setPen(this->intersectionsPen());
  painterostream = PainterOstream<CK>(painter);
  for(typename std::vector<Inter_variant>::iterator it = intersections.begin(); it != intersections.end(); ++it){
    if(auto cap_ui = std::get_if<std::pair<Circular_arc_point_2,unsigned>>(&*it)){
      QTransform matrix = painter->worldTransform();
      painter->resetTransform();
      painter->drawPoint(matrix.map(convert(cap_ui->first)));
      painter->setWorldTransform(matrix);
    }if(auto ca = std::get_if<Circular_arc_2>(&(*it))){
      painterostream << *ca;
    } else if(auto la = std::get_if<Line_arc_2>(&(*it))){
      painterostream << *la;
    }
  }
}

template <typename CK>
void
ArcsGraphicsItem<CK>::updateBoundingBox()
{
  bounding_rect = QRectF(0,0,100, 100);
  Bbox_2 bb;
  bool initialized = false;
  for(typename std::vector<Arc_variant>::iterator it = arcs.begin(); it != arcs.end(); ++it){
    Circular_arc_2 ca;
    Line_arc_2 la;
    if(auto ca = std::get_if<Circular_arc_2>(&(*it))){
      if(initialized){
        bb = bb + ca->supporting_circle().bbox();
      } else {
        initialized = true;
        bb = ca->supporting_circle().bbox();
      }
    } else if(auto la = std::get_if<Line_arc_2>(&(*it))){
      if(initialized){
        bb = bb + la->bbox();
      } else {
        initialized = true;
        bb = la->bbox();
      }
    }
  }

  prepareGeometryChange();
  bounding_rect = convert(bb);
}


template <typename CK>
void
ArcsGraphicsItem<CK>::modelChanged()
{
  if((arcs.empty()) ){
    this->hide();
  } else if((! arcs.empty()) && (! this->isVisible())){
    this->show();
  }
  updateBoundingBox();
  update();
}


} // namespace Qt
} // namespace CGAL

#endif // CGAL_QT_ARCS_GRAPHICS_ITEM_H
