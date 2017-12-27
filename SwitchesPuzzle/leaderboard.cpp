#include "leaderboard.h"
#include <QSettings>
#include <QVBoxLayout>
#include <QPushButton>
#include <algorithm>
#include <QHeaderView>

static const QString SettingsString("Leaderboard%1");
static const QString FilePath("SwitchesPuzzleLeaders.ini");
static const QString TimeHeader("Time");
static const QString NameHeader("Name");
static const QString ButtonText("Apply");
static const QString WindowTitle("Leaderboard %1 x %1");

static const qint32 NumberOfEntries = 10;

static void sortData(QList<QPair<qint32, QString>>& data)
{
	static auto sortFunc = [](const QPair<qint32, QString>& a, const QPair<qint32, QString>& b)
	{
		return a.first < b.first;
	};
	std::sort(data.begin(), data.end(), sortFunc);
}

Leaderboard::Leaderboard(qint32 size, QWidget *parent)
	: QDialog(parent), _size(size)
{
	setWindowTitle(WindowTitle.arg(size));
	loadLeaders(size);
	initTableWidget();
	formLayout();
}

Leaderboard::~Leaderboard()
{
	saveLeaders();
}

bool Leaderboard::isLeader(qint32 time) const
{
	return _data.size() < NumberOfEntries || _data.last().first > time;
}

void Leaderboard::addResult(qint32 time)
{
	auto pair = qMakePair(time, QString());
	_data.push_back(pair);
	sortData(_data);
	_editedRow = _data.lastIndexOf(pair);
	if (_editedRow < NumberOfEntries)
	{
		fillTable();
		_button->show();
		_button->setDisabled(false);
		auto item = _board->item(_editedRow, NameColumn);
		item->setFlags(item->flags() | Qt::ItemIsEditable);
		_board->selectRow(_editedRow);
		_board->openPersistentEditor(item);
	}
}

void Leaderboard::removeResult()
{
	if (_editedRow >= 0 && _editedRow < _data.size())
	{
		_data.removeAt(_editedRow);
		auto item = _board->item(_editedRow, NameColumn);
		item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	}
	_editedRow = -1;
	fillTable();
	_button->hide();
	_button->setDisabled(true);
}

void Leaderboard::refreshLeaderboard(qint32 size)
{
	_size = size;
	setWindowTitle(WindowTitle.arg(size));
	loadLeaders(size);
	fillTable();
}

void Leaderboard::applyChanges()
{
	while (_data.size() > NumberOfEntries)
	{
		_data.pop_back();
	}
	auto item = _board->item(_editedRow, NameColumn);
	_board->closePersistentEditor(item);
	item->setFlags(item->flags() & ~Qt::ItemIsEditable);
	_data[_editedRow].second = item->text();
	_editedRow = -1;
	_button->hide();
	_button->setDisabled(true);
	saveLeaders();
	accept();
}

void Leaderboard::initTableWidget()
{
	_board = new QTableWidget(NumberOfEntries, 2, this);
	_board->setHorizontalHeaderItem(TimeColumn, new QTableWidgetItem(TimeHeader));
	_board->setHorizontalHeaderItem(NameColumn, new QTableWidgetItem(NameHeader));
	_board->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	_board->horizontalHeader()->setSectionResizeMode(TimeColumn, QHeaderView::ResizeToContents);
	_board->horizontalHeader()->setStretchLastSection(true);
	const auto minSize = std::min(_data.size(), NumberOfEntries);
	for (int i = 0; i < minSize; ++i)
	{
		setRow(i, _data[i]);
	}
	for (int i = minSize; i < NumberOfEntries; ++i)
	{
		setRow(i, qMakePair(-1, QString()));
	}
	_board->setMinimumHeight((NumberOfEntries + 1) * _board->verticalHeader()->defaultSectionSize());
}

void Leaderboard::formLayout()
{
	auto mainLayoiut = new QVBoxLayout;
	mainLayoiut->addWidget(_board);
	_button = new QPushButton(ButtonText, this);
	connect(_button, &QPushButton::pressed, this, &Leaderboard::applyChanges);
	mainLayoiut->addWidget(_button);
	setLayout(mainLayoiut);
	_button->hide();
}

void Leaderboard::fillTable()
{
	for (int i = 0; i < NumberOfEntries; ++i)
	{
		_board->item(i, TimeColumn)->setData(Qt::UserRole, -1);
		_board->item(i, NameColumn)->setText(QString());
	}
	auto rows = _board->rowCount();
	const auto minSize = std::min(_data.size(), NumberOfEntries);
	for (int i = 0; i < minSize; ++i)
	{
		_board->item(i, TimeColumn)->setData(Qt::UserRole, _data[i].first);
		_board->item(i, NameColumn)->setText(_data[i].second);
	}
}

void Leaderboard::saveLeaders()
{
	QSettings settings(FilePath, QSettings::IniFormat);
	QList<QVariant> leaders;
	for (auto iter : _data)
	{
		leaders.push_back(iter.first);
		leaders.push_back(iter.second);
	}
	settings.setValue(SettingsString.arg(_size), leaders);
}

void Leaderboard::loadLeaders(qint32 size)
{
	_data.clear();
	QSettings settings(FilePath, QSettings::IniFormat);
	auto list = settings.value(SettingsString.arg(size), QList<QVariant>()).toList();
	for (int i = 0; i < list.size(); ++i)
	{
		bool b;
		int time = list[i].toInt(&b);
		if (b && ++i < list.size())
		{
			QString name = list[i].toString();
			_data.push_back(qMakePair(time, name));
		}
		else
		{
			break;
		}
	}
	sortData(_data);
}

void Leaderboard::setRow(qint32 row, const QPair<qint32, QString> data)
{
	auto time = new TimeItem(data.first);
	time->setTextAlignment(Qt::AlignLeft);
	time->setFlags(time->flags() & ~Qt::ItemIsEditable);
	_board->setItem(row, TimeColumn, time);
	auto name = new QTableWidgetItem(data.second);
	name->setTextAlignment(Qt::AlignLeft);
	name->setFlags(name->flags() & ~Qt::ItemIsEditable);
	_board->setItem(row, NameColumn, name);
}

QVariant Leaderboard::TimeItem::data(int role) const
{
	switch (role)
	{
	case Qt::DisplayRole:
		return _time >= 0 ? QString("%1.%2").arg(_time / 1000).
											 arg(_time % 1000, 3, 10, QChar('0')) : QString();
	case Qt::UserRole:
		return _time;
	default:
		return QTableWidgetItem::data(role);
	}
}

void Leaderboard::TimeItem::setData(int role, const QVariant & value)
{
	switch (role)
	{
	case Qt::UserRole:
		_time = value.toInt();
		break;
	default:
		return QTableWidgetItem::setData(role, value);
	}
}