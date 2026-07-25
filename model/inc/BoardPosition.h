#ifndef BOARD_POSITION_H
#define BOARD_POSITION_H

/**
 *  BoardPosition class is used to encapsulate the position on the board.
 *  
 */
class BoardPosition
{
private:
	int row;
	int column;

public:
	BoardPosition(){}

	BoardPosition(int r, int col):row(r),column(col){}

	BoardPosition(const BoardPosition & other);

	bool operator <(const BoardPosition & other)const;
	
	int GetRow()const;

	int GetColumn()const;

};

#endif
