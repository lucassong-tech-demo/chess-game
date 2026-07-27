#ifndef PIECE_HISTORY_H
#define PIECE_HISTORY_H

#include <optional>
#include <string>

#include "BoardPosition.h"
#include "Piece.h"

struct PieceSnapshot
{
	PieceType type;
	PieceColor color;
	int row;
	int column;

	static PieceSnapshot FromPiece(const Piece & piece) noexcept;
};

struct CastlingRights
{
	bool white_king_side = true;
	bool white_queen_side = true;
	bool black_king_side = true;
	bool black_queen_side = true;

	bool operator==(const CastlingRights &) const = default;
};

enum class MoveKind {
	Normal,
	Castling,
	EnPassant,
	Promotion,
};

class PieceHistory
{
public:
	PieceHistory(
		const Piece & moving_piece,
		int start_row,
		int start_column,
		int end_row,
		int end_column,
		const Piece * attacked_piece = nullptr);
	PieceHistory(
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
		PieceColor turn_before);

	PieceType GetType_Moving() const noexcept;
	PieceColor GetColor_Moving() const noexcept;
	bool IsAttackPieceHere() const noexcept;
	PieceType GetType_Attack() const;
	PieceColor GetColor_Attack() const;
	const PieceSnapshot & GetMovingSnapshot() const noexcept;
	const std::optional<PieceSnapshot> & GetAttackSnapshot() const noexcept;
	int Get_S_Row() const noexcept;
	int Get_S_Column() const noexcept;
	int Get_E_Row() const noexcept;
	int Get_E_Column() const noexcept;
	MoveKind Kind() const noexcept;
	const std::optional<PieceType> & Promotion() const noexcept;
	const std::optional<BoardPosition> & RookStart() const noexcept;
	const std::optional<BoardPosition> & RookEnd() const noexcept;
	const CastlingRights & CastlingBefore() const noexcept;
	const std::optional<BoardPosition> & EnPassantBefore() const noexcept;
	unsigned HalfmoveBefore() const noexcept;
	unsigned FullmoveBefore() const noexcept;
	PieceColor TurnBefore() const noexcept;

private:
	PieceSnapshot moving_piece_;
	std::optional<PieceSnapshot> attacked_piece_;
	int start_row_;
	int start_column_;
	int end_row_;
	int end_column_;
	MoveKind kind_ = MoveKind::Normal;
	std::optional<PieceType> promotion_;
	std::optional<BoardPosition> rook_start_;
	std::optional<BoardPosition> rook_end_;
	CastlingRights castling_before_{};
	std::optional<BoardPosition> en_passant_before_;
	unsigned halfmove_before_ = 0;
	unsigned fullmove_before_ = 1;
	PieceColor turn_before_ = WHITE;
};

#endif
