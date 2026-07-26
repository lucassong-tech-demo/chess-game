#include <iostream>
#include <string>

#include "ChessSession.h"

namespace {

int failures = 0;

void Check(bool condition, const std::string & message)
{
	if (!condition) {
		++failures;
		std::cerr << "FAIL: " << message << '\n';
	}
}

bool HasMove(const ChessSession & session, int row, int col)
{
	return session.LegalMoves().find(BoardPosition(row, col))
		!= session.LegalMoves().end();
}

} // namespace

int main()
{
	ChessSession session;
	Check(session.Turn() == WHITE, "white starts");
	Check(session.Board().GetPiece(6, 4)->GetType() == PAWN,
		"initial board is visible through the session");

	Check(
		session.SelectCell(6, 4) == ChessSession::Interaction::SelectionChanged,
		"clicking the current side selects a piece");
	Check(HasMove(session, 5, 4) && HasMove(session, 4, 4),
		"selection exposes legal destinations");
	Check(
		session.SelectCell(1, 0) == ChessSession::Interaction::Ignored,
		"an invalid enemy target does not replace the selection");
	Check(HasMove(session, 4, 4),
		"an invalid target keeps the legal move set usable");
	Check(
		session.SelectCell(4, 4) == ChessSession::Interaction::MoveCompleted,
		"a highlighted destination completes a move");
	Check(session.Turn() == BLACK, "a completed move changes turn");
	Check(session.Board().GetPiece(4, 4)->GetType() == PAWN,
		"completed move updates the core board");

	Check(session.SelectCell(1, 3) == ChessSession::Interaction::SelectionChanged,
		"black can select after white");
	Check(session.SelectCell(3, 3) == ChessSession::Interaction::MoveCompleted,
		"black can complete a move");
	Check(session.SelectCell(4, 4) == ChessSession::Interaction::SelectionChanged,
		"white pawn can be selected for a capture");
	Check(session.IsCaptureTarget(3, 3), "occupied legal move is a capture target");
	Check(session.SelectCell(3, 3) == ChessSession::Interaction::MoveCompleted,
		"capture completes");
	Check(session.Board().GetPiece(3, 3)->GetColor() == WHITE,
		"capturing piece replaces the victim");

	Check(session.Undo(), "capture can be undone");
	Check(session.Turn() == WHITE, "undo restores the moving side's turn");
	Check(session.Board().GetPiece(4, 4)->GetColor() == WHITE,
		"undo restores the capturing pawn");
	Check(session.Board().GetPiece(3, 3)->GetColor() == BLACK,
		"undo restores the captured pawn");

	session.NewGame();
	Check(session.Turn() == WHITE, "new game restores the first turn");
	Check(session.Board().GetPiece(6, 4)->GetColor() == WHITE,
		"new game restores the initial board");
	Check(!session.Selected(), "new game clears selection");

	if (failures != 0) {
		std::cerr << failures << " session test assertion(s) failed\n";
		return 1;
	}
	std::cout << "All chess session tests passed\n";
	return 0;
}
