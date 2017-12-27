#include "switchwidget.h"
#include <QPainter>
#include <QMouseEvent>
#include <QPixmap>

#include <QTime>

static const qint32 Height = 50;
static const qint32 Width = 50;
static const qint32 RotationRate = 10;
static const qint32 RotationStep = 90;
static const qint32 FullCircle = 360;
static const qint32 PictureWidth = 36;
static const qint32 PictureHeight = 20;
static const qint32 RotationInterval = 20;

static const QPixmap SwitchPixmap()
{
	static QPixmap pixmap(":/SwitchesPuzzle/Resources/thumbler.png");
	return pixmap;
}

static QTime* newTime()
{
	auto time = new QTime;
	time->start();
	return time;
}

QTime* SwitchWidget::_time = newTime();

SwitchWidget::SwitchWidget(qint32 row, qint32 column, QWidget *parent)
	: QWidget(parent)
	, _row(row)
	, _column(column)
{
	setMinimumSize(sizeHint());
}

void SwitchWidget::addRotation(qint32 destRow, qint32 destColumn)
{
	if (_rotations.isEmpty())
	{
		_destAngle += RotationStep;
		_destAngle %= FullCircle;
		_nextRotationTime = getTime() + RotationInterval;
	}
	_rotations.push_back(qMakePair(destRow, destColumn));
}

void SwitchWidget::changeState()
{
	_state = _state == SwitchState::Vertical ? SwitchState::Horizontal : SwitchState::Vertical;
}

void SwitchWidget::initState(SwitchState state)
{
	_state = state;
	switch (state)
	{
	case SwitchWidget::SwitchState::Vertical:
		_destAngle = 90;
		_lastAngle = 90;
		break;
	case SwitchWidget::SwitchState::Horizontal:
		_destAngle = 0;
		_lastAngle = 0;
		break;
	default:
		break;
	}
}

void SwitchWidget::accelerate()
{
	_rotations.clear();
	initState(_state);
}

QSize SwitchWidget::sizeHint() const
{
	return QSize(Width, Height);
}

QSize SwitchWidget::minimumSizeHint() const
{
	return sizeHint();
}

void SwitchWidget::paintEvent(QPaintEvent* event)
{
	static const QRect rect(-PictureWidth / 2, -PictureHeight / 2, PictureWidth, PictureHeight);
	rotate();
	QPainter painter(this);
	painter.setRenderHint(QPainter::SmoothPixmapTransform);
	painter.translate(Width / 2, Height / 2);
	painter.rotate(_lastAngle);
	painter.drawPixmap(rect, SwitchPixmap());
}

void SwitchWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		emit activated(_row, _column);
	}
}

qint32 SwitchWidget::getTime()
{
	return _time->elapsed();
}

void SwitchWidget::rotate()
{
	if (getTime() < _nextRotationTime)
	{
		return;
	}
	_nextRotationTime += RotationInterval;
	if (_lastAngle != _destAngle)
	{
		_lastAngle += RotationRate;
		_lastAngle %= FullCircle;
	}
	if(_lastAngle == _destAngle)
	{
		if (!_rotations.isEmpty())
		{
			auto dest = _rotations.front();
			_rotations.pop_front();
			if (dest.first != _row || dest.second != _column)
			{
				emit rotationFinished(_row, _column, dest.first, dest.second);
			}
			else
			{
				emit rotationFinished(); 
			}
			if (!_rotations.isEmpty())
			{
				_destAngle += RotationStep;
				_destAngle %= FullCircle;

			}
		}
	}
}
