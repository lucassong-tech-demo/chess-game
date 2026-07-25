#ifndef PAWN_H
#define PAWN_H

#include "Piece.h"


class Pawn : public Piece
{

private:
	std::set<BoardPosition> * movements;

public:
	Pawn(PieceType type, PieceColor color, int row, int column): Piece(type, color, row, column){
		movements = '\0';
	}

	~Pawn(){
		if (movements != NULL)
			delete movements;	
		//std::cout<<"Pawn destructor.."<<std::endl;
	}

	void Forward(ChessBoard * board);

	void Forward_left(ChessBoard * board);

	void Forward_right(ChessBoard * board);

	/**
	 * Find out the available paths (cells). Encapsulate each cell location 
	 * into a BoardPosition class. Put all these BoardPosition object
	 * into a set so that ChessController and Highlight them.
	 */
	std::set<BoardPosition> & GetValidMove(ChessBoard * board);
	
};

#endif
