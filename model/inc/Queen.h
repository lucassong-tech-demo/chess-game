#ifndef QUEEN_H
#define QUEEN_H

//#include <string>
//#include <set>
#include "Piece.h"
//#include "BoardPosition.h"



class Queen : public Piece
{
private:
	std::set<BoardPosition> * movements;

public:
	Queen(PieceType type, PieceColor color, int row, int column):Piece(type, color, row, column){
		movements = '\0';
	}

	~Queen(){
		if (movements != NULL)
			delete movements;
		//std::cout<<"Queue destructor.."<<std::endl;
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
