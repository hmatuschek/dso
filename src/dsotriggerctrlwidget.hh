#ifndef DSOTRIGGERCTRLWIDGET_HH
#define DSOTRIGGERCTRLWIDGET_HH

#include <QWidget>
#include "dso.hh"
#include <QComboBox>
#include <QLineEdit>
#include <QLabel>


class DSOTriggerCtrlWidget : public QWidget
{
  Q_OBJECT
public:
  explicit DSOTriggerCtrlWidget(DSO *dso, QWidget *parent = 0);
  virtual ~DSOTriggerCtrlWidget();

protected slots:
  void onModeChanged(int idx);
  void onThesholdChanged(QString value);
  void onBaseChanged(QString value);
  void onTrigger();
  void onResetPeriod();

protected:
  DSO *_dso;
  QComboBox *_mode;
  QLineEdit *_threshold;
  QLineEdit *_base;
  QLabel    *_count;
  QLabel    *_period;
  QLabel    *_rate;
};

#endif // DSOTRIGGERCTRLWIDGET_HH
