#include "dsotriggerctrlwidget.hh"
#include <QPushButton>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include "plotwidget.hh"


DSOTriggerCtrlWidget::DSOTriggerCtrlWidget(DSO *dso, QWidget *parent)
  : QWidget(parent), _dso(dso)
{
  _mode  = new QComboBox();
  _mode->addItem(tr("None"), int(TRIGGER_NONE));
  _mode->addItem(tr("Rising Edge"), int(TRIGGER_RISING));
  _mode->addItem(tr("Falling Edge"), int(TRIGGER_FALLING));
  _mode->addItem(tr("Exceeds Range"), int(TRIGGER_EXCEED));
  switch (_dso->trigger()) {
  case TRIGGER_NONE:    _mode->setCurrentIndex(0); break;
  case TRIGGER_RISING:  _mode->setCurrentIndex(1); break;
  case TRIGGER_FALLING: _mode->setCurrentIndex(2); break;
  case TRIGGER_EXCEED:  _mode->setCurrentIndex(3); break;
  }
  QObject::connect(_mode, SIGNAL(currentIndexChanged(int)), this, SLOT(onModeChanged(int)));

  _threshold = new QLineEdit();
  QDoubleValidator *t_val = new QDoubleValidator(-1,1, 3);
  _threshold->setValidator(t_val);
  _threshold->setText(QString::number(double(_dso->threshold())/((1<<15)-1)));
  if (TRIGGER_NONE == _dso->trigger()) {
    _threshold->setEnabled(false);
  }
  QObject::connect(_threshold, SIGNAL(textEdited(QString)), this, SLOT(onThesholdChanged(QString)));

  _base = new QLineEdit();
  QDoubleValidator *base_val = new QDoubleValidator(-1,1, 3);
  _base->setValidator(base_val);
  if (TRIGGER_EXCEED != _dso->trigger()) {
    _base->setEnabled(false);
    _dso->setBase(_dso->threshold());
  }
  _base->setText(QString::number(double(_dso->base())/((1<<15)-1)));
  QObject::connect(_base, SIGNAL(textEdited(QString)), this, SLOT(onBaseChanged(QString)));

  _count = new QLabel();
  _rate = new QLabel(); _rate->setTextFormat(Qt::RichText);
  _period = new QLabel(); _period->setTextFormat(Qt::RichText);
  if (TRIGGER_NONE == _dso->trigger()) {
    _rate->setText(tr("-- Hz"));
    _period->setText(tr("-- s"));
  } else {
    double mean, lower, upper, err;
    _dso->periodStatistics(mean, lower, upper); err = (upper-lower)/mean;
    if (0 > upper) { err = std::numeric_limits<double>::quiet_NaN(); }
    _period->setText(tr("%1 (%2%) s").arg(mean, 0, 'g', 3).arg(err, 0, 'g', 3));
    _dso->rateStatistics(mean, lower, upper); err = (upper-lower)/mean;
    if (0 > lower) { err = std::numeric_limits<double>::quiet_NaN(); }
    _rate->setText(tr("%1 (%2%) Hz").arg(mean, 0, 'g', 3).arg(err, 0, 'g', 3));
  }
  _count->setText(QString::number(_dso->eventCount()));
  QPushButton *reset_period = new QPushButton(tr("Reset"));
  QObject::connect(_dso->plotBuffer(), SIGNAL(buffersUpdated()), this, SLOT(onTrigger()));
  QObject::connect(reset_period, SIGNAL(clicked()), this, SLOT(onResetPeriod()));

  QVBoxLayout *box = new QVBoxLayout();

  QFormLayout *trigger_layout = new QFormLayout();
  trigger_layout->addRow(tr("Trigger"), _mode);
  trigger_layout->addRow(tr("Threshold"), _threshold);
  trigger_layout->addRow(tr("Base"), _base);
  QGroupBox *trigger_box = new QGroupBox(tr("Trigger"));
  trigger_box->setLayout(trigger_layout);
  box->addWidget(trigger_box);

  QVBoxLayout *stat_vbox = new QVBoxLayout();
  QFormLayout *stat_layout = new QFormLayout();
  stat_layout->addRow(tr("Count"), _count);
  stat_layout->addRow(tr("Period"), _period);
  stat_layout->addRow(tr("Rate"), _rate);
  stat_vbox->addLayout(stat_layout);
  stat_vbox->addWidget(reset_period);
  QGroupBox *stat_box = new QGroupBox(tr("Statistics"));
  stat_box->setLayout(stat_vbox);
  box->addWidget(stat_box);

  this->setLayout(box);
}

DSOTriggerCtrlWidget::~DSOTriggerCtrlWidget() {
  // pass...
}


void
DSOTriggerCtrlWidget::onModeChanged(int idx) {
  DSOTrigger trigger = DSOTrigger(_mode->itemData(idx).toInt());
  _dso->setTrigger(trigger);

  switch (trigger) {
  case TRIGGER_NONE:
    _threshold->setEnabled(false);
    _base->setEnabled(false);
    _count->setText("0");
    _period->setText(tr("-- s"));
    _rate->setText(tr("-- Hz"));
    break;
  case TRIGGER_EXCEED:
    _base->setEnabled(true);
    _threshold->setEnabled(true);
    break;
  case TRIGGER_RISING:
  case TRIGGER_FALLING:
    _threshold->setEnabled(true);
    _base->setEnabled(false);
    _base->setText(_threshold->text());
    _dso->setBase(_dso->threshold());
    break;
  }
  _dso->plotBuffer()->triggerUpdate();
}

void
DSOTriggerCtrlWidget::onThesholdChanged(QString value) {
  bool ok;
  double val = value.toDouble(&ok);
  val *= (1<<15)-1;
  if (ok) {
    _dso->setThreshold(val);
    if (TRIGGER_EXCEED != _dso->trigger()) {
      _base->setText(value);
      _dso->setBase(val);
    }
  }
  _dso->plotBuffer()->triggerUpdate();
}

void
DSOTriggerCtrlWidget::onBaseChanged(QString value) {
  bool ok;
  double val = value.toDouble(&ok);
  val *= (1<<15)-1;
  if (ok) { _dso->setBase(val); }
  _dso->plotBuffer()->triggerUpdate();
}

void
DSOTriggerCtrlWidget::onTrigger() {
  if (TRIGGER_NONE != _dso->trigger()) {
    double mean, lower, upper, err;
    _dso->periodStatistics(mean, lower, upper); err = (upper-lower)/mean;
    if (0 > upper) { err = std::numeric_limits<double>::quiet_NaN(); }
    _period->setText(tr("%1 (%2%) s").arg(mean, 0, 'g', 3).arg(err, 0, 'g', 3));
    _dso->rateStatistics(mean, lower, upper); err = (upper-lower)/mean;
    if (0 > lower) { err = std::numeric_limits<double>::quiet_NaN(); }
    _rate->setText(tr("%1 (%2%) Hz").arg(mean, 0, 'g', 3).arg(err, 0, 'g', 3));
    _count->setText(QString::number(_dso->eventCount()));
  }
}

void
DSOTriggerCtrlWidget::onResetPeriod() {
  _dso->resetStatistics();
  this->onTrigger();
}
