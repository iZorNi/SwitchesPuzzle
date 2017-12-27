#include "switchespuzzle.h"
#include <QGridLayout>
#include <QTimer>
#include "switchwidget.h"
#include <random>

static const qint32 UpdateInterval = 20;

SwitchesPuzzle::SwitchesPuzzle(qint32 rows, qint32 columns, QWidget *parent)
	: QWidget(parent)
	, _rows(rows)
	, _columns(columns)
{
	init();
	generateRandomInitialState();
}

SwitchesPuzzle::SwitchesPuzzle(const QStringList& config, QWidget* parent)
	: QWidget(parent)
	, _rows(config.size())
	, _columns(config.isEmpty() ? 0 : config.first().size())
{
	init();
	loadFromConfig(config);
}

void SwitchesPuzzle::activateSwitch(qint32 row, qint32 column)
{
	changeStates(row, column);
	_switches[row][column]->addRotation(-1, -1);
	update();
	if (_rotationsNumber == 0)
	{
		_timer->start(UpdateInterval);
	}
}

void SwitchesPuzzle::undoSwitchActivation(qint32 row, qint32 column)
{
	accelerate();
	changeStates(row, column);
	bool rotationToCenter = false;
	auto maxDist = std::max(_rows - row - 1, row);
	maxDist = std::max(maxDist, std::max(_columns - column - 1, column));
	if (row > 0)
	{
		auto destRow = (row == maxDist) ? row : row - 1;
		_switches[0][column]->addRotation(destRow, column);
		++_rotationsNumber;
		rotationToCenter = row == maxDist;
	}
	if (row < _rows - 1)
	{
		auto destRow = (!rotationToCenter && (_rows - row - 1 == maxDist)) ? row : row + 1;
		_switches[_rows - 1][column]->addRotation(destRow, column);
		++_rotationsNumber;
		rotationToCenter = (_rows - row - 1 == maxDist) || rotationToCenter;
	}
	if (column > 0)
	{
		auto destColumn = (!rotationToCenter && (column == maxDist)) ? column : column - 1;
		_switches[row][0]->addRotation(row, destColumn);
		rotationToCenter = rotationToCenter || (column == maxDist);
		++_rotationsNumber;
	}
	if (column < _columns - 1)
	{
		auto destColumn = (!rotationToCenter && (_columns - column - 1 == maxDist)) ? column : 
																					  column + 1;
		_switches[row][_columns - 1]->addRotation(row, destColumn);
		++_rotationsNumber;
	}
	update();
	if (_rotationsNumber <= 4)
	{
		_timer->start(UpdateInterval);
	}
}

void SwitchesPuzzle::redoSwitchActivation(qint32 row, qint32 column)
{
	accelerate();
	activateSwitch(row, column);
}

void SwitchesPuzzle::switchActivated(qint32 row, qint32 column)
{
	if (_rotationsNumber == 0 && !isFinished())
	{
		emit activated(row, column);
	}
}

QStringList SwitchesPuzzle::getConfiguration() const
{
	QStringList res;
	for (int i = 0; i < _rows; ++i)
	{
		QString row;
		for (int j = 0; j < _columns; ++j)
		{
			row.push_back(_switches[i][j]->currentState() == SwitchWidget::SwitchState::Vertical 
							? '0' : '1');
		}
		res.push_back(row);
	}
	return res;
}

void SwitchesPuzzle::rotationFinished(qint32 row, qint32 column, qint32 destRow, qint32 destColumn)
{
	if (destRow == -1 || destColumn == -1)
	{
		if (row > 0)
		{
			_switches[row - 1][column]->addRotation(0, column);
			++_rotationsNumber;
		}
		if (row < _rows - 1)
		{
			_switches[row + 1][column]->addRotation(_rows - 1, column);
			++_rotationsNumber;
		}
		if (column > 0)
		{
			_switches[row][column - 1]->addRotation(row, 0);
			++_rotationsNumber;
		}
		if (column < _columns - 1)
		{
			_switches[row][column + 1]->addRotation(row, _columns - 1);
			++_rotationsNumber;
		}
	}
	else
	{
		auto newRow = (destRow == row ? row : (destRow < row ? row - 1 : row + 1));
		auto newColumn = (destColumn == column ? column : 
												 (destColumn < column ? column - 1 : column + 1));
		_switches[newRow][newColumn]->addRotation(destRow, destColumn);
	}
	update();
}

