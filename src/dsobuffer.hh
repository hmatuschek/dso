#ifndef __SDR_DSO_DSOBUFFER_HH__
#define __SDR_DSO_DSOBUFFER_HH__

#include <logger.hh>
#include <buffer.hh>
#include <node.hh>
#include <traits.hh>
#include <pthread.h>
#include <fstream>
#include <limits>

typedef enum {
  TRIGGER_NONE,    ///< No trigger at all, every frame will be processed.
  TRIGGER_RISING,  ///< Rising edge trigger.
  TRIGGER_FALLING, ///< Falling edge trigger.
  TRIGGER_EXCEED   ///< Triggers if a value exceeds a specified range.
} DSOTrigger;


/** A buffer node that tiggers a redraw whenever a certain condtion is satisfied. */
template <class Scalar>
class DSOBuffer: public sdr::Sink<Scalar>, public sdr::Source
{
public:
  DSOBuffer(DSOTrigger trigger, double timeFrame, Scalar base=0, Scalar thres=0, int maxEvents=1024)
    : sdr::Sink<Scalar>(), sdr::Source(), _trigger(trigger), _timeFrame(timeFrame),
      _base(base), _threshold(thres), _Fs(0), _N(0),
      _eventTimes(), _eventTimesIdx(0), _sampleCount(0), _buffer(), _outBuffer()
  {
    _eventTimes.reserve(maxEvents);
    pthread_mutex_init(&_bufferLock, 0);
  }

  virtual ~DSOBuffer() {
    pthread_mutex_destroy(&_bufferLock);
  }

  virtual void config(const sdr::Config &src_cfg) {
    // Requires type, sample rate & buffer size
    if (!src_cfg.hasType() || !src_cfg.hasSampleRate() || !src_cfg.hasBufferSize()) { return; }
    // Check buffer type
    if (sdr::Config::typeId<Scalar>() != src_cfg.type()) {
      sdr::ConfigError err;
      err << "Can not configure DSOBuffer: Invalid type " << src_cfg.type()
          << ", expected " << sdr::Config::typeId<Scalar>();
      throw err;
    }

    _eventTimesIdx = 0; _sampleCount = 0;
    _Fs = src_cfg.sampleRate();
    // Estimate minimum # samples to match time frame
    _N  = _timeFrame*_Fs;
    // Ensure N >= 2
    if (2 > _N) { _N = 2; }

    // Allocate ring-buffer
    _buffer = sdr::RingBuffer<Scalar>(2*_N);
    // Allocate output buffer
    _outBuffer = sdr::Buffer<Scalar>(_N);

    sdr::LogMessage msg(sdr::LOG_DEBUG);
    msg << "Configured DSOBuffer node:" << std::endl
        << " type " << sdr::Config::typeId<Scalar>() << std::endl
        << " sample-rate " << _Fs << "Hz" << std::endl
        << " ring buffer size " << _buffer.size() << std::endl
        << " out buffer size " << _outBuffer.size() << std::endl;
    if (TRIGGER_NONE == _trigger) {
      msg <<" trigger: none";
    } else if (TRIGGER_RISING == _trigger) {
      msg << " trigger: rising edge from " << _base << " to " << _threshold;
    } else if (TRIGGER_RISING == _trigger) {
      msg << " trigger: falling edge from " << _base << " to " << _threshold;
    } else if (TRIGGER_EXCEED == _trigger) {
      msg << " trigger: exceeds range from " << _base << " to " << _threshold;
    }
    sdr::Logger::get().log(msg);

    // Propergate config
   this->setConfig(sdr::Config(sdr::Traits<Scalar>::scalarId, _Fs, _N, 1));
  }


  virtual void process(const sdr::Buffer<Scalar> &buffer, bool allow_overwrite) {
    size_t inOffset = 0;
    while (inOffset < buffer.size()) {
      try {
        pthread_mutex_lock(&_bufferLock);
        // store some values into the ring buffer
        size_t Nstored = std::min(buffer.size()-inOffset, _buffer.free());
        if (0 < Nstored) {
          _buffer.put(buffer.sub(inOffset, Nstored)); inOffset += Nstored;
          // If ring buffer is full
          if (0 == _buffer.free()) {
            _checkTrigger();
          }
        }
        pthread_mutex_unlock(&_bufferLock);
      } catch (...) { pthread_mutex_unlock(&_bufferLock); throw; }
    }
  }


  inline DSOTrigger trigger() const { return _trigger; }
  void setTrigger(DSOTrigger trigger) { _trigger = trigger; }

  inline Scalar base() const { return _base; }
  void setBase(Scalar base) { _base = base; }

  inline Scalar threshold() const { return _threshold; }
  void setThreshold(Scalar thres) { _threshold = thres; }

  inline double timeFrame() const { return _timeFrame; }

