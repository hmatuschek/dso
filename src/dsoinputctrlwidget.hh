#ifndef DSOINPUTCTRLWIDGET_HH
#define DSOINPUTCTRLWIDGET_HH

#include <QWidget>
#include "dso.hh"
#include <QComboBox>


class DSOInputCtrlWidget : public QWidget
{
  Q_OBJECT
public:
  explicit DSOInputCtrlWidget(DSO *dso, QWidget *parent = 0);
  virtual ~DSOInputCtrlWidget();

protected slots:
  void onDeviceSelected(int idx);
  void onSampleRateSelected(int idx);

protected:
  void _probeSampleRates();

protected:
  DSO *_dso;
  QComboBox *_devices;
  QComboBox *_sampleRates;
};

#endif // DSOINPUTCTRLWIDGET_HH
