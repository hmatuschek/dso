#include "dsoctrlwidget.hh"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QIntValidator>
#include <QDoubleValidator>
#include <QGroupBox>
#include <QDir>
#include <QDate>
#include <QFileDialog>
#include <queue.hh>
#include <QTabWidget>

#include "plotwidget.hh"
#include "dsoinputctrlwidget.hh"
#include "dsotriggerctrlwidget.hh"
#include "dsoplotctrlwidget.hh"
#include "dsorecordctrlwidget.hh"
#include "dsoaboutwidget.hh"


DSOCtrlWidget::DSOCtrlWidget(DSO *dso, QWidget *parent)
  : QWidget(parent), _dso(dso)
{
  _start = new QPushButton();
  _start->setCheckable(true);
  if (sdr::Queue::get().isRunning()) {
    _start->setChecked(true);
    _start->setText(tr("stop"));
  } else {
    _start->setChecked(false);
    _start->setText(tr("start"));
  }
  QObject::connect(_start, SIGNAL(toggled(bool)), this, SLOT(onStartToggled(bool)));


  QVBoxLayout *box = new QVBoxLayout();
  box->addWidget(_start);
  QTabWidget *tabs = new QTabWidget();
  tabs->addTab(new DSOInputCtrlWidget(_dso), tr("Input"));
  tabs->addTab(new DSOTriggerCtrlWidget(_dso), tr("Trigger"));
  tabs->addTab(new DSOPlotCtrlWidget(_dso), tr("Plot"));
  tabs->addTab(new DSORecordCtrlWidget(_dso), tr("Record"));
  tabs->addTab(new DSOAboutWidget(), tr("About"));

  // Set "Trigger" as the default tab to be shown.
  tabs->setCurrentIndex(1);
  box->addWidget(tabs);
  setLayout(box);
}


void
DSOCtrlWidget::onStartToggled(bool toggled) {
  if (toggled && sdr::Queue::get().isStopped()) {
    sdr::Queue::get().start();
    _start->setText(tr("stop"));
  } else if (!toggled && sdr::Queue::get().isRunning()) {
    sdr::Queue::get().stop();
    _start->setText(tr("start"));
  }
  _dso->plotBuffer()->triggerUpdate();
}

