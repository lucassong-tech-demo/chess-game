#include "GameFacade.h"

#include <stdexcept>
#include <utility>

#include "Bishop.h"
#include "King.h"
#include "Knight.h"
#include "Pawn.h"
#include "Queen.h"
#include "Rook.h"
#include "UnitTest.h"
#include "Xml.h"

namespace {

std::unique_ptr<Piece> MakePiece(const PieceSnapshot & snapshot, int row, int col)
{
	switch (snapshot.type) {
	case KING: return std::make_unique<King>(snapshot.type, snapshot.color, row, col);
	case QUEEN: return std::make_unique<Queen>(snapshot.type, snapshot.color, row, col);
	case KNIGHT: return std::make_unique<Knight>(snapshot.type, snapshot.color, row, col);
	case BISHOP: return std::make_unique<Bishop>(snapshot.type, snapshot.color, row, col);
	case ROOK: return std::make_unique<Rook>(snapshot.type, snapshot.color, row, col);
	case PAWN: return std::make_unique<Pawn>(snapshot.type, snapshot.color, row, col);
	}
	throw std::invalid_argument("unknown piece type in history");
}

bool Contains(const std::set<BoardPosition> & moves, int row, int col)
{
	return moves.find(BoardPosition(row, col)) != moves.end();
}

} // namespace

GameFacade::GameFacade() = default;

void GameFacade::RequireBoard() const
{
	if (!board_) {
		throw std::logic_error("game has no active board");
	}
}

void GameFacade::NewGame()
{
	board_ = std::make_unique<ChessBoard>();
	current_piece_ = nullptr;
	valid_moves_.clear();
	move_history_.clear();
	undo_once_.reset();
}

void GameFacade::Clear_Board() noexcept
{
	board_.reset();
	current_piece_ = nullptr;
	valid_moves_.clear();
	undo_once_.reset();
}

void GameFacade::ClearUndo() noexcept
{
	undo_once_.reset();
}

void GameFacade::ClearHistory() noexcept
{
	move_history_.clear();
	undo_once_.reset();
}

Piece * GameFacade::GetPiece(int row, int col, PieceColor color)
{
	RequireBoard();
	Piece * piece = board_->GetPiece(row, col);
	current_piece_ = piece && piece->GetColor() == color ? piece : nullptr;
	valid_moves_.clear();
	return current_piece_;
}

std::set<BoardPosition> & GameFacade::GetValidMoves()
{
	LookForMoves();
	return valid_moves_;
}

void GameFacade::LookForMoves()
{
	RequireBoard();
	if (!current_piece_) {
		throw std::logic_error("select a piece before requesting valid moves");
	}
	FilterMoves(current_piece_->GetValidMove(*board_));
}

void GameFacade::FilterMoves(const std::set<BoardPosition> & moves)
{
	valid_moves_.clear();
	for (const BoardPosition & move : moves) {
		const auto opponent_moves = BoardCheck(
			current_piece_->GetRow(),
			current_piece_->GetColumn(),
			move.GetRow(),
			move.GetColumn());
		const Piece * king = board_->GetKing(current_piece_->GetColor());
		if (!king) {
			throw std::logic_error("selected side has no king");
		}

		const int king_row = current_piece_ == king ? move.GetRow() : king->GetRow();
		const int king_col = current_piece_ == king ? move.GetColumn() : king->GetColumn();
		if (!Contains(opponent_moves, king_row, king_col)) {
			valid_moves_.insert(move);
		}
	}
}

std::set<BoardPosition> GameFacade::BoardCheck(
	int source_row,
	int source_col,
	int destination_row,
	int destination_col)
{
	const PieceColor moving_color = current_piece_->GetColor();
	auto captured_piece = board_->MovePiece(
		source_row, source_col, destination_row, destination_col);

	std::set<BoardPosition> opponent_moves;
	try {
		for (int row = 0; row < ChessBoard::Size; ++row) {
			for (int col = 0; col < ChessBoard::Size; ++col) {
				const Piece * piece = board_->GetPiece(row, col);
				if (piece && piece->GetColor() != moving_color) {
					const auto piece_moves = piece->GetValidMove(*board_);
					opponent_moves.insert(piece_moves.begin(), piece_moves.end());
				}
			}
		}
	} catch (...) {
		board_->RestoreMove(
			source_row,
			source_col,
			destination_row,
			destination_col,
			std::move(captured_piece));
		throw;
	}

	board_->RestoreMove(
		source_row,
		source_col,
		destination_row,
		destination_col,
		std::move(captured_piece));
	return opponent_moves;
}

bool GameFacade::isCellTaken(int row, int col)
{
	RequireBoard();
	return board_->GetPiece(row, col) != nullptr;
}

