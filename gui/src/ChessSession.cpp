#include "ChessSession.h"

#include <random>
#include <stdexcept>
#include <vector>

namespace {

std::string TurnName(PieceColor color)
{
	return color == WHITE ? "White" : "Black";
}

std::string StateStatus(const GameFacade & game)
{
	const std::string turn = TurnName(game.Turn());
	switch (game.Status()) {
	case GameStatus::Ongoing: return turn + " to move";
	case GameStatus::Check: return turn + " to move — check";
	case GameStatus::Checkmate:
		return "Checkmate — " + TurnName(game.Turn() == WHITE ? BLACK : WHITE)
			+ " wins";
	case GameStatus::Stalemate: return "Stalemate";
	case GameStatus::DrawThreefold: return "Draw — threefold repetition";
	case GameStatus::DrawFiftyMove: return "Draw — fifty-move rule";
	case GameStatus::DrawInsufficientMaterial:
		return "Draw — insufficient material";
	}
	throw std::logic_error("unknown game status");
}

bool IsPromotionSquare(const Piece & piece, int row)
{
	return piece.GetType() == PAWN && (row == 0 || row == 7);
}

} // namespace

ChessSession::ChessSession()
	: ChessSession(std::random_device{}())
{
}

ChessSession::ChessSession(std::uint32_t random_seed)
	: random_(random_seed)
{
	NewGame();
}

void ChessSession::NewGame()
{
	game_.NewGame();
	ClearSelection();
	pending_promotion_.reset();
	RefreshStatus("New game");
}

ChessSession::Interaction ChessSession::SelectCell(int row, int col)
{
	if (game_.IsGameOver() || PlayerFor(game_.Turn()) == PlayerKind::Computer
		|| pending_promotion_) {
		ClearSelection();
		RefreshStatus();
		return Interaction::Ignored;
	}

	if (selected_ && game_.isValidMove(row, col)) {
		const Piece * moving = game_.Board().GetPiece(
			selected_->GetRow(), selected_->GetColumn());
		const PendingMove move{
			selected_->GetRow(), selected_->GetColumn(), row, col};
		if (moving && IsPromotionSquare(*moving, row)) {
			pending_promotion_ = move;
			ClearSelection();
			status_ = "Choose promotion: queen, rook, bishop, or knight";
			return Interaction::PromotionRequired;
		}
		CompleteMove(move, std::nullopt);
		return Interaction::MoveCompleted;
	}

	const Piece * clicked_piece = game_.Board().GetPiece(row, col);
	if (clicked_piece && clicked_piece->GetColor() == game_.Turn()) {
		SelectPiece(row, col);
		return Interaction::SelectionChanged;
	}

	ClearSelection();
	RefreshStatus();
	return Interaction::Ignored;
}

void ChessSession::Promote(PieceType type)
{
	if (!pending_promotion_) {
		throw std::logic_error("no promotion is pending");
	}
	const PendingMove move = *pending_promotion_;
	pending_promotion_.reset();
	CompleteMove(move, type);
}

void ChessSession::CompleteMove(
	const PendingMove & move,
	std::optional<PieceType> promotion)
{
	game_.MovePiece(
		move.source_row,
		move.source_col,
		move.destination_row,
		move.destination_col,
		promotion);
	ClearSelection();
	pending_promotion_.reset();
	RefreshStatus();
}

bool ChessSession::Undo()
{
	ClearInteraction();
	if (!game_.Undo()) {
		status_ = "Nothing to undo — " + StateStatus(game_);
		return false;
	}
	RefreshStatus("Move undone");
	return true;
}

void ChessSession::Save()
{
	ClearInteraction();
	if (game_.CurrentFile().empty()) {
		throw std::logic_error("save path required");
	}
	game_.SaveGame(game_.CurrentFile());
	RefreshStatus("Game saved");
}

void ChessSession::SaveAs(const std::string & file_name)
{
	ClearInteraction();
	game_.SaveAs(file_name);
	RefreshStatus("Game saved");
}

