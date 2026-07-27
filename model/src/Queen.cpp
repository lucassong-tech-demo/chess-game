#include "Queen.h"

#include "MoveGeneration.h"

std::set<BoardPosition> Queen::GetValidMove(const ChessBoard & board) const
{
	std::set<BoardPosition> moves;
	for (int row_step = -1; row_step <= 1; ++row_step) {
		for (int col_step = -1; col_step <= 1; ++col_step) {
			if (row_step != 0 || col_step != 0) {
				AddRay(moves, board, color, row, column, row_step, col_step);
			}
		}
	}
	return moves;
}
