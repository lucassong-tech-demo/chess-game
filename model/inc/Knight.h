#ifndef KNIGHT_H
#define KNIGHT_H

#include "Piece.h"

class Knight : public Piece
{

private:
	std::set<BoardPosition> * movements;

public:
	Knight(PieceType type, PieceColor color, int row, int column): Piece(type, color, row, column){
		movements = '\0';	
	}

	~Knight(){
		if (movements != NULL)
			delete movements;
		//std::cout<<"Knight destructor.."<<std::endl;
	}

	void Forward_left(ChessBoard * board);

	void Forward_right(ChessBoard * board);

	void Left_forward(ChessBoard * board);

	void Right_forward(ChessBoard * board);

	void Back_left(ChessBoard * board);

	void Back_right(ChessBoard * board);

	void Left_back(ChessBoard * board);

	void Right_back(ChessBoard * board);
	/**
	 * Find out the available paths (cells). Encapsulate each cell location 
	 * into a BoardPosition class. Put all these BoardPosition object
	 * into a set so that ChessController and Highlight them.
	 */
	std::set<BoardPosition> & GetValidMove(ChessBoard * board);

};

#endif
