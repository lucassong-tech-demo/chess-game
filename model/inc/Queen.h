#ifndef QUEEN_H
#define QUEEN_H

#include "Piece.h"

class Queen final : public Piece
{
public:
	using Piece::Piece;
	std::set<BoardPosition> GetValidMove(const ChessBoard & board) const override;
};

#endif
