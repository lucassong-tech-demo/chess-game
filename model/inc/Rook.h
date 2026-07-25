#ifndef ROOK_H
#define ROOK_H

#include "Piece.h"



class Rook : public Piece
{

private:
	std::set<BoardPosition> * movements;

public:
	Rook(PieceType type, PieceColor color, int row, int column): Piece(type, color, row, column) {
		movements = '\0';
	}

	~Rook(){
		if (movements != NULL)
			delete movements;
		//std::cout<<"Rook destructor.."<<std::endl;
	}

	void Forward(ChessBoard * board);
	
	void Right(ChessBoard * board);

	void Left(ChessBoard * board);

	void Back(ChessBoard * board);

	

	/**
	 * Find out the available paths (cells). Encapsulate each cell location 
	 * into a BoardPosition class. Put all these BoardPosition object
	 * into a set so that ChessController and Highlight them.
	 */
	std::set<BoardPosition> & GetValidMove(ChessBoard * board);



};

#endif
