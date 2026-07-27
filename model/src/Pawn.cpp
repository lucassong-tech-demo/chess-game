#include "Pawn.h"

#include "ChessBoard.h"

std::set<BoardPosition> Pawn::GetValidMove(const ChessBoard & board) const
{
	std::set<BoardPosition> moves;
	const int direction = color == WHITE ? -1 : 1;
	const int next_row = row + direction;

	if (ChessBoard::IsValidPosition(next_row, column)
		&& board.GetPiece(next_row, column) == nullptr) {
		moves.emplace(next_row, column);
		const int start_row = color == WHITE ? 6 : 1;
		const int two_steps = row + (2 * direction);
		if (row == start_row && board.GetPiece(two_steps, column) == nullptr) {
			moves.emplace(two_steps, column);
		}
	}

	for (const int col_offset : {-1, 1}) {
		const int capture_col = column + col_offset;
		if (!ChessBoard::IsValidPosition(next_row, capture_col)) {
			continue;
		}
		const Piece * target = board.GetPiece(next_row, capture_col);
		if (target && target->GetColor() != color) {
			moves.emplace(next_row, capture_col);
		}
	}
	return moves;
}
