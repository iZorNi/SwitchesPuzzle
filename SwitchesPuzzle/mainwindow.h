#pragma once
#include <QMainWindow>
#include <QTime>

class QPushButton;
class QSpinBox;
class SwitchesPuzzle;
class Leaderboard;
class HistoryWidget;
class QLabel;
class QTimer;

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget* parent = 0);

private slots:
	void			startNewGame();
	void			finishGame();
	void			updateTimerLabel();
	void			addCommand(qint32 row, qint32 column);
	void			saveConfig();
	void			loadConfig();

private:
	QPushButton*	_newGamePushButton{ nullptr };
	QSpinBox*		_fieldSizeSpinBox{ nullptr };
	SwitchesPuzzle* _puzzle{ nullptr };
	Leaderboard*	_leaders{ nullptr };
	HistoryWidget*	_history{ nullptr };
	QLabel*			_timerLabel{ nullptr };
	QTimer*			_timer{ nullptr };
	QTime			_time;

	void			initWidgets();
	void			initField(const QStringList& config = QStringList());
	void			formGameOptions();
	void			formHistoryDock();
	void			startGame(const QStringList& config);
	void			reset();

	void			saveConfigToFile(const QString& filePath, const QStringList& config);
	QStringList		loadConfigFromFile(const QString& filePath);
};
