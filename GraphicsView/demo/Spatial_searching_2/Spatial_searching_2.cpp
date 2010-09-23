#include <fstream>
// CGAL headers
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/point_generators_2.h>
#include <CGAL/Orthogonal_k_neighbor_search.h>
#include <CGAL/Search_traits_2.h>

// Qt headers
#include <QtGui>
#include <QString>
#include <QFileDialog>
#include <QGraphicsLineItem>

// GraphicsView items and event filters (input classes)
#include <CGAL/Qt/PointsInKdTreeGraphicsItem.h>
#include <CGAL/Qt/utility.h>
  
// the two base classes
#include "ui_Spatial_searching_2.h"
#include <CGAL/Qt/DemosMainWindow.h>

#include "NearestNeighbor.h"

typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
typedef K::Point_2 Point_2;
typedef K::Vector_2 Vector_2;
typedef K::Segment_2 Segment_2;
typedef K::Iso_rectangle_2 Iso_rectangle_2;
typedef CGAL::Search_traits_2<K> TreeTraits;
typedef CGAL::Orthogonal_k_neighbor_search<TreeTraits> Neighbor_search;
typedef Neighbor_search::Tree Tree;

typedef CGAL::Qt::NearestNeighbor<Neighbor_search> NearestNeighbor;

class MainWindow :
  public CGAL::Qt::DemosMainWindow,
  public Ui::Spatial_searching_2
{
  Q_OBJECT
  
private:
  Tree tree;

  CGAL::Qt::Converter<K> convert;
  QGraphicsScene scene;  

  CGAL::Qt::PointsInKdTreeGraphicsItem<Tree> * pgi;
  NearestNeighbor * nearest_neighbor;

public:
  MainWindow();

  template <typename G>
  void
  on_actionGenerate_triggered()
  {

    QRectF rect = CGAL::Qt::viewportsBbox(&scene);
    CGAL::Qt::Converter<K> convert;  
    Iso_rectangle_2 isor = convert(rect);
    Point_2 center = CGAL::midpoint(isor[0], isor[2]);
    Vector_2 offset = center - CGAL::ORIGIN;
    double w = isor.xmax() - isor.xmin();
    double h = isor.ymax() - isor.ymin();
    double radius = (w<h) ? w/2 : h/2;

    G pg(radius);
    bool ok = false;
    const int number_of_points = 
      QInputDialog::getInteger(this, 
                               tr("Number of random points"),
                               tr("Enter number of random points"),
                               100,
                               0,
                               std::numeric_limits<int>::max(),
                               1,
                               &ok);

    if(!ok) {
      return;
    }

    // wait cursor
    QApplication::setOverrideCursor(Qt::WaitCursor);

    std::vector<Point_2> points; 

    points.reserve(number_of_points);
    for(int i = 0; i < number_of_points; ++i){
      points.push_back(*pg + offset);
      ++pg;
    }
    tree.insert(points.begin(), points.end());
    
    // default cursor
    QApplication::restoreOverrideCursor();
    emit(changed());
  }

public slots:

  virtual void open(QString fileName);
  void N_changed(int i);
  void on_actionClear_triggered();
  void on_actionLoadPoints_triggered();
  void on_actionRecenter_triggered();
  void on_actionGeneratePointsOnCircle_triggered();
  void on_actionGeneratePointsInSquare_triggered();
  void on_actionGeneratePointsInDisc_triggered();

  void clear();

signals:
  void changed();
};


