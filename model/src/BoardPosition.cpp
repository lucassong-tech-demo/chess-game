#include "BoardPosition.h"

BoardPosition::BoardPosition(const BoardPosition & other) {
	row = other.row;
	column = other.column;
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
