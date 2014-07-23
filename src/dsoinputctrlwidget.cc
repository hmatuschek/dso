#include "dsoinputctrlwidget.hh"
#include <QLabel>
#include <QFormLayout>


DSOInputCtrlWidget::DSOInputCtrlWidget(DSO *dso, QWidget *parent)
  : QWidget(parent), _dso(dso)
{
  _devices = new QComboBox();
  int idx=0, currentDeviceIdx = -1;
  for (int i=0; i<sdr::PortAudio::numDevices(); i++) {
    if (sdr::PortAudio::hasInputStream(i)) {
      _devices->addItem(sdr::PortAudio::deviceName(i).c_str(), i);
      if (_dso->currentInputDevice() == i) {
        currentDeviceIdx = idx;
      }
      idx++;
    }
  }
  // Select current device
  _devices->setCurrentIndex(currentDeviceIdx);

  // Probe sample rates for selected device
  _sampleRates = new QComboBox();
  _probeSampleRates();
  QObject::connect(_sampleRates,SIGNAL(currentIndexChanged(int)),
                   this, SLOT(onSampleRateSelected(int)));

  QFormLayout *layout = new QFormLayout();
  layout->addRow(tr("Device"), _devices);
  layout->addRow(tr("Sample Rate"), _sampleRates);
  this->setLayout(layout);
}

DSOInputCtrlWidget::~DSOInputCtrlWidget() {
  // pass...
}


void
DSOInputCtrlWidget::_probeSampleRates() {
  double sampleRates[7] = {8000., 16000., 32000., 44100., 96000., 192000.};
  int selectedSampleRateIdx = -1;
  for (int i=0; i<7; i++) {
    if (_dso->hasSampleRate(sampleRates[i])) {
      _sampleRates->addItem(tr("%1 Hz").arg(sampleRates[i]), sampleRates[i]);
      if (sampleRates[i] == _dso->sampleRate()) { selectedSampleRateIdx = i; }
    }
  }
  if (0 > selectedSampleRateIdx) {
    // Add currently selected sample rate to the list:
    double rate = _dso->sampleRate();
    _sampleRates->addItem(tr("%1 Hz").arg(rate), rate);
    selectedSampleRateIdx = 7;
  }
  _sampleRates->setCurrentIndex(selectedSampleRateIdx);
}

void
DSOInputCtrlWidget::onDeviceSelected(int idx) {
  int devIdx = _devices->itemData(idx).toInt();
  bool wasRunning = sdr::Queue::get().isRunning();
  if (wasRunning) {
    sdr::Queue::get().stop();
    sdr::Queue::get().wait();
  }
  _dso->setCurrentInputDevice(devIdx);
  if (wasRunning) {
    sdr::Queue::get().start();
  }
}

void
DSOInputCtrlWidget::onSampleRateSelected(int idx) {
  double rate = _sampleRates->itemData(idx).toDouble();
  bool wasRunning = sdr::Queue::get().isRunning();
  if (wasRunning) {
    sdr::Queue::get().stop();
    sdr::Queue::get().wait();
  }
  _dso->setSampleRate(rate);
  if (wasRunning) {
    sdr::Queue::get().start();
  }
}
