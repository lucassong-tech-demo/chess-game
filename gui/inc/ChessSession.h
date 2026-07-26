#ifndef CHESS_SESSION_H
#define CHESS_SESSION_H

#include <optional>
#include <set>
#include <string>

#include "BoardPosition.h"
#include "GameFacade.h"

class ChessSession
{
public:
	enum class Interaction {
		SelectionChanged,
		MoveCompleted,
		Ignored,
	};

	ChessSession();

	void NewGame();
	Interaction SelectCell(int row, int col);
	bool Undo();
	void Save(const std::string & file_name);

	const ChessBoard & Board() const;
	PieceColor Turn() const noexcept;
	const std::set<BoardPosition> & LegalMoves() const noexcept;
	const std::optional<BoardPosition> & Selected() const noexcept;
	const std::string & Status() const noexcept;
	bool IsCaptureTarget(int row, int col) const;

private:
	GameFacade game_;
	PieceColor turn_ = WHITE;
	std::optional<BoardPosition> selected_;
	std::set<BoardPosition> legal_moves_;
	std::string status_;
	bool game_active_ = true;

	void SelectPiece(int row, int col);
	void ClearSelection();
	void UpdateStatusAfterMove(int row, int col, bool captured);
};

#endif
