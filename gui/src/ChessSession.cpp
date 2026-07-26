#include "ChessSession.h"

namespace {

std::string TurnStatus(PieceColor color)
{
	return color == WHITE ? "White to move" : "Black to move";
}

std::string TurnPiece(PieceColor color)
{
	return color == WHITE ? "white piece" : "black piece";
}

} // namespace

ChessSession::ChessSession()
{
	NewGame();
}

void ChessSession::NewGame()
{
	game_.NewGame();
	turn_ = WHITE;
	game_active_ = true;
	ClearSelection();
	status_ = TurnStatus(turn_);
}

ChessSession::Interaction ChessSession::SelectCell(int row, int col)
{
	if (!game_active_) {
		status_ = "Game over — start a new game or undo";
		return Interaction::Ignored;
	}

	if (selected_ && game_.isValidMove(row, col)) {
		const bool captured = game_.isCellTaken(row, col);
		const int source_row = selected_->GetRow();
		const int source_col = selected_->GetColumn();
		game_.MovePiece(source_row, source_col, row, col);
		UpdateStatusAfterMove(row, col, captured);
		ClearSelection();
		return Interaction::MoveCompleted;
	}

	const Piece * clicked_piece = game_.Board().GetPiece(row, col);
	if (clicked_piece && clicked_piece->GetColor() == turn_) {
		SelectPiece(row, col);
		return Interaction::SelectionChanged;
	}

	if (selected_) {
		status_ = "Choose a highlighted square or another "
			+ TurnPiece(turn_);
	} else {
		status_ = TurnStatus(turn_);
	}
	return Interaction::Ignored;
}

bool ChessSession::Undo()
{
	if (!game_.Undo()) {
		status_ = "Nothing to undo";
		return false;
	}

	turn_ = turn_ == WHITE ? BLACK : WHITE;
	game_active_ = true;
	ClearSelection();
	status_ = "Move undone — " + TurnStatus(turn_);
	return true;
}

void ChessSession::Save(const std::string & file_name)
{
	game_.SaveGame(file_name);
	status_ = "Game saved";
}

const ChessBoard & ChessSession::Board() const
{
	return game_.Board();
}

PieceColor ChessSession::Turn() const noexcept
{
	return turn_;
}

const std::set<BoardPosition> & ChessSession::LegalMoves() const noexcept
{
	return legal_moves_;
}

const std::optional<BoardPosition> & ChessSession::Selected() const noexcept
{
	return selected_;
}

const std::string & ChessSession::Status() const noexcept
{
	return status_;
}

bool ChessSession::IsCaptureTarget(int row, int col) const
{
	return game_.isValidMove(row, col) && game_.isCellTaken(row, col);
}

void ChessSession::SelectPiece(int row, int col)
{
	game_.GetPiece(row, col, turn_);
	selected_.emplace(row, col);
	legal_moves_ = game_.GetValidMoves();
	status_ = TurnStatus(turn_) + " — "
		+ std::to_string(legal_moves_.size()) + " legal move";
	if (legal_moves_.size() != 1) {
		status_ += "s";
	}
}

void ChessSession::ClearSelection()
{
	selected_.reset();
	legal_moves_.clear();
}

void ChessSession::UpdateStatusAfterMove(int row, int col, bool captured)
{
	const bool check = game_.Check(row, col);
	const bool mate = game_.Mate(row, col);
	turn_ = turn_ == WHITE ? BLACK : WHITE;

	if (mate) {
		game_active_ = false;
		status_ = check ? "Checkmate" : "Stalemate";
		return;
	}

	status_ = TurnStatus(turn_);
	if (check) {
		status_ += " — check";
	} else if (captured) {
		status_ += " — piece captured";
	}
}
