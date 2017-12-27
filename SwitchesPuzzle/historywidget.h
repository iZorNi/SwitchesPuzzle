#pragma once
#include <QWidget>
#include <QUndoCommand>
class QUndoView;
class QUndoStack;
class SwitchesPuzzle;
class SwitchCommand;

class HistoryWidget : public QWidget
{
	Q_OBJECT

public:
	HistoryWidget(QWidget *parent);
	void addCommand(SwitchCommand* command);
	void clear();
private:
	QUndoView* _view{ nullptr };
	QUndoStack* _stack{ nullptr };
};

class SwitchCommand : public QUndoCommand
{
public:
	SwitchCommand(qint32 row, qint32 column, SwitchesPuzzle* executor);
	void undo() override;
	void redo() override;
	static void resetCounter() { commandsCount = 0; }
private:
	qint32 _row;
	qint32 _column;
	SwitchesPuzzle* _executor{ nullptr };
	static qint32 commandsCount;
};