void SwitchesPuzzle::rotationFinished() 
{
	--_rotationsNumber;
	if (_rotationsNumber == 0)
	{
		_timer->stop();
		if (isFinished())
		{
			emit completed();
		}
	}
}

void SwitchesPuzzle::init()
{
	generateWidgets(_rows, _columns);
	formLayout();
	_timer = new QTimer(this);
	connect(_timer, SIGNAL(timeout()), this, SLOT(update()));
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void SwitchesPuzzle::generateWidgets(qint32 rows, qint32 columns)
{
	_switches.reserve(rows);
	for (qint32 i = 0; i < rows; ++i)
	{
		_switches.push_back(QList<SwitchWidget*>());
		_switches[i].reserve(columns);
		for (qint32 j = 0; j < columns; ++j)
		{
			_switches[i].push_back(new SwitchWidget(i, j, this));
			connect(_switches[i][j], SIGNAL(rotationFinished(qint32, qint32, qint32, qint32)),
					this, SLOT(rotationFinished(qint32, qint32, qint32, qint32)));
			connect(_switches[i][j], SIGNAL(rotationFinished()),
					this, SLOT(rotationFinished()));
			connect(_switches[i][j], SIGNAL(activated(qint32, qint32)),
					this, SLOT(switchActivated(qint32, qint32)));
		}
	}
}

void SwitchesPuzzle::loadFromConfig(const QStringList& config)
{
	for (int i = 0; i < _rows; ++i)
	{
		for (int j = 0; j < _columns; ++j)
		{
			SwitchWidget::SwitchState state = config[i][j] == '0' ? 
											  SwitchWidget::SwitchState::Vertical : 
											  SwitchWidget::SwitchState::Horizontal;
			_switches[i][j]->initState(state);
		}
	}
}

void SwitchesPuzzle::formLayout()
{
	auto mainLayout = new QGridLayout();
	for (qint32 i = 0; i < _rows; ++i)
	{
		for (qint32 j = 0; j < _columns; ++j)
		{
			mainLayout->addWidget(_switches[i][j], i, j);
		}
	}
	mainLayout->setSpacing(15);
	setLayout(mainLayout);
}

bool SwitchesPuzzle::isFinished() const
{
	for (auto row : _switches)
	{
		for (auto elem : row)
		{
			auto res = (elem->currentState() != SwitchWidget::SwitchState::Vertical);
			if (!res)
			{
				return res;
			}
		}
	}
	return true;
}

void SwitchesPuzzle::generateRandomInitialState()
{
	static std::random_device generator;
	do 
	{
		for (auto row : _switches)
		{
			for (auto elem : row)
			{
				if (generator() % 2)
				{
					elem->initState(SwitchWidget::SwitchState::Horizontal);
				}
				else
				{
					elem->initState(SwitchWidget::SwitchState::Vertical);
				}
			}
		}
	} while (isFinished());
}

void SwitchesPuzzle::accelerate()
{
	_timer->stop();
	update();
	for (auto row : _switches)
	{
		for (auto elem : row)
		{
			elem->accelerate();
		}
	}
	_rotationsNumber = 0;
	update();
}

void SwitchesPuzzle::changeStates(qint32 row, qint32 column)
{
	for (qint32 i = 0; i < _rows; ++i)
	{
		_switches[i][column]->changeState();
	}
	for (qint32 i = 0; i < _columns; ++i)
	{
		if (i == column)
		{
			continue;
		}
		_switches[row][i]->changeState();
	}
}
