#ifndef __SDR_DSO_PLOTWIDGET_HH__
#define __SDR_DSO_PLOTWIDGET_HH__

#include <QWidget>
#include <node.hh>
#include <pthread.h>
#include "dso.hh"


/** Receives time-series from a @c DSOBuffer and collects them for plotting. */
class PlotBuffer : public QObject, public sdr::Sink<int16_t>
{
  Q_OBJECT

public:
  PlotBuffer(size_t Nbuf, DSO *dso, QObject *parent=0);
  virtual ~PlotBuffer();

  virtual void config(const sdr::Config &src_cfg);
  virtual void process(const sdr::Buffer<int16_t> &buffer, bool allow_overwrite);

  inline const sdr::Buffer<int16_t> &buffer(size_t idx) {
    return _buffers[idx];
  }

  inline size_t numBuffers() const { return _buffers.size(); }
  void setNumBuffers(size_t N);

  inline size_t lastBuffer() const { return _lastBuffer; }

  inline int32_t scale() const { return _scale; }
  inline void setScale(int32_t scale) { _scale = scale; }

  inline void triggerUpdate() { emit buffersUpdated(); }

  inline double timeFrame() const { return _bufferSize/_sampleRate; }
  inline int16_t threshold() const { return _dso->threshold(); }

  inline const DSO *dso() const { return _dso; }

  void reset();

signals:
  void buffersUpdated();

protected:
  DSO *_dso;
  QVector< sdr::Buffer<int16_t> > _buffers;
  pthread_mutex_t _buffers_lock;
  size_t _lastBuffer;
  size_t _bufferSize;
  double _sampleRate;
  int32_t _scale;
};


/** Draws a collection of buffers received from a @c DSOBuffer. */
class PlotWidget : public QWidget
{
  Q_OBJECT

public:
  explicit PlotWidget(PlotBuffer *buffer, QWidget *parent = 0);
  virtual ~PlotWidget();

protected slots:
  void onUpdatePlot();

protected:
  void paintEvent(QPaintEvent *evt);
  void _drawAxes(QPainter &painter);
  void _drawGraphs(QPainter &painter);
  void _drawGraph(const sdr::Buffer<int16_t> &buffer, QPainter &painter);

protected:
  PlotBuffer *_buffer;
};

#endif // __SDR_DSO_PLOTWIDGET_HH__
