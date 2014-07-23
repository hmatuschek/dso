#include "dso.hh"
#include <portaudio.hh>
#include <queue.hh>

#include "dsowidget.hh"
#include "dsobuffer.hh"
#include "plotwidget.hh"



DSO::DSO(int &argc, char **argv) :
  QApplication(argc, argv), _src(0), _dsoBuffer(0), _plotBuffer(0)
{
  sdr::PortAudio::init();
  sdr::Queue &queue = sdr::Queue::get();

  _src = new sdr::PortSource<int16_t>(96000.0, 1024);
  _dsoBuffer = new DSOBuffer<int16_t>(TRIGGER_NONE, 0.01, 8000, 8000);
  _plotBuffer = new PlotBuffer(1, this);
  _recorder = new DSOCSVRecorder<int16_t>();

  _src->connect(_dsoBuffer);
  _dsoBuffer->connect(_plotBuffer);
  _dsoBuffer->connect(_recorder);

  queue.addIdle(_src, &sdr::PortSource<int16_t>::next);
}

DSO::~DSO() {
  if (sdr::Queue::get().isRunning()) {
    sdr::Queue::get().stop();
    sdr::Queue::get().wait();
  }
  sdr::PortAudio::terminate();

  delete _src;
  delete _dsoBuffer;
  delete _plotBuffer;
  delete _recorder;
}


QWidget *
DSO::view() {
  return new DSOWidget(this);
}

DSOTrigger DSO::trigger() const { return _dsoBuffer->trigger(); }
void DSO::setTrigger(DSOTrigger trigger) { _dsoBuffer->setTrigger(trigger); }
int16_t DSO::base() const { return _dsoBuffer->base(); }
void DSO::setBase(int16_t base) { _dsoBuffer->setBase(base); }
int16_t DSO::threshold() const { return _dsoBuffer->threshold(); }
void DSO::setThreshold(int16_t thres) { _dsoBuffer->setThreshold(thres); }
double DSO::timeFrame() const { return _dsoBuffer->timeFrame(); }
void DSO::setTimeFrame(double time) { _dsoBuffer->setTimeFrame(time); }

void
DSO::record(const QString &filename) {
  _recorder->open(filename.toStdString());
}

void
DSO::stopRecord() {
  _recorder->close();
}
