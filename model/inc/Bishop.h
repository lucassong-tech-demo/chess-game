#ifndef BISHOP_H
#define BISHOP_H

#include "Piece.h"



class Bishop : public Piece
{

private:
	std::set<BoardPosition> * movements;

public:
	Bishop(PieceType type, PieceColor color, int row, int column): Piece(type, color, row, column){
		movements = '\0';
	}

	~Bishop(){
		if (movements != NULL)
			delete movements;
	}

	void Forward_left(ChessBoard * board);

	void Forward_right(ChessBoard * board);

	void Back_left(ChessBoard * board);

	void Back_right(ChessBoard * board);

	bool inCheck_Filter();

	/**
	 * Find out the available paths (cells). Encapsulate each cell location 
	 * into a BoardPosition class. Put all these BoardPosition object
	 * into a set so that ChessController and Highlight them.
	 */
	std::set<BoardPosition> & GetValidMove(ChessBoard * board);
};

#endif
