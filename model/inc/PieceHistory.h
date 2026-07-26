#ifndef PIECE_HISTORY_H
#define PIECE_HISTORY_H

#include <optional>

#include "Piece.h"

struct PieceSnapshot
{
	PieceType type;
	PieceColor color;
	int row;
	int column;

	static PieceSnapshot FromPiece(const Piece & piece) noexcept;
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

private:
	PieceSnapshot moving_piece_;
	std::optional<PieceSnapshot> attacked_piece_;
	int start_row_;
	int start_column_;
	int end_row_;
	int end_column_;
};

#endif
