#include "Knight.h"

#include <array>
#include <utility>

#include "MoveGeneration.h"

std::set<BoardPosition> Knight::GetValidMove(const ChessBoard & board) const
{
	static constexpr std::array<std::pair<int, int>, 8> offsets{{
		{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2},
		{1, -2}, {1, 2}, {2, -1}, {2, 1}
	}};
	std::set<BoardPosition> moves;
	for (const auto [row_offset, col_offset] : offsets) {
		AddMoveIfAvailable(
			moves, board, color, row + row_offset, column + col_offset);
	}
	return moves;
}
