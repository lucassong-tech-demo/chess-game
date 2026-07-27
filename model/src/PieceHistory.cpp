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

PieceHistory::PieceHistory(
	PieceSnapshot moving_piece,
	int start_row,
	int start_column,
	int end_row,
	int end_column,
	std::optional<PieceSnapshot> attacked_piece,
	MoveKind kind,
	std::optional<PieceType> promotion,
	std::optional<BoardPosition> rook_start,
	std::optional<BoardPosition> rook_end,
	CastlingRights castling_before,
	std::optional<BoardPosition> en_passant_before,
	unsigned halfmove_before,
	unsigned fullmove_before,
	PieceColor turn_before)
	: moving_piece_(moving_piece),
	  attacked_piece_(attacked_piece),
	  start_row_(start_row),
	  start_column_(start_column),
	  end_row_(end_row),
	  end_column_(end_column),
	  kind_(kind),
	  promotion_(promotion),
	  rook_start_(rook_start),
	  rook_end_(rook_end),
	  castling_before_(castling_before),
	  en_passant_before_(en_passant_before),
	  halfmove_before_(halfmove_before),
	  fullmove_before_(fullmove_before),
	  turn_before_(turn_before)
{
	if (!ChessBoard::IsValidPosition(start_row, start_column)
		|| !ChessBoard::IsValidPosition(end_row, end_column)) {
		throw std::out_of_range("history coordinates must be between 0 and 7");
	}
	if ((rook_start_.has_value() != rook_end_.has_value())
		|| (kind_ == MoveKind::Castling && !rook_start_)) {
		throw std::invalid_argument("castling history requires both rook coordinates");
	}
	if (kind_ == MoveKind::Promotion && !promotion_) {
		throw std::invalid_argument("promotion history requires a piece type");
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
MoveKind PieceHistory::Kind() const noexcept { return kind_; }
const std::optional<PieceType> & PieceHistory::Promotion() const noexcept { return promotion_; }
const std::optional<BoardPosition> & PieceHistory::RookStart() const noexcept
{
	return rook_start_;
}
const std::optional<BoardPosition> & PieceHistory::RookEnd() const noexcept
{
	return rook_end_;
}
const CastlingRights & PieceHistory::CastlingBefore() const noexcept
{
	return castling_before_;
}
const std::optional<BoardPosition> & PieceHistory::EnPassantBefore() const noexcept
{
	return en_passant_before_;
}
unsigned PieceHistory::HalfmoveBefore() const noexcept { return halfmove_before_; }
unsigned PieceHistory::FullmoveBefore() const noexcept { return fullmove_before_; }
PieceColor PieceHistory::TurnBefore() const noexcept { return turn_before_; }
