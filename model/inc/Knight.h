#ifndef KNIGHT_H
#define KNIGHT_H

#include "Piece.h"

class Knight final : public Piece
{
public:
	using Piece::Piece;
	std::set<BoardPosition> GetValidMove(const ChessBoard & board) const override;
};

#endif
