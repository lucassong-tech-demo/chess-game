#ifndef PIECE_H
#define PIECE_H

#include <iostream>
#include <string>
#include <set>
#include <cassert>
#include "BoardPosition.h"

enum PieceType {
	KING, QUEEN, KNIGHT, BISHOP, ROOK, PAWN
};

enum PieceColor {
	WHITE, BLACK
};

class ChessBoard;
/**
 * Piece class is a super class all kinds of Piece. It has six differenct subclasses
 * , such as King subtype class. Pawn subtype class, Bishop subtype class, and so on.
 * All these subtype classes inheret from Piece class directly.
 * Piece class encapulates a piece' type, color, position on the board.
 * It also can finds out the available paths depending on the rules of each piece. 
 */
class Piece
{
protected:
	PieceType type;  // the type of Piece
	PieceColor color; // color: white/black
	int row;
	int column;

public:
	Piece(PieceType type, PieceColor color, int row, int column);

	virtual ~Piece(){}

	PieceType GetType() const;

	PieceColor GetColor() const;

	std::string Type_String();

	std::string Color_String();

	int GetRow() const;

	int GetColumn() const;

	void Update_location(const int r, const int col);
	/**
	 * Find out the available paths (cells). Encapsulate each cell location 
	 * into a BoardPosition class. Put all these BoardPosition object
	 * into a set so that ChessController and Highlight them.
	 */
	virtual std::set<BoardPosition> & GetValidMove(ChessBoard * board) = 0; 



};

#endif

