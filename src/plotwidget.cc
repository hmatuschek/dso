#include "plotwidget.hh"
#include <QPainter>
#include <QPaintEvent>


/* ******************************************************************************************** *
 * Implementation of PlotBuffer
 * ******************************************************************************************** */
PlotBuffer::PlotBuffer(size_t Nbuf, DSO *dso, QObject *parent)
  : QObject(parent), sdr::Sink<int16_t>(), _dso(dso), _buffers(std::max(size_t(1), Nbuf)), _lastBuffer(0),
    _bufferSize(0), _sampleRate(0), _scale(1)
{
  pthread_mutex_init(&_buffers_lock, NULL);
}

PlotBuffer::~PlotBuffer() {
  pthread_mutex_destroy(&_buffers_lock);
}

void
PlotBuffer::config(const sdr::Config &src_cfg) {
  // Requires type, sample rate & buffer size
  if (!src_cfg.hasType() || !src_cfg.hasSampleRate() || !src_cfg.hasBufferSize()) { return; }
  // Check buffer type
  if (sdr::Config::typeId<int16_t>() != src_cfg.type()) {
    sdr::ConfigError err;
    err << "Can not configure IQBaseBand: Invalid type " << src_cfg.type()
        << ", expected " << sdr::Config::typeId<int16_t>();
    throw err;
  }
  _bufferSize = src_cfg.bufferSize();
  _sampleRate = src_cfg.sampleRate();

  // Allocate and initialize buffers
  for (int i=0; i<_buffers.size(); i++) {
    _buffers[i] = sdr::Buffer<int16_t>(_bufferSize);
    for (size_t j=0; j<_bufferSize; j++) {
      _buffers[i][j] = 0;
    }
  }

  sdr::LogMessage msg(sdr::LOG_DEBUG);
  msg << "Configured PlotBuffer node:" << std::endl
      << " type " << sdr::Config::typeId<int16_t>() << std::endl
      << " sample-rate " << _sampleRate << "Hz" << std::endl
      << " # buffers " << _buffers.size() << std::endl
      << " buffer size " << _bufferSize << std::endl;
  sdr::Logger::get().log(msg);

  // Signal plotwidget to update the plot
  emit buffersUpdated();
}

void
PlotBuffer::process(const sdr::Buffer<int16_t> &buffer, bool allow_overwrite) {
  Q_UNUSED(allow_overwrite);
  pthread_mutex_lock(&_buffers_lock);
  _lastBuffer = (_lastBuffer+1)%_buffers.size();
  memcpy(_buffers[_lastBuffer].data(), buffer.data(), buffer.bytesLen());
  pthread_mutex_unlock(&_buffers_lock);
  emit buffersUpdated();
}


void
PlotBuffer::setNumBuffers(size_t N) {
  pthread_mutex_lock(&_buffers_lock);
  if (int(N) > _buffers.size()) {
    _buffers.reserve(N);
    for (int i=_buffers.size(); i<int(N); i++) {
      _buffers.append(sdr::Buffer<int16_t>(_bufferSize));
      for (size_t j=0; j<_bufferSize; j++) {
        _buffers[i][j] = 0;
      }
    }
  } else {
    _buffers.resize(N); _lastBuffer = (_lastBuffer % N);
  }
  pthread_mutex_unlock(&_buffers_lock);
}

void
PlotBuffer::reset() {
  pthread_mutex_lock(&_buffers_lock);
  // Set all buffers to 0:
  for (int i=0; i<_buffers.size(); i++) {
    for (size_t j=0; j<_bufferSize; j++) {
      _buffers[i][j] = 0;
    }
  }
  pthread_mutex_unlock(&_buffers_lock);
}



/* ******************************************************************************************** *
 * Implementation of PlotWidget
 * ******************************************************************************************** */
PlotWidget::PlotWidget(PlotBuffer *buffer, QWidget *parent) :
  QWidget(parent), _buffer(buffer)
{
  QObject::connect(_buffer, SIGNAL(buffersUpdated()), this, SLOT(onUpdatePlot()));

  this->setMinimumSize(480,480);
}

PlotWidget::~PlotWidget() {
  // pass...
}


void
PlotWidget::onUpdatePlot() {
  this->update();
}

void
PlotWidget::paintEvent(QPaintEvent *evt) {
  // First, paint widget background
  QWidget::paintEvent(evt);

  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);
  painter.setClipRect(evt->rect());
  painter.save();

  painter.fillRect(evt->rect(), Qt::white);

  _drawGraphs(painter);
  _drawAxes(painter);

  painter.restore();
}

