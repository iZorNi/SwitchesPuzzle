#include "historywidget.h"
#include "switchespuzzle.h"
#include <QUndoStack>
#include <QUndoView>
#include <QHBoxLayout>

HistoryWidget::HistoryWidget(QWidget *parent)
	: QWidget(parent)
{
	_stack = new QUndoStack(this);
	_stack->setUndoLimit(100);
	_view = new QUndoView(_stack, this);
	_view->setEmptyLabel("New game");
	auto mainLayout = new QHBoxLayout;
	mainLayout->addWidget(_view);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	setLayout(mainLayout);
	setWindowTitle("History");
}

void HistoryWidget::addCommand(SwitchCommand* command)
{
	_stack->push(command);
}

void HistoryWidget::clear()
{
	_stack->clear();
	SwitchCommand::resetCounter();
}

qint32 SwitchCommand::commandsCount = 0;

SwitchCommand::SwitchCommand(qint32 row, qint32 column, SwitchesPuzzle* executor)
	: _executor(executor), _row(row), _column(column)
{
	setText("Step " + QString::number(++commandsCount));
}

void SwitchCommand::undo()
{
	if (_executor)
	{
		_executor->undoSwitchActivation(_row, _column);
	}
}

void SwitchCommand::redo()
{
	if (_executor)
	{
		_executor->redoSwitchActivation(_row, _column);
	}
}