void ChessSession::Load(const std::string & file_name)
{
	ClearInteraction();
	game_.LoadGame(file_name);
	RefreshStatus("Game loaded");
}

void ChessSession::ClearInteraction()
{
	ClearSelection();
	pending_promotion_.reset();
}

void ChessSession::SetPlayers(PlayerKind white, PlayerKind black)
{
	white_player_ = white;
	black_player_ = black;
	ClearInteraction();
	RefreshStatus("Player mode changed");
}

bool ChessSession::AdvanceComputer()
{
	if (game_.IsGameOver() || PlayerFor(game_.Turn()) != PlayerKind::Computer) {
		return false;
	}
	const std::optional<ComputerMove> move = ChooseBeginnerMove();
	if (!move) {
		RefreshStatus();
		return false;
	}

	const Piece * piece = game_.Board().GetPiece(
		move->source.GetRow(), move->source.GetColumn());
	const std::optional<PieceType> promotion =
		piece && IsPromotionSquare(*piece, move->destination.GetRow())
		? std::optional<PieceType>(QUEEN) : std::nullopt;
	game_.MovePiece(
		move->source.GetRow(),
		move->source.GetColumn(),
		move->destination.GetRow(),
		move->destination.GetColumn(),
		promotion);
	RefreshStatus("Computer moved");
	return true;
}

std::optional<ChessSession::ComputerMove> ChessSession::ChooseBeginnerMove()
{
	std::vector<ComputerMove> choices;
	for (int row = 0; row < ChessBoard::Size; ++row) {
		for (int col = 0; col < ChessBoard::Size; ++col) {
			const Piece * piece = game_.Board().GetPiece(row, col);
			if (!piece || piece->GetColor() != game_.Turn()) {
				continue;
			}
			const auto moves = game_.LegalMoves(row, col);
			if (moves.empty()) {
				continue;
			}
			// Preserve the 2010 Beginner strategy: choose a random movable
			// piece, then use that piece's first coordinate-ordered move.
			choices.push_back({BoardPosition(row, col), *moves.begin()});
		}
	}

	if (choices.empty()) {
		return std::nullopt;
	}
	const std::size_t choice =
		static_cast<std::size_t>(random_()) % choices.size();
	return choices[choice];
}

const ChessBoard & ChessSession::Board() const { return game_.Board(); }
PieceColor ChessSession::Turn() const noexcept { return game_.Turn(); }
GameStatus ChessSession::GameState() const noexcept { return game_.Status(); }
const std::set<BoardPosition> & ChessSession::LegalMoves() const noexcept
{
	return legal_moves_;
}
const std::optional<BoardPosition> & ChessSession::Selected() const noexcept
{
	return selected_;
}
const std::string & ChessSession::Status() const noexcept { return status_; }
const std::string & ChessSession::CurrentFile() const noexcept
{
	return game_.CurrentFile();
}

bool ChessSession::IsCaptureTarget(int row, int col) const
{
	if (!game_.isValidMove(row, col)) {
		return false;
	}
	if (game_.isCellTaken(row, col)) {
		return true;
	}
	const Piece * selected_piece = selected_
		? game_.Board().GetPiece(selected_->GetRow(), selected_->GetColumn())
		: nullptr;
	return selected_piece && selected_piece->GetType() == PAWN
		&& selected_->GetColumn() != col;
}

bool ChessSession::HasPendingPromotion() const noexcept
{
	return pending_promotion_.has_value();
}

ChessSession::PlayerKind ChessSession::PlayerFor(PieceColor color) const noexcept
{
	return color == WHITE ? white_player_ : black_player_;
}

void ChessSession::SelectPiece(int row, int col)
{
	game_.GetPiece(row, col, game_.Turn());
	selected_.emplace(row, col);
	legal_moves_ = game_.GetValidMoves();
	status_ = StateStatus(game_) + " — "
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

void ChessSession::RefreshStatus(const std::string & prefix)
{
	status_ = prefix.empty() ? StateStatus(game_) : prefix + " — " + StateStatus(game_);
}
