#include "dsowidget.hh"
#include <QHBoxLayout>

DSOWidget::DSOWidget(DSO *dso, QWidget *parent)
  : QWidget(parent), _dso(dso), _plot(0), _ctrl(0)
{
  _plot = new PlotWidget(_dso->plotBuffer());
  _ctrl = new DSOCtrlWidget(_dso);

  QHBoxLayout *box = new QHBoxLayout();
  box->addWidget(_plot, 1);
  box->addWidget(_ctrl, 0);
  this->setLayout(box);

  //this->setMinimumSize(840, 480);
}


void
DSOWidget::updatePlot() {
  _plot->update();
}