void
PlotWidget::_drawAxes(QPainter &painter) {
  int32_t height = this->size().height();
  int32_t width = this->size().width();

  QPen pen(Qt::black);
  pen.setWidth(1); pen.setCosmetic(true);
  painter.setPen(pen);

  // Draw axes ticks:
  double dt = _buffer->timeFrame()/8;
  double t  = -_buffer->timeFrame()/2;

  QFont font; font.setPointSize(10);
  QFontMetrics fm(font);
  painter.setFont(font);

  // Draw threshold, if tigger != NONE
  if (TRIGGER_NONE != _buffer->dso()->trigger()) {
    // Draw base if != threshold
    if (_buffer->dso()->base() != _buffer->dso()->threshold()) {
      int y = height*(1 - _buffer->scale()*double(_buffer->dso()->base())/((1<<15)-1))/2;
      pen.setStyle(Qt::DotLine);
      painter.setPen(pen);
      painter.drawLine(0, y, width, y);
    }

    int y = height*(1 - _buffer->scale()*double(_buffer->dso()->threshold())/((1<<15)-1))/2;
    pen.setStyle(Qt::DashLine);
    painter.setPen(pen);
    painter.drawLine(0, y, width, y);
  }

  pen.setStyle(Qt::DashLine);
  painter.setPen(pen);
  for (size_t i=0; i<9; i++, t+=dt) {
    int x  = (i*width)/8;
    for (size_t j=0; j<9; j++) {
      int    y  = (j*height)/8;
      painter.drawPoint(x, y);
    }

    if ((0!=i)&&(8!=i)) {
      QString label = QString::number(t*1000, 'f', 2);
      QRectF bb = fm.boundingRect(label);
      painter.drawText(x-bb.width()/2, 3+fm.ascent(), label);
      painter.drawText(x-bb.width()/2, height-3-fm.descent(), label);
    }
  }

  double dv = 2./8, v  = 1-dv;
  for (size_t i=1; i<8; i++, v-=dv) {
    int y = (i*height)/8;
    QString label = QString::number(v/_buffer->scale(), 'g', 3);
    QRectF bb = fm.boundingRect(label);
    float shift = bb.height()/2 - fm.ascent();
    painter.drawText(3, y-shift, label);
    painter.drawText(width-bb.width()-3, y-shift, label);
  }
}


void
PlotWidget::_drawGraphs(QPainter &painter) {
  // Draw "old" graphs
  if (1 != _buffer->numBuffers()) {
    QPen pen(QColor(128,128,255));
    pen.setWidth(1); pen.setCosmetic(true);
    painter.setPen(pen);
    // Draw all graphs, except the last one
    for (size_t i=0; i<_buffer->numBuffers(); i++) {
      if (i == _buffer->lastBuffer()) { continue; }
      _drawGraph(_buffer->buffer(i), painter);
    }
  }

  // Draw new graph
  {
    QPen pen(QColor(0,0,255));
    pen.setWidth(1); pen.setCosmetic(true); painter.setPen(pen);
    _drawGraph(_buffer->buffer(_buffer->lastBuffer()), painter);
  }
}


void
PlotWidget::_drawGraph(const sdr::Buffer<int16_t> &buffer, QPainter &painter) {
  int32_t heigth = this->size().height();
  int32_t width = this->size().width();

  if (int(buffer.size())<=width) {
    // If there are less samples than pixels -> iterate over samples
    for (int32_t j=1; j<int(buffer.size()); j++) {
      int32_t x1 = ((j-1)*width)/buffer.size();
      int32_t y1 = heigth/2-((int32_t(buffer[j-1])*_buffer->scale()*heigth)>>16);
      int32_t x2 = (j*width)/buffer.size();
      int32_t y2 = heigth/2-((int32_t(buffer[j])*_buffer->scale()*heigth)>>16);
      painter.drawLine(x1,y1, x2,y2);
    }
  } else {
    // If there are more samples than pixels -> iterate over pixels
    for (int32_t j=1; j<width; j++) {
      int32_t i1 = ((j-1)*buffer.size())/width;
      int32_t y1 = (heigth/2) - ((int32_t(buffer[i1]*_buffer->scale())*heigth)>>16);
      int32_t i2 = (j*buffer.size())/width;
      int32_t y2 = (heigth/2) - ((int32_t(buffer[i2]*_buffer->scale())*heigth)>>16);
      painter.drawLine(j-1, y1, j, y2);
    }
  }
}