  /** Do not call this function while the Queue is running. */
  void setTimeFrame(double time) {
    _timeFrame = time;
    if (0 != _Fs) {
      pthread_mutex_lock(&_bufferLock);
      // Estimate minimum # samples to match time frame
      _N  = _timeFrame*_Fs;
      // Ensure N >= 2
      if (2 > _N) { _N = 2; }
      // Allocate ring-buffer
      _buffer = sdr::RingBuffer<Scalar>(2*_N);
      // Allocate output buffer
      _outBuffer = sdr::Buffer<Scalar>(_N);
      pthread_mutex_unlock(&_bufferLock);
      // Reconfigure
      this->setConfig(sdr::Config(sdr::Traits<Scalar>::scalarId, _Fs, _N, 1));
    }
  }

  /** Returns the mean of the inter-event times. */
  double avgPeriod() const {
    double period = 0;
    for (size_t i=0; i<_eventTimes.size(); i++) {
      period += _eventTimes[i];
    }
    return period/_eventTimes.size();
  }

  size_t eventCount() const {
    return _eventTimes.size();
  }

  /** Estimates the rate statistics, by assuming a Gamma distribution of the rate
   * parameter of an exponential distribution of inter-event times. (Conjugate prior of a
   * exponential distribution in the limes of an uninformative prior).
   * Retruns mean, upper and lower confidence interval (95%) */
  void rateStatistics(double &mean, double &lower, double &upper) const {
    double sumPeriod = 0;
    if (0 == _eventTimes.size()) { mean = lower = upper = 0; return; }
    for (size_t i=0; i<_eventTimes.size(); i++) { sumPeriod += _eventTimes[i]; }
    double stdRate  = std::sqrt(_eventTimes.size())/sumPeriod;
    mean = double(_eventTimes.size())/sumPeriod;
    lower = mean - 2*stdRate;
    upper = mean + 2*stdRate;
  }

  /** Estimates the statistics of the period, by assuming a Gamma distribution of the rate
   * parameter of an exponential distribution of inter-event times. (Conjugate prior of a
   * exponential distribution in the limes of an uninformative prior).
   * retuns mean, upper and lower confidence interval (95%) */
  void periodStatistics(double &mean, double &lower, double &upper) const {
    if (0 == _eventTimes.size()) { mean = lower = upper = 0; return; }
    rateStatistics(mean, lower, upper);
    std::swap(lower, upper);
    mean = 1./mean; lower = 1./lower; upper = 1./upper;
  }

  /** Resets the inter-event times statistics. */
  void resetStatistics() {
    _eventTimesIdx = 0;
    _eventTimes.clear();
  }

protected:
  void _checkTrigger() {
    if (TRIGGER_NONE == _trigger) {
      // Simply send the complete buffer
      _buffer.take(_outBuffer, _N);
      this->send(_outBuffer, true);
    }
    else if (TRIGGER_RISING == _trigger) {
      for (size_t i=1; i<_N; i++) {
        if ((_buffer[_N/2+i-1] < _base) && (_buffer[_N/2+i] > _threshold)) {
          // Drop the first i values, such that the values that triggered are in the center
          // of the output buffer:
          _buffer.drop(i-1);
          _sampleCount += i-1+_N/2;
          if (_eventTimesIdx >= _eventTimes.size()) {
            if (_eventTimes.size() == _eventTimes.capacity()) {
              _eventTimesIdx = 0;
              _eventTimes[_eventTimesIdx] = double(_sampleCount)/_Fs; _sampleCount=0;
            } else {
              _eventTimesIdx++;
              _eventTimes.push_back(double(_sampleCount)/_Fs); _sampleCount=0;
            }
          } else {
            _eventTimes[_eventTimesIdx] = double(_sampleCount)/_Fs; _sampleCount=0;
            _eventTimesIdx = (_eventTimesIdx+1)%_eventTimes.size();
          }
          // Take _N values from the buffer, the values that triggered are now at _N/2
          _buffer.take(_outBuffer, _N);
          // and send this buffer
          this->send(_outBuffer, true);
          return;
        }
      }
      // If no value triggered, drop _N values and continue
      _buffer.drop(_N);
      _sampleCount += _N;
    }
    else if (TRIGGER_FALLING == _trigger) {
      for (size_t i=1; i<_N; i++) {
        if ((_buffer[_N/2+i-1] > _base) && (_buffer[_N/2+i] < _threshold)) {
          // Drop the first i values, such that the values that triggered are in the center
          // of the output buffer:
          _buffer.drop(i-1);
          _sampleCount += i-1+_N/2;
          if (_eventTimesIdx >= _eventTimes.size()) {
            if (_eventTimes.size() == _eventTimes.capacity()) {
              _eventTimesIdx = 0;
              _eventTimes[_eventTimesIdx] = double(_sampleCount)/_Fs; _sampleCount=0;
            } else {
              _eventTimesIdx++;
              _eventTimes.push_back(double(_sampleCount)/_Fs); _sampleCount=0;
            }
          } else {
            _eventTimes[_eventTimesIdx] = double(_sampleCount)/_Fs; _sampleCount=0;
            _eventTimesIdx = (_eventTimesIdx+1)%_eventTimes.size();
          }
          // Take _N values from the buffer
          _buffer.take(_outBuffer, _N);
          // and send this buffer
          this->send(_outBuffer, true);
          return;
        }
      }
      // If no value triggered, drop _N values and continue
      _buffer.drop(_N);
      _sampleCount += _N;
    }
    else if (TRIGGER_EXCEED == _trigger) {
      for (size_t i=1; i<_N; i++) {
        if (((_buffer[_N/2+i-1] > _base) && (_buffer[_N/2+i] < _base)) ||
            ((_buffer[_N/2+i-1] < _threshold) && (_buffer[_N/2+i] > _threshold))) {
          // Drop the first i values, such that the values that triggered are in the center
          // of the output buffer:
          _buffer.drop(i-1);
          _sampleCount += i-1+_N/2;
          if (_eventTimesIdx >= _eventTimes.size()) {
            if (_eventTimes.size() == _eventTimes.capacity()) {
              _eventTimesIdx = 0;
              _eventTimes[_eventTimesIdx] = double(_sampleCount)/_Fs; _sampleCount=0;
            } else {
              _eventTimesIdx++;
              _eventTimes.push_back(double(_sampleCount)/_Fs); _sampleCount=0;
            }
          } else {
            _eventTimes[_eventTimesIdx] = double(_sampleCount)/_Fs; _sampleCount=0;
            _eventTimesIdx = (_eventTimesIdx+1)%_eventTimes.size();
          }
          // Take _N values from the buffer
          _buffer.take(_outBuffer, _N);
          // and send this buffer
          this->send(_outBuffer, true);
          return;
        }
      }
      // If no value triggered, drop _N values and continue
      _buffer.drop(_N);
      _sampleCount += _N;
    }
    else {
      sdr::RuntimeError err;
      err << "Can not process data: Unknwon trigger type: " << _trigger;
      throw err;      
    }
  }


protected:
  DSOTrigger _trigger;
  double _timeFrame;
  Scalar _base;
  Scalar _threshold;
  double _Fs;
  size_t _N;
  /** Collects the last event times. */
  std::vector<double> _eventTimes;
  /** Index of the next event time. */
  size_t _eventTimesIdx;
  /** Sample count since last trigger. */
  size_t _sampleCount;
  sdr::RingBuffer<Scalar> _buffer;
  sdr::Buffer<Scalar> _outBuffer;
  pthread_mutex_t _bufferLock;
};



