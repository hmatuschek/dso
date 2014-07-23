#ifndef __SDR_DSO_DSOWIDGET_HH__
#define __SDR_DSO_DSOWIDGET_HH__

#include <QWidget>
#include "dso.hh"
#include "plotwidget.hh"
#include "dsoctrlwidget.hh"


class DSOWidget : public QWidget
{
  Q_OBJECT
public:
  explicit DSOWidget(DSO *dso, QWidget *parent = 0);

  void updatePlot();

protected:
  DSO *_dso;
  PlotWidget *_plot;
  DSOCtrlWidget *_ctrl;
};

#endif // DSWWIDGET_HH
