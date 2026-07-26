#ifndef ROOK_H
#define ROOK_H

#include "Piece.h"

class Rook final : public Piece
{
public:
	using Piece::Piece;
	std::set<BoardPosition> GetValidMove(const ChessBoard & board) const override;
};

#endif
