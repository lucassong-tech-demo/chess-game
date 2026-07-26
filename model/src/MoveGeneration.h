#ifndef MOVE_GENERATION_H
#define MOVE_GENERATION_H

#include <set>

#include "BoardPosition.h"
#include "ChessBoard.h"
#include "Piece.h"

inline void AddMoveIfAvailable(
	std::set<BoardPosition> & moves,
	const ChessBoard & board,
	PieceColor color,
	int row,
	int col)
{
	if (!ChessBoard::IsValidPosition(row, col)) {
		return;
	}
	const Piece * occupant = board.GetPiece(row, col);
	if (!occupant || occupant->GetColor() != color) {
		moves.emplace(row, col);
	}
}

inline void AddRay(
	std::set<BoardPosition> & moves,
	const ChessBoard & board,
	PieceColor color,
	int row,
	int col,
	int row_step,
	int col_step)
{
	for (int next_row = row + row_step, next_col = col + col_step;
		 ChessBoard::IsValidPosition(next_row, next_col);
		 next_row += row_step, next_col += col_step) {
		const Piece * occupant = board.GetPiece(next_row, next_col);
		if (occupant) {
			if (occupant->GetColor() != color) {
				moves.emplace(next_row, next_col);
			}
			break;
		}
		moves.emplace(next_row, next_col);
	}
}

#endif
