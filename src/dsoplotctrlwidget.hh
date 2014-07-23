#ifndef DSOPLOTCTRLWIDGET_HH
#define DSOPLOTCTRLWIDGET_HH

#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>

#include "dso.hh"


class DSOPlotCtrlWidget : public QWidget
{
  Q_OBJECT
public:
  explicit DSOPlotCtrlWidget(DSO *dso, QWidget *parent = 0);
  virtual ~DSOPlotCtrlWidget();

protected slots:
  void onTimeFrameChanged(QString value);
  void onScaleChanged(int value);
  void onHistChanged(int N);
  void onPlotReset();

protected:
  DSO *_dso;
  QLineEdit *_timeFrame;
  QSpinBox  *_scale;
  QSpinBox  *_hist;
};

#endif // DSOPLOTCTRLWIDGET_HH
