#include "dsoplotctrlwidget.hh"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include "plotwidget.hh"


DSOPlotCtrlWidget::DSOPlotCtrlWidget(DSO *dso, QWidget *parent)
  : QWidget(parent), _dso(dso)
{
  _timeFrame = new QLineEdit();
  QDoubleValidator *f_val = new QDoubleValidator();
  f_val->setBottom(0); _timeFrame->setValidator(f_val);
  _timeFrame->setText(QString("%1").arg(_dso->timeFrame()*1000));
  QObject::connect(_timeFrame, SIGNAL(textEdited(QString)), this, SLOT(onTimeFrameChanged(QString)));

  _scale = new QSpinBox();
  _scale->setMinimum(1); _scale->setMaximum((1<<15)-1);
  _scale->setValue(_dso->plotBuffer()->scale());
  QObject::connect(_scale, SIGNAL(valueChanged(int)), this, SLOT(onScaleChanged(int)));

  _hist = new QSpinBox();
  _hist->setMinimum(1); _hist->setMaximum(1024);
  _hist->setValue(_dso->plotBuffer()->numBuffers());
  QObject::connect(_hist, SIGNAL(valueChanged(int)), this, SLOT(onHistChanged(int)));

  QPushButton *reset = new QPushButton(tr("Reset"));
  QObject::connect(reset, SIGNAL(clicked()), this, SLOT(onPlotReset()));

  QVBoxLayout *plot_vbox = new QVBoxLayout();
  QFormLayout *plot_layout = new QFormLayout();
  plot_layout->addRow(tr("Time Frame (ms)"), _timeFrame);
  plot_layout->addRow(tr("History"), _hist);
  plot_layout->addRow(tr("Scale"), _scale);
  plot_vbox->addLayout(plot_layout);
  plot_vbox->addWidget(reset);
  this->setLayout(plot_vbox);
}


DSOPlotCtrlWidget::~DSOPlotCtrlWidget() {
  // pass...
}

void
DSOPlotCtrlWidget::onTimeFrameChanged(QString value) {
  // Get value
  bool ok; double val = value.toDouble(&ok);
  if (! ok) { return; }

  bool wasRunning = sdr::Queue::get().isRunning();
  if (wasRunning) {
    sdr::Queue::get().stop();
    sdr::Queue::get().wait();
  }

  _dso->setTimeFrame(val/1000);

  if (wasRunning) {
    sdr::Queue::get().start();
  }
  _dso->plotBuffer()->triggerUpdate();
}


void
DSOPlotCtrlWidget::onScaleChanged(int value) {
  _dso->plotBuffer()->setScale(value);
  _dso->plotBuffer()->triggerUpdate();
}


void
DSOPlotCtrlWidget::onHistChanged(int N) {
  // Determing if Queue is running
  bool wasRunning = sdr::Queue::get().isRunning();
  // If it is running -> stop it
  if (wasRunning) {
    sdr::Queue::get().stop();
    sdr::Queue::get().wait();
  }
  // set number of buffers
  _dso->plotBuffer()->setNumBuffers(N);
  // If queue was running, restart it
  if (wasRunning) {
    sdr::Queue::get().start();
  }
  _dso->plotBuffer()->triggerUpdate();
}

void
DSOPlotCtrlWidget::onPlotReset() {
  bool wasRunning = sdr::Queue::get().isRunning();
  if (wasRunning) {
    sdr::Queue::get().stop();
    sdr::Queue::get().wait();
  }

  _dso->plotBuffer()->reset();

  if (wasRunning) {
    sdr::Queue::get().start();
  }
  _dso->plotBuffer()->triggerUpdate();
}
