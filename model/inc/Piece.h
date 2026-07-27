#ifndef PIECE_H
#define PIECE_H

#include <set>
#include <string>

#include "BoardPosition.h"

enum PieceType {
	KING, QUEEN, KNIGHT, BISHOP, ROOK, PAWN
};

enum PieceColor {
	WHITE, BLACK
};

class ChessBoard;

class Piece
{
public:
	Piece(PieceType type, PieceColor color, int row, int column);
	virtual ~Piece() = default;
	Piece(const Piece &) = delete;
	Piece & operator=(const Piece &) = delete;
	Piece(Piece &&) = delete;
	Piece & operator=(Piece &&) = delete;

	PieceType GetType() const noexcept;
	PieceColor GetColor() const noexcept;
	std::string Type_String() const;
	std::string Color_String() const;
	int GetRow() const noexcept;
	int GetColumn() const noexcept;
	void Update_location(int row, int col);

	// Move sets are independent values. Callers never observe piece-owned storage.
	virtual std::set<BoardPosition> GetValidMove(const ChessBoard & board) const = 0;

protected:
	PieceType type;
	PieceColor color;
	int row;
	int column;
};

#endif