MainWindow::MainWindow()
  : DemosMainWindow()
{
  setupUi(this);

  this->graphicsView->setAcceptDrops(false);

  std::vector<Point_2> points; 
  points.push_back(Point_2(0,0));
  points.push_back(Point_2(2,2));
  points.push_back(Point_2(102,2));
  tree.insert(points.begin(), points.end());
  // Add a GraphicItem for the point set
  pgi = new CGAL::Qt::PointsInKdTreeGraphicsItem<Tree>(&tree);

  QObject::connect(this, SIGNAL(changed()),
		   pgi, SLOT(modelChanged()));

  pgi->setVerticesPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  scene.addItem(pgi);

  nearest_neighbor = new NearestNeighbor(&scene, &tree, this, this->nn->value());
  nearest_neighbor->setPen(QPen(Qt::red, 0, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
  scene.installEventFilter(nearest_neighbor);

  // 
  // Manual handling of actions
  //

  QObject::connect(this->nn, SIGNAL(valueChanged(int)),
		   this, SLOT(N_changed(int)));

  QObject::connect(this->actionQuit, SIGNAL(triggered()), 
		   this, SLOT(close()));

 
  //
  // Setup the scene and the view
  //
  scene.setItemIndexMethod(QGraphicsScene::NoIndex);
  scene.setSceneRect(0, 0, 10, 10);
  this->graphicsView->setScene(&scene);
  this->graphicsView->setMouseTracking(true);

  // Uncomment the following line to get antialiasing by default.
//   actionUse_Antialiasing->setChecked(true);

  // Turn the vertical axis upside down
  this->graphicsView->scale(1, -1);
                                                      
  // The navigation adds zooming and translation functionality to the
  // QGraphicsView
  this->addNavigation(this->graphicsView);

  this->setupStatusBar();
  this->setupOptionsMenu();
  this->addAboutDemo(":/cgal/help/about_Spatial_searching_2.html");
  this->addAboutCGAL();

  this->addRecentFiles(this->menuFile, this->actionQuit);
  connect(this, SIGNAL(openRecentFile(QString)),
	  this, SLOT(open(QString)));
}



void MainWindow::N_changed(int i)
{
  nearest_neighbor->setN(i);
  emit(changed());
}


/* 
 *  Qt Automatic Connections
 *  http://doc.trolltech.com/4.4/designer-using-a-component.html#automatic-connections
 * 
 *  setupUi(this) generates connections to the slots named
 *  "on_<action_name>_<signal_name>"
 */



void
MainWindow::on_actionClear_triggered()
{
  clear();
  emit(changed());
}

void
MainWindow::on_actionRecenter_triggered()
{
  // this->graphicsView->setSceneRect(convert(square));
  // this->graphicsView->fitInView(convert(square), Qt::KeepAspectRatio);  
}

void
MainWindow::on_actionGeneratePointsOnCircle_triggered()
{
  typedef CGAL::Random_points_on_circle_2<Point_2> Generator;
  on_actionGenerate_triggered<Generator>();
}


void
MainWindow::on_actionGeneratePointsInSquare_triggered()
{
  typedef CGAL::Random_points_in_square_2<Point_2> Generator;
  on_actionGenerate_triggered<Generator>();
}


void
MainWindow::on_actionGeneratePointsInDisc_triggered()
{
  typedef CGAL::Random_points_in_disc_2<Point_2> Generator;
  on_actionGenerate_triggered<Generator>();
}


void
MainWindow::on_actionLoadPoints_triggered()
{
  QString fileName = QFileDialog::getOpenFileName(this,
						  tr("Open Points file"),
						  ".");
  if(! fileName.isEmpty()){
    open(fileName);
  }
}



void
MainWindow::open(QString fileName)
{
  // wait cursor
  QApplication::setOverrideCursor(Qt::WaitCursor);
  std::ifstream ifs(qPrintable(fileName));
  
  K::Point_2 p;
  std::vector<K::Point_2> points;
  while(ifs >> p) {
    points.push_back(p);
  }
  tree.insert(points.begin(), points.end());

  // default cursor
  QApplication::restoreOverrideCursor();
  this->addToRecentFiles(fileName);
  actionRecenter->trigger();
  emit(changed());
    
}

void
MainWindow::clear()
{
  tree.clear();  
}


#include "Spatial_searching_2.moc"

int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  app.setOrganizationDomain("geometryfactory.com");
  app.setOrganizationName("GeometryFactory");
  app.setApplicationName("Spatial_searching_2 demo");

  // Import resources from libCGALQt4.
  // See http://doc.trolltech.com/4.4/qdir.html#Q_INIT_RESOURCE
  Q_INIT_RESOURCE(File);
  Q_INIT_RESOURCE(Spatial_searching_2);
  Q_INIT_RESOURCE(Input);
  Q_INIT_RESOURCE(CGAL);

  MainWindow mainWindow;
  mainWindow.show();
  QApplication::setOverrideCursor(Qt::CrossCursor);
  mainWindow.on_actionRecenter_triggered();
  return app.exec();
}