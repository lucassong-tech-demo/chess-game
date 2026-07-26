#include "PieceHistory.h"

#include <stdexcept>

#include "ChessBoard.h"

PieceSnapshot PieceSnapshot::FromPiece(const Piece & piece) noexcept
{
	return {piece.GetType(), piece.GetColor(), piece.GetRow(), piece.GetColumn()};
}

PieceHistory::PieceHistory(
	const Piece & moving_piece,
	int start_row,
	int start_column,
	int end_row,
	int end_column,
	const Piece * attacked_piece)
	: moving_piece_(PieceSnapshot::FromPiece(moving_piece)),
	  start_row_(start_row),
	  start_column_(start_column),
	  end_row_(end_row),
	  end_column_(end_column)
{
	if (!ChessBoard::IsValidPosition(start_row, start_column)
		|| !ChessBoard::IsValidPosition(end_row, end_column)) {
		throw std::out_of_range("history coordinates must be between 0 and 7");
	}
	if (attacked_piece) {
		attacked_piece_ = PieceSnapshot::FromPiece(*attacked_piece);
	}
}

PieceType PieceHistory::GetType_Moving() const noexcept { return moving_piece_.type; }
PieceColor PieceHistory::GetColor_Moving() const noexcept { return moving_piece_.color; }
bool PieceHistory::IsAttackPieceHere() const noexcept { return attacked_piece_.has_value(); }

PieceType PieceHistory::GetType_Attack() const
{
	if (!attacked_piece_) {
		throw std::logic_error("move did not capture a piece");
	}
	return attacked_piece_->type;
}

PieceColor PieceHistory::GetColor_Attack() const
{
	if (!attacked_piece_) {
		throw std::logic_error("move did not capture a piece");
	}
	return attacked_piece_->color;
}

const PieceSnapshot & PieceHistory::GetMovingSnapshot() const noexcept { return moving_piece_; }
const std::optional<PieceSnapshot> & PieceHistory::GetAttackSnapshot() const noexcept
{
	return attacked_piece_;
}
int PieceHistory::Get_S_Row() const noexcept { return start_row_; }
int PieceHistory::Get_S_Column() const noexcept { return start_column_; }
int PieceHistory::Get_E_Row() const noexcept { return end_row_; }
int PieceHistory::Get_E_Column() const noexcept { return end_column_; }
