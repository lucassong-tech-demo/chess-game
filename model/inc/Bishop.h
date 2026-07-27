#ifndef BISHOP_H
#define BISHOP_H

#include "Piece.h"

class Bishop final : public Piece
{
public:
	using Piece::Piece;
	std::set<BoardPosition> GetValidMove(const ChessBoard & board) const override;
};

#endif
