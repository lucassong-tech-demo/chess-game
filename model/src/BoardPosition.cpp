#include "BoardPosition.h"

#include <stdexcept>

BoardPosition::BoardPosition(int position_row, int position_column)
	: row(position_row),
	  column(position_column)
{
	if (row < 0 || row >= 8 || column < 0 || column >= 8) {
		throw std::out_of_range("board coordinates must be between 0 and 7");
	}
}


bool BoardPosition::operator <(const BoardPosition & other)const {
	if (row < other.row)
		return true;
	else if (row == other.row && column < other.column)
		return true;
	return false;
}


int BoardPosition::GetRow()const {
	return row;
}

int BoardPosition::GetColumn()const {
	return column;
}