bool GameFacade::isValidMove(int row, int col) const
{
	if (!ChessBoard::IsValidPosition(row, col)) {
		throw std::out_of_range("board coordinates must be between 0 and 7");
	}
	return Contains(valid_moves_, row, col);
}

void GameFacade::MovePiece(
	int source_row,
	int source_col,
	int destination_row,
	int destination_col)
{
	RequireBoard();
	Piece * moving_piece = board_->GetPiece(source_row, source_col);
	if (!moving_piece) {
		throw std::invalid_argument("cannot move from an empty board cell");
	}
	const Piece * attacked_piece = board_->GetPiece(destination_row, destination_col);
	move_history_.emplace_back(
		*moving_piece,
		source_row,
		source_col,
		destination_row,
		destination_col,
		attacked_piece);

	[[maybe_unused]] auto captured_piece = board_->MovePiece(
		source_row, source_col, destination_row, destination_col);
	current_piece_ = board_->GetPiece(destination_row, destination_col);
	valid_moves_.clear();
	undo_once_.reset();
}

const PieceHistory * GameFacade::Undo()
{
	RequireBoard();
	undo_once_.reset();
	if (move_history_.empty()) {
		return nullptr;
	}

	undo_once_ = move_history_.back();
	move_history_.pop_back();
	std::unique_ptr<Piece> captured_piece;
	if (undo_once_->GetAttackSnapshot()) {
		captured_piece = MakePiece(
			*undo_once_->GetAttackSnapshot(),
			undo_once_->Get_E_Row(),
			undo_once_->Get_E_Column());
	}
	board_->RestoreMove(
		undo_once_->Get_S_Row(),
		undo_once_->Get_S_Column(),
		undo_once_->Get_E_Row(),
		undo_once_->Get_E_Column(),
		std::move(captured_piece));
	current_piece_ = nullptr;
	valid_moves_.clear();
	return &*undo_once_;
}

bool GameFacade::Check(int row, int col)
{
	RequireBoard();
	if (!ChessBoard::IsValidPosition(row, col)) {
		throw std::out_of_range("board coordinates must be between 0 and 7");
	}
	if (!current_piece_) {
		throw std::logic_error("select a piece before checking");
	}
	const PieceColor opponent = current_piece_->GetColor() == WHITE ? BLACK : WHITE;
	const Piece * king = board_->GetKing(opponent);
	if (!king) {
		return false;
	}
	return Contains(
		current_piece_->GetValidMove(*board_), king->GetRow(), king->GetColumn());
}

bool GameFacade::Mate(int row, int col)
{
	RequireBoard();
	if (!ChessBoard::IsValidPosition(row, col)) {
		throw std::out_of_range("board coordinates must be between 0 and 7");
	}
	if (!current_piece_) {
		throw std::logic_error("select a piece before checking mate");
	}
	const PieceColor last_mover = current_piece_->GetColor();
	for (int row = 0; row < ChessBoard::Size; ++row) {
		for (int col = 0; col < ChessBoard::Size; ++col) {
			Piece * piece = board_->GetPiece(row, col);
			if (piece && piece->GetColor() != last_mover) {
				current_piece_ = piece;
				LookForMoves();
				if (!valid_moves_.empty()) {
					return false;
				}
			}
		}
	}
	return true;
}

void GameFacade::SaveGame(const std::string & file_name)
{
	RequireBoard();
	Xml(file_name).WriteIntoFile(*board_, move_history_);
}

void GameFacade::SaveAs(const std::string & file_name)
{
	file_name_ = file_name;
	SaveGame(file_name_);
}

bool GameFacade::LoadGame(const std::string &)
{
	return false;
}

void GameFacade::UpdateMoveHistory(std::vector<PieceHistory> & moves)
{
	move_history_ = moves;
}

void GameFacade::ReadMoveHistory() {}
void GameFacade::Clean_History() { ClearHistory(); }
void GameFacade::Reset_Chess_Board() { NewGame(); }

void GameFacade::Quit()
{
	std::cout << "Quit Facade ...\n";
}

std::size_t GameFacade::HistorySize() const noexcept
{
	return move_history_.size();
}

const ChessBoard & GameFacade::Board() const
{
	RequireBoard();
	return *board_;
}

bool GameFacade::Test(std::ostream & os)
{
	using std::endl;
	bool success = true;
	GameFacade game;
	game.NewGame();
	TEST(game.GetPiece(0, 4, BLACK)->GetType() == KING);
	game.MovePiece(7, 6, 5, 5);
	TEST(game.Board().GetPiece(5, 5)->GetType() == KNIGHT);
	TEST(game.HistorySize() == 1);
	TEST(game.Undo() != nullptr);
	TEST(game.Board().GetPiece(7, 6)->GetType() == KNIGHT);
	TEST(game.HistorySize() == 0);
	os << "\tGameFacade ownership tests passed\n";
	return success;
}
