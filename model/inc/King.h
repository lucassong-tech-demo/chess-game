#ifndef KING_H
#define KING_H

#include <string>
#include "Piece.h"



class King : public Piece
{

private:
	std::set<BoardPosition> * movements;

public:
	King(PieceType type, PieceColor color, int row, int column): Piece(type, color, row, column){
		movements = '\0';
	}

	~King(){
		if (movements != NULL)
			delete movements;
		//std::cout<<"King destructor.."<<std::endl;
	}

	void Forward(ChessBoard * board);

	void Forward_left(ChessBoard * board);

	void Forward_right(ChessBoard * board);

	void Right(ChessBoard * board);

	void Left(ChessBoard * board);

	void Back(ChessBoard * board);

	void Back_left(ChessBoard * board);

	void Back_right(ChessBoard * board);


	/**
	 * Find out the available paths (cells). Encapsulate each cell location 
	 * into a BoardPosition class. Put all these BoardPosition object
	 * into a set so that ChessController and Highlight them.
	 */
	std::set<BoardPosition> & GetValidMove(ChessBoard * board);



};

#endif
