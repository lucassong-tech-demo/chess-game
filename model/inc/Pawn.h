#ifndef PAWN_H
#define PAWN_H

#include "Piece.h"

class Pawn final : public Piece
{
public:
	using Piece::Piece;
	std::set<BoardPosition> GetValidMove(const ChessBoard & board) const override;
};

#endif
