#pragma once

#include <QDialog>
#include <QList>
#include <QTableWidget>

class QPushButton;

class Leaderboard : public QDialog
{
	Q_OBJECT

public:
	Leaderboard(qint32 size, QWidget *parent);
	~Leaderboard();
	bool			isLeader(qint32 time) const;
	void			addResult(qint32 time);
	void			removeResult();

public slots:
	void			refreshLeaderboard(qint32 size);

private slots:
	void			applyChanges();

private:
	QTableWidget*					_board{ nullptr };
	QPushButton*					_button{ nullptr };
	QList<QPair<qint32, QString>>	_data;
	qint32							_editedRow{ -1 };
	qint32							_size{ 4 };
	
	void			initTableWidget();
	void			formLayout();
	void			fillTable();
	void			saveLeaders();
	void			loadLeaders(qint32 size);
	void			setRow(qint32 row, const QPair<qint32, QString> data);
	
	class TimeItem : public QTableWidgetItem
	{
	public:
		TimeItem(qint32 time) : _time(time) {};
		QVariant data(int role) const override;
		void	setData(int role, const QVariant & value) override;
	private:
		qint32	_time{ -1 };
	};

	enum Columns
	{
		TimeColumn = 0,
		NameColumn = 1
	};
};
