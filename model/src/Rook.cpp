#include "Rook.h"

#include "MoveGeneration.h"

std::set<BoardPosition> Rook::GetValidMove(const ChessBoard & board) const
{
	std::set<BoardPosition> moves;
	AddRay(moves, board, color, row, column, -1, 0);
	AddRay(moves, board, color, row, column, 0, 1);
	AddRay(moves, board, color, row, column, 0, -1);
	AddRay(moves, board, color, row, column, 1, 0);
	return moves;
}
