#ifndef __SDR_DSO_DSO_HH__
#define __SDR_DSO_DSO_HH__

#include <QApplication>
#include "dsobuffer.hh"
#include <portaudio.hh>


// Forward declaration
class PlotBuffer;

/** Central application object for a DSO application. */
class DSO : public QApplication
{
  Q_OBJECT

public:
  explicit DSO(int &argc, char **argv);
  virtual ~DSO();

  /** Constructs a new view to the DSO object. */
  QWidget *view();

  /** Retunrs the selected trigger. */
  DSOTrigger trigger() const;
  /** Sets the selected trigger. */
  void setTrigger(DSOTrigger trigger);

  /** Returns the lower threshold for a trigger.
   * If the selected trigger is none, the value is ignored. */
  int16_t base() const;
  /** Sets the lower threshold for a trigger. */
  void setBase(int16_t base);

  int16_t threshold() const;
  void setThreshold(int16_t thres);

  double timeFrame() const;
  void setTimeFrame(double time);

  inline PlotBuffer *plotBuffer() { return _plotBuffer; }

  inline bool isRecording() const { return _recorder->isOpen(); }
  void record(const QString &filename);
  void stopRecord();

  inline double avgPeriod() const { return _dsoBuffer->avgPeriod(); }
  inline size_t eventCount() const { return _dsoBuffer->eventCount(); }
  inline double avgRate() const { return 1./_dsoBuffer->avgPeriod(); }
  inline void rateStatistics(double &mean, double &lower, double &upper) const {
    _dsoBuffer->rateStatistics(mean, lower, upper);
  }
  inline void periodStatistics(double &mean, double &lower, double &upper) const {
    _dsoBuffer->periodStatistics(mean, lower, upper);
  }
  inline void resetStatistics() const { _dsoBuffer->resetStatistics(); }

  inline int currentInputDevice() const { return _src->deviceIndex(); }
  inline void setCurrentInputDevice(int idx) { _src->setDeviceIndex(idx); }
  double sampleRate() const { return _src->sampleRate(); }
  bool hasSampleRate(double sampleRate) const { return _src->hasSampleRate(sampleRate); }
  void setSampleRate(double rate) { _src->setSampleRate(rate); }

protected:
  sdr::PortSource<int16_t> *_src;
  DSOBuffer<int16_t> *_dsoBuffer;
  PlotBuffer *_plotBuffer;
  DSOCSVRecorder<int16_t> *_recorder;
};

#endif // __SDR_DSO_DSO_HH__
