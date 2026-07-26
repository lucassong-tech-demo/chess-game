#include "Bishop.h"

#include "MoveGeneration.h"

std::set<BoardPosition> Bishop::GetValidMove(const ChessBoard & board) const
{
	std::set<BoardPosition> moves;
	AddRay(moves, board, color, row, column, -1, -1);
	AddRay(moves, board, color, row, column, -1, 1);
	AddRay(moves, board, color, row, column, 1, -1);
	AddRay(moves, board, color, row, column, 1, 1);
	return moves;
}
