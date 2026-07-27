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
		PromotionRequired,
		Ignored,
	};
	enum class PlayerKind {
		Human,
		Computer,
	};

	ChessSession();

	void NewGame();
	Interaction SelectCell(int row, int col);
	void Promote(PieceType type);
	bool Undo();
	void Save();
	void SaveAs(const std::string & file_name);
	void Load(const std::string & file_name);
	void ClearInteraction();
	void SetPlayers(PlayerKind white, PlayerKind black);
	bool AdvanceComputer();

	const ChessBoard & Board() const;
	PieceColor Turn() const noexcept;
	GameStatus GameState() const noexcept;
	const std::set<BoardPosition> & LegalMoves() const noexcept;
	const std::optional<BoardPosition> & Selected() const noexcept;
	const std::string & Status() const noexcept;
	const std::string & CurrentFile() const noexcept;
	bool IsCaptureTarget(int row, int col) const;
	bool HasPendingPromotion() const noexcept;
	PlayerKind PlayerFor(PieceColor color) const noexcept;

private:
	struct PendingMove
	{
		int source_row;
		int source_col;
		int destination_row;
		int destination_col;
	};

	GameFacade game_;
	std::optional<BoardPosition> selected_;
	std::set<BoardPosition> legal_moves_;
	std::optional<PendingMove> pending_promotion_;
	std::string status_;
	PlayerKind white_player_ = PlayerKind::Human;
	PlayerKind black_player_ = PlayerKind::Human;

	void SelectPiece(int row, int col);
	void ClearSelection();
	void RefreshStatus(const std::string & prefix = {});
	void CompleteMove(const PendingMove & move, std::optional<PieceType> promotion);
};

#endif
