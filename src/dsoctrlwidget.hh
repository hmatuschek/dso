#ifndef __SDR_DSO_DSOCTRLWIDGET_HH__
#define __SDR_DSO_DSOCTRLWIDGET_HH__

#include <QWidget>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>
#include "dso.hh"


class DSOCtrlWidget : public QWidget
{
  Q_OBJECT
public:
  explicit DSOCtrlWidget(DSO *dso, QWidget *parent = 0);

protected slots:
  void onStartToggled(bool toggled);

protected:
  DSO *_dso;
  QPushButton *_start;
};

#endif // __SDR_DSO_DSOCTRLWIDGET_HH__
