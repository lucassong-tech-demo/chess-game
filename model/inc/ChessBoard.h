#ifndef CHESS_BOARD_H
#define CHESS_BOARD_H

#include <array>
#include <iostream>
#include <memory>

#include "Piece.h"

class ChessBoard
{
public:
	static constexpr int Size = 8;

	ChessBoard();
	~ChessBoard() = default;

	ChessBoard(const ChessBoard &);
	ChessBoard & operator=(const ChessBoard &);
	ChessBoard(ChessBoard &&) noexcept = default;
	ChessBoard & operator=(ChessBoard &&) noexcept = default;

	static bool IsValidPosition(int row, int col) noexcept;

	void ClearBoard() noexcept;
	void Reset();

	Piece * GetPiece(int row, int col);
	const Piece * GetPiece(int row, int col) const;
	Piece * GetKing(PieceColor color);
	const Piece * GetKing(PieceColor color) const;

	// Transfers ownership of any captured piece to the caller.
	std::unique_ptr<Piece> MovePiece(
		int source_row,
		int source_col,
		int destination_row,
		int destination_col);

	// Reverses MovePiece and restores ownership of an optional captured piece.
	void RestoreMove(
		int source_row,
		int source_col,
		int destination_row,
		int destination_col,
		std::unique_ptr<Piece> captured_piece = {});

	void PutPiece(std::unique_ptr<Piece> piece, int row, int col);
	std::unique_ptr<Piece> TakePiece(int row, int col);

	static bool Test(std::ostream & os);

private:
	using Row = std::array<std::unique_ptr<Piece>, Size>;
	std::array<Row, Size> board_{};

	void InitializePieces();
	static void ValidatePosition(int row, int col);
};

#endif
