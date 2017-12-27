#pragma once

#include <QtWidgets/QMainWindow>

class QTimer;
class SwitchWidget;

class SwitchesPuzzle : public QWidget
{
	Q_OBJECT

public:
	SwitchesPuzzle(qint32 rows, qint32 columns, QWidget* parent = 0);
	SwitchesPuzzle(const QStringList& config, QWidget* parent = 0);
	void		undoSwitchActivation(qint32 row, qint32 column);
	void		redoSwitchActivation(qint32 row, qint32 column);
	QStringList getConfiguration() const;
	qint32		fieldSize() const { return _rows; }
signals:
	void		completed();
	void		activated(qint32 row, qint32 column);

private slots:
	void		switchActivated(qint32 row, qint32 column);
	void		rotationFinished(qint32 row, qint32 column, qint32 destRow, qint32 destColumn);
	void		rotationFinished();

private:
	QList<QList<SwitchWidget*>> _switches;
	qint32		_rows{ 0 };
	qint32		_columns{ 0 };
	qint32		_rotationsNumber{ 0 };
	QTimer*		 _timer{ nullptr };

	void		activateSwitch(qint32 row, qint32 column);
	void		init();
	void		generateWidgets(qint32 rows, qint32 columns);
	void		formLayout();
	bool		isFinished() const;
	void		loadFromConfig(const QStringList& config);
	void		generateRandomInitialState();
	void		accelerate();
	void		changeStates(qint32 row, qint32 column);
};
