#include "King.h"

#include "MoveGeneration.h"

std::set<BoardPosition> King::GetValidMove(const ChessBoard & board) const
{
	std::set<BoardPosition> moves;
	for (int row_offset = -1; row_offset <= 1; ++row_offset) {
		for (int col_offset = -1; col_offset <= 1; ++col_offset) {
			if (row_offset != 0 || col_offset != 0) {
				AddMoveIfAvailable(
					moves, board, color, row + row_offset, column + col_offset);
			}
		}
	}
	return moves;
}
