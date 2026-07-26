#ifndef KING_H
#define KING_H

#include "Piece.h"

class King final : public Piece
{
public:
	using Piece::Piece;
	std::set<BoardPosition> GetValidMove(const ChessBoard & board) const override;
};

#endif
