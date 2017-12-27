#include "mainwindow.h"
#include "switchespuzzle.h"
#include "leaderboard.h"
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenuBar>
#include <QDockWidget>
#include <QTimer>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include "historywidget.h"

static const QString NewGameText("New game");
static const QString WidthText("Width");
static const QString HeightText("Height");
static const QString SizeText("Field size");
static const QString Leaders("Leaderboard");
static const QString HistoryTitle("History");
static const QString TimerTitle("Your time: ");
static const QString SaveText("Save config");
static const QString LoadText("Load config");

MainWindow::MainWindow(QWidget* parent)
: QMainWindow(parent)
{
	setWindowIcon(QPixmap(":/SwitchesPuzzle/Resources/icon.png"));
	initWidgets();
	formGameOptions();
	formHistoryDock();
	_timer = new QTimer(this);
	_timer->setInterval(10);
	connect(_timer, &QTimer::timeout, this, &MainWindow::updateTimerLabel);
	startNewGame();
}

void MainWindow::startNewGame()
{
	initField();
	reset();
}

void MainWindow::startGame(const QStringList& config)
{
	initField(config);
	reset();
}

void MainWindow::reset()
{
	_history->clear();
	_history->setEnabled(true);
	_history->resize(_history->width(), centralWidget()->height());
	_timer->start();
	_time.restart();
}

void MainWindow::finishGame()
{
	auto roundTime = _time.elapsed();
	_timer->stop();
	_history->setDisabled(true); 
	_leaders->refreshLeaderboard(_puzzle->fieldSize());
	if (_leaders->isLeader(roundTime))
	{
		_leaders->addResult(roundTime);
		if (_leaders->exec() != QDialog::Accepted)
		{
			_leaders->removeResult();
		}
	}
	_leaders->refreshLeaderboard(_fieldSizeSpinBox->value());
}

void MainWindow::updateTimerLabel()
{
	auto time = _time.elapsed();
	QString text = QString("%1.%2").arg(time / 1000, 5, 10, QChar(' '));
	text = text.arg(((time % 1000) / 10), 2, 10, QChar('0'));
	_timerLabel->setText(text);
}

void MainWindow::addCommand(qint32 row, qint32 column)
{
	auto command = new SwitchCommand(row, column, _puzzle);
	_history->addCommand(command);
}

void MainWindow::saveConfigToFile(const QString& filePath, const QStringList& config)
{
	QFile outputFile(filePath);
	if (outputFile.open(QIODevice::WriteOnly))
	{
		QByteArray array;
		QDataStream output(&array, QIODevice::WriteOnly);
		for (int i = 0; i < config.size(); ++i)
		{
			for (int j = 0; j < config[i].size(); ++j)
			{
				output << config[i][j];
			}
			output << QChar(' ');
		}
		outputFile.write(array);
	}
	else
	{
		QMessageBox::critical(this, "Error", "Can not open the file");
	}
}

QStringList MainWindow::loadConfigFromFile(const QString& filePath)
{
	QStringList res;
	QFile inputFile(filePath);
	if (inputFile.open(QIODevice::ReadOnly))
	{
		QByteArray array = inputFile.read(100 * 4);
		QDataStream input(array);

		while (!input.atEnd())
		{
			QChar c;
			do
			{
				input >> c;
			} while (c != '1' && c != '0');
			QString row;
			while (c == '1' || c == '0')
			{
				row.push_back(c);
				input >> c;
			}
			res.push_back(row);
		}
	}
	else
	{
		QMessageBox::critical(this, "Error", "Can not  to load from file");
	}
	bool error = res.size() < 4;
	if (!error)
	{
		auto rowSize = res.first().size();
		for (int i = 1; i < res.size(); ++i)
		{
			error = res[i].size() != rowSize;
			if (error)
			{
				break;
			}
		}
	}
	if (error)
	{
		QMessageBox::critical(this, "Error", "Input data is invalid");
	}
	return !error ? res : QStringList();
}

void MainWindow::saveConfig()
{
	auto filePath = QFileDialog::getSaveFileName(this, "Save config");
	if (!filePath.isEmpty())
	{
		saveConfigToFile(filePath, _puzzle->getConfiguration());
	}
}

void MainWindow::loadConfig()
{
	auto filePath = QFileDialog::getOpenFileName(this);
	if (!filePath.isEmpty())
	{
		auto config = loadConfigFromFile(filePath);
		if (!config.isEmpty())
		{
			_fieldSizeSpinBox->setValue(config.size());
			startGame(config);
		}
	}
}

void MainWindow::initWidgets()
{
	_newGamePushButton = new QPushButton(NewGameText, this);
	connect(_newGamePushButton, &QPushButton::pressed, this, &MainWindow::startNewGame);
	_fieldSizeSpinBox = new QSpinBox(this);
	_fieldSizeSpinBox->setRange(4, 10);
	_fieldSizeSpinBox->setValue(4);
	_leaders = new Leaderboard(_fieldSizeSpinBox->value(), this);
	connect(_fieldSizeSpinBox, SIGNAL(valueChanged(qint32)), _leaders, SLOT(refreshLeaderboard(qint32)));
	auto menuBar = new QMenuBar(this);
	auto newGameAct = menuBar->addAction(NewGameText);
	auto leadersAct = menuBar->addAction(Leaders);
	auto saveAct = menuBar->addAction(SaveText);
	auto loadAct = menuBar->addAction(LoadText);
	setMenuBar(menuBar);
	connect(leadersAct, &QAction::triggered, _leaders, &QDialog::exec);
	connect(newGameAct, &QAction::triggered, this, &MainWindow::startNewGame);
	connect(saveAct, &QAction::triggered, this, &MainWindow::saveConfig);
	connect(loadAct, &QAction::triggered, this, &MainWindow::loadConfig);
	_history = new HistoryWidget(this);
	_timerLabel = new QLabel(this);
	_timerLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed);
}

void MainWindow::initField(const QStringList& config)
{
	delete _puzzle;
	if (config.isEmpty())
	{
		_puzzle = new SwitchesPuzzle(_fieldSizeSpinBox->value(), _fieldSizeSpinBox->value(), this);
	}
	else
	{
		_puzzle = new SwitchesPuzzle(config, this);
	}
	connect(_puzzle, &SwitchesPuzzle::completed, this, &MainWindow::finishGame);
	connect(_puzzle, &SwitchesPuzzle::activated, this, &MainWindow::addCommand);
	centralWidget()->layout()->addWidget(_puzzle);
}

void MainWindow::formGameOptions()
{
	auto widget = new QWidget(this);
	auto mainLayout = new QVBoxLayout;
	auto topLayout = new QHBoxLayout;
	topLayout->addWidget(_newGamePushButton);
	topLayout->addWidget(new QLabel(SizeText, widget));
	topLayout->addWidget(_fieldSizeSpinBox);
	topLayout->addWidget(new QLabel(TimerTitle, widget));
	topLayout->addWidget(_timerLabel);
	topLayout->setAlignment(Qt::AlignLeft);
	topLayout->setSpacing(5);
	mainLayout->addItem(topLayout);
	mainLayout->setAlignment(Qt::AlignTop);
	widget->setLayout(mainLayout);
	setCentralWidget(widget);
}

void MainWindow::formHistoryDock()
{
	auto dockWidget = new QDockWidget(HistoryTitle, this);
	dockWidget->setWidget(_history);
	dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, dockWidget);
}
