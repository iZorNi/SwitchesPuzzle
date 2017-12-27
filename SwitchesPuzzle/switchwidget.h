#pragma once

#include <QWidget>

class QTime;

class SwitchWidget : public QWidget
{
	Q_OBJECT

public:
	enum class SwitchState
	{
		Vertical,
		Horizontal
	};
	SwitchWidget(qint32 row, qint32 column, QWidget *parent);
	SwitchState currentState() const { return _state; }
	void		changeState();
	void		addRotation(qint32 destRow, qint32 destColumn);
	void		initState(SwitchState state);
	void		accelerate();
	QSize		sizeHint() const override;
	QSize		minimumSizeHint() const override;

signals:
	void		rotationFinished();
	void		rotationFinished(qint32 row, qint32 column, qint32 destRow, qint32 destColumn);
	void		activated(qint32 row, qint32 column);

protected:
	void		paintEvent(QPaintEvent* event) override;
	void		mousePressEvent(QMouseEvent* event) override;
	
private:
	SwitchState						_state{ SwitchState::Horizontal };
	QList<QPair<qint32, qint32>>	_rotations;
	qint32							_lastAngle{ 0 };
	qint32							_destAngle{ 0 };
	qint32							_row{ -1 };
	qint32							_column{ -1 };
	qint32							_nextRotationTime{ 0 };

	static QTime*					_time;

	static qint32					getTime();

	void							rotate();
};