/** Writes the received buffers line-by-line into a text file in CSV format with tab delimiter. */
template <class Scalar>
class DSOCSVRecorder: public sdr::Sink<Scalar>
{
public:
  DSOCSVRecorder()
    : sdr::Sink<Scalar>(), _file() {
    pthread_mutex_init(&_file_lock, 0);
  }

  DSOCSVRecorder(const std::string &filename)
    : sdr::Sink<Scalar>(), _file() {
    _file.open(filename.c_str(), std::fstream::out|std::fstream::trunc);
    pthread_mutex_init(&_file_lock, 0);
  }

  virtual ~DSOCSVRecorder() {
    if (_file.is_open()) {
      _file.close();
    }
    pthread_mutex_destroy(&_file_lock);
  }

  inline bool isOpen() const {
    return _file.is_open();
  }

  inline void close() {
    _file.close();
  }

  void open(const std::string &filename) {
    // get file lock
    pthread_mutex_lock(&_file_lock);
    if (_file.is_open()) {
      _file.close();
    }
    _file.open(filename.c_str(), std::fstream::out|std::fstream::trunc);
    pthread_mutex_unlock(&_file_lock);
  }

  virtual void config(const sdr::Config &src_cfg) {
    // Requires type, sample-rate and buffer size
    if (!src_cfg.hasType() || !src_cfg.hasSampleRate() || !src_cfg.hasBufferSize()) { return; }
    // Check buffer type
    if (sdr::Config::typeId<Scalar>() != src_cfg.type()) {
      sdr::ConfigError err;
      err << "Can not configure DSOCSVRecorder: Invalid type " << src_cfg.type()
          << ", expected " << sdr::Config::typeId<Scalar>();
      throw err;
    }
    // done.
  }

  virtual void process(const sdr::Buffer<Scalar> &buffer, bool allow_overwrite) {
    if (!_file.is_open()) { return; }
    if (0 == buffer.size()) { return; }
    pthread_mutex_lock(&_file_lock);
    for (size_t i=0; i<buffer.size()-1; i++) {
      _file << buffer[i] << "\t";
    }
    _file << buffer[buffer.size()-1] << std::endl;
    pthread_mutex_unlock(&_file_lock);
  }

protected:
  std::ofstream _file;
  pthread_mutex_t _file_lock;
};

#endif // __SDR_DSO_DSOBUFFER_HH__
