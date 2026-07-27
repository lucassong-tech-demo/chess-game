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
	BoardPosition(int r, int col);

	bool operator <(const BoardPosition & other)const;
	bool operator==(const BoardPosition & other) const = default;
	
	int GetRow()const;

	int GetColumn()const;

};

#endif
