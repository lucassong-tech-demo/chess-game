#include "GameFacade.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <map>
#include <sstream>
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

PieceColor Opposite(PieceColor color)
{
	return color == WHITE ? BLACK : WHITE;
}

std::unique_ptr<Piece> MakePiece(PieceType type, PieceColor color, int row, int col)
{
	switch (type) {
	case KING: return std::make_unique<King>(type, color, row, col);
	case QUEEN: return std::make_unique<Queen>(type, color, row, col);
	case KNIGHT: return std::make_unique<Knight>(type, color, row, col);
	case BISHOP: return std::make_unique<Bishop>(type, color, row, col);
	case ROOK: return std::make_unique<Rook>(type, color, row, col);
	case PAWN: return std::make_unique<Pawn>(type, color, row, col);
	}
	throw std::invalid_argument("unknown piece type");
}

std::unique_ptr<Piece> MakePiece(const PieceSnapshot & snapshot)
{
	return MakePiece(snapshot.type, snapshot.color, snapshot.row, snapshot.column);
}

bool Contains(const std::set<BoardPosition> & moves, int row, int col)
{
	return moves.contains(BoardPosition(row, col));
}

bool IsPromotionType(PieceType type)
{
	return type == QUEEN || type == ROOK || type == BISHOP || type == KNIGHT;
}

char PieceCode(const Piece & piece)
{
	static constexpr std::array<char, 6> white{'K', 'Q', 'N', 'B', 'R', 'P'};
	const char code = white.at(static_cast<std::size_t>(piece.GetType()));
	return piece.GetColor() == WHITE ? code : static_cast<char>(code + ('a' - 'A'));
}

void ValidateHistory(const ChessBoard & current, const std::vector<PieceHistory> & history)
{
	ChessBoard board(current);
	for (auto it = history.rbegin(); it != history.rend(); ++it) {
		const PieceHistory & move = *it;
		const PieceSnapshot & moving = move.GetMovingSnapshot();
		if (moving.row != move.Get_S_Row()
			|| moving.column != move.Get_S_Column()) {
			throw std::invalid_argument("history moving snapshot has wrong coordinates");
		}
		const Piece * destination =
			board.GetPiece(move.Get_E_Row(), move.Get_E_Column());
		const PieceType expected_type = move.Kind() == MoveKind::Promotion
			? *move.Promotion() : moving.type;
		if (!destination || destination->GetColor() != moving.color
			|| destination->GetType() != expected_type) {
			throw std::invalid_argument("history does not match the saved board");
		}
		if (board.GetPiece(move.Get_S_Row(), move.Get_S_Column())) {
			throw std::invalid_argument("history source is unexpectedly occupied");
		}
		if (move.GetAttackSnapshot()
			&& (move.GetAttackSnapshot()->type == KING
				|| move.GetAttackSnapshot()->color == moving.color)) {
			throw std::invalid_argument("history contains an invalid capture");
		}
		if (move.Kind() == MoveKind::Promotion
			&& (moving.type != PAWN || !IsPromotionType(*move.Promotion()))) {
			throw std::invalid_argument("history contains an invalid promotion");
		}
		if (move.Kind() == MoveKind::Castling) {
			if (moving.type != KING || !move.RookStart() || !move.RookEnd()) {
				throw std::invalid_argument("history contains invalid castling");
			}
			const Piece * rook =
				board.GetPiece(move.RookEnd()->GetRow(), move.RookEnd()->GetColumn());
			if (!rook || rook->GetType() != ROOK || rook->GetColor() != moving.color
				|| board.GetPiece(
					move.RookStart()->GetRow(), move.RookStart()->GetColumn())) {
				throw std::invalid_argument("castling history does not match the board");
			}
			auto rook_owner =
				board.TakePiece(move.RookEnd()->GetRow(), move.RookEnd()->GetColumn());
			board.PutPiece(
				std::move(rook_owner),
				move.RookStart()->GetRow(),
				move.RookStart()->GetColumn());
		}
		board.TakePiece(move.Get_E_Row(), move.Get_E_Column());
		board.PutPiece(MakePiece(moving), move.Get_S_Row(), move.Get_S_Column());
		if (move.GetAttackSnapshot()) {
			const PieceSnapshot & captured = *move.GetAttackSnapshot();
			if (board.GetPiece(captured.row, captured.column)) {
				throw std::invalid_argument(
					"history captured square is unexpectedly occupied");
			}
			board.PutPiece(MakePiece(captured), captured.row, captured.column);
		}
	}
}

} // namespace

GameFacade::GameFacade()
{
	NewGame();
}

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
	turn_ = WHITE;
	status_ = GameStatus::Ongoing;
	castling_ = {};
	en_passant_.reset();
	halfmove_clock_ = 0;
	fullmove_number_ = 1;
	position_keys_.clear();
	position_keys_.push_back(PositionKey());
	file_name_.clear();
}

void GameFacade::Clear_Board() noexcept
{
	board_.reset();
	current_piece_ = nullptr;
	valid_moves_.clear();
	undo_once_.reset();
	position_keys_.clear();
}

void GameFacade::ClearUndo() noexcept
{
	undo_once_.reset();
}

void GameFacade::ClearHistory() noexcept
{
	move_history_.clear();
	undo_once_.reset();
	position_keys_.clear();
	if (board_) {
		position_keys_.push_back(PositionKey());
	}
}

Piece * GameFacade::GetPiece(int row, int col, PieceColor color)
{
	RequireBoard();
	Piece * piece = board_->GetPiece(row, col);
	current_piece_ = piece && piece->GetColor() == color && color == turn_
		? piece : nullptr;
	valid_moves_.clear();
	return current_piece_;
}

std::set<BoardPosition> & GameFacade::GetValidMoves()
{
	LookForMoves();
	return valid_moves_;
}

std::set<BoardPosition> GameFacade::LegalMoves(int row, int col) const
{
	RequireBoard();
	return LegalMovesFor(*board_, row, col, turn_, castling_, en_passant_);
}

void GameFacade::LookForMoves()
{
	RequireBoard();
	if (!current_piece_) {
		throw std::logic_error("select a current-side piece before requesting legal moves");
	}
	valid_moves_ = LegalMovesFor(
		*board_,
		current_piece_->GetRow(),
		current_piece_->GetColumn(),
		turn_,
		castling_,
		en_passant_);
}

std::set<BoardPosition> GameFacade::PseudoMovesFor(
	const ChessBoard & board,
	const Piece & piece,
	const CastlingRights & castling,
	const std::optional<BoardPosition> & en_passant) const
{
	std::set<BoardPosition> moves = piece.GetValidMove(board);
	for (auto it = moves.begin(); it != moves.end();) {
		const Piece * target = board.GetPiece(it->GetRow(), it->GetColumn());
		if (target && target->GetType() == KING) {
			it = moves.erase(it);
		} else {
			++it;
		}
	}

	if (piece.GetType() == PAWN && en_passant) {
		const int direction = piece.GetColor() == WHITE ? -1 : 1;
		if (en_passant->GetRow() == piece.GetRow() + direction
			&& std::abs(en_passant->GetColumn() - piece.GetColumn()) == 1
			&& board.GetPiece(en_passant->GetRow(), en_passant->GetColumn()) == nullptr) {
			const Piece * victim = board.GetPiece(
				piece.GetRow(), en_passant->GetColumn());
			if (victim && victim->GetType() == PAWN
				&& victim->GetColor() != piece.GetColor()) {
				moves.insert(*en_passant);
			}
		}
	}

	if (piece.GetType() != KING || piece.GetColumn() != 4) {
		return moves;
	}
	const int home_row = piece.GetColor() == WHITE ? 7 : 0;
	if (piece.GetRow() != home_row
		|| IsSquareAttacked(board, home_row, 4, Opposite(piece.GetColor()))) {
		return moves;
	}

	const bool king_side = piece.GetColor() == WHITE
		? castling.white_king_side : castling.black_king_side;
	const Piece * king_rook = board.GetPiece(home_row, 7);
	if (king_side && king_rook && king_rook->GetType() == ROOK
		&& king_rook->GetColor() == piece.GetColor()
		&& board.GetPiece(home_row, 5) == nullptr
		&& board.GetPiece(home_row, 6) == nullptr
		&& !IsSquareAttacked(board, home_row, 5, Opposite(piece.GetColor()))
		&& !IsSquareAttacked(board, home_row, 6, Opposite(piece.GetColor()))) {
		moves.emplace(home_row, 6);
	}

	const bool queen_side = piece.GetColor() == WHITE
		? castling.white_queen_side : castling.black_queen_side;
	const Piece * queen_rook = board.GetPiece(home_row, 0);
	if (queen_side && queen_rook && queen_rook->GetType() == ROOK
		&& queen_rook->GetColor() == piece.GetColor()
		&& board.GetPiece(home_row, 1) == nullptr
		&& board.GetPiece(home_row, 2) == nullptr
		&& board.GetPiece(home_row, 3) == nullptr
		&& !IsSquareAttacked(board, home_row, 3, Opposite(piece.GetColor()))
		&& !IsSquareAttacked(board, home_row, 2, Opposite(piece.GetColor()))) {
		moves.emplace(home_row, 2);
	}
	return moves;
}

std::set<BoardPosition> GameFacade::LegalMovesFor(
	const ChessBoard & board,
	int row,
	int col,
	PieceColor side,
	const CastlingRights & castling,
	const std::optional<BoardPosition> & en_passant) const
{
	const Piece * piece = board.GetPiece(row, col);
	if (!piece || piece->GetColor() != side) {
		return {};
	}

	std::set<BoardPosition> legal;
	for (const BoardPosition & move :
			PseudoMovesFor(board, *piece, castling, en_passant)) {
		ChessBoard simulation(board);
		const bool is_en_passant = piece->GetType() == PAWN
			&& en_passant && move == *en_passant
			&& simulation.GetPiece(move.GetRow(), move.GetColumn()) == nullptr
			&& move.GetColumn() != col;
		if (is_en_passant) {
			simulation.TakePiece(row, move.GetColumn());
		}
		simulation.MovePiece(row, col, move.GetRow(), move.GetColumn());
		if (piece->GetType() == KING && std::abs(move.GetColumn() - col) == 2) {
			const int rook_source_col = move.GetColumn() == 6 ? 7 : 0;
			const int rook_destination_col = move.GetColumn() == 6 ? 5 : 3;
			simulation.MovePiece(
				row, rook_source_col, row, rook_destination_col);
		}
		if (!IsInCheck(simulation, side)) {
			legal.insert(move);
		}
	}
	return legal;
}

bool GameFacade::IsSquareAttacked(
	const ChessBoard & board,
	int target_row,
	int target_col,
	PieceColor by_color) const
{
	for (int row = 0; row < ChessBoard::Size; ++row) {
		for (int col = 0; col < ChessBoard::Size; ++col) {
			const Piece * piece = board.GetPiece(row, col);
			if (!piece || piece->GetColor() != by_color) {
				continue;
			}
			if (piece->GetType() == PAWN) {
				const int direction = by_color == WHITE ? -1 : 1;
				if (target_row == row + direction
					&& std::abs(target_col - col) == 1) {
					return true;
				}
			} else if (piece->GetType() == KING) {
				if (std::max(
						std::abs(target_row - row),
						std::abs(target_col - col)) == 1) {
					return true;
				}
			} else if (Contains(piece->GetValidMove(board), target_row, target_col)) {
				return true;
			}
		}
	}
	return false;
}

bool GameFacade::IsInCheck(const ChessBoard & board, PieceColor color) const
{
	const Piece * king = board.GetKing(color);
	if (!king) {
		throw std::invalid_argument("position must contain both kings");
	}
	return IsSquareAttacked(
		board, king->GetRow(), king->GetColumn(), Opposite(color));
}

bool GameFacade::IsInCheck(PieceColor color) const
{
	RequireBoard();
	return IsInCheck(*board_, color);
}

bool GameFacade::HasLegalMove(PieceColor color) const
{
	for (int row = 0; row < ChessBoard::Size; ++row) {
		for (int col = 0; col < ChessBoard::Size; ++col) {
			const Piece * piece = board_->GetPiece(row, col);
			if (piece && piece->GetColor() == color
				&& !LegalMovesFor(
					*board_, row, col, color, castling_, en_passant_).empty()) {
				return true;
			}
		}
	}
	return false;
}

bool GameFacade::isCellTaken(int row, int col) const
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
	int destination_col,
	std::optional<PieceType> promotion)
{
	RequireBoard();
	if (IsGameOver()) {
		throw std::logic_error("the game is over; undo or start a new game");
	}
	Piece * moving_piece = board_->GetPiece(source_row, source_col);
	if (!moving_piece || moving_piece->GetColor() != turn_) {
		throw std::invalid_argument("only the current side may move");
	}
	const auto legal = LegalMovesFor(
		*board_, source_row, source_col, turn_, castling_, en_passant_);
	if (!Contains(legal, destination_row, destination_col)) {
		throw std::invalid_argument("illegal chess move");
	}

	const bool promotes = moving_piece->GetType() == PAWN
		&& (destination_row == 0 || destination_row == 7);
	if (promotes && (!promotion || !IsPromotionType(*promotion))) {
		throw std::invalid_argument(
			"promotion requires queen, rook, bishop, or knight");
	}
	if (!promotes && promotion) {
		throw std::invalid_argument("promotion is only valid on the final rank");
	}

	const PieceSnapshot moving_snapshot = PieceSnapshot::FromPiece(*moving_piece);
	const bool en_passant_capture = moving_piece->GetType() == PAWN
		&& en_passant_
		&& destination_row == en_passant_->GetRow()
		&& destination_col == en_passant_->GetColumn()
		&& board_->GetPiece(destination_row, destination_col) == nullptr
		&& source_col != destination_col;
	const int captured_row = en_passant_capture ? source_row : destination_row;
	const int captured_col = destination_col;
	std::optional<PieceSnapshot> captured_snapshot;
	if (const Piece * captured = board_->GetPiece(captured_row, captured_col)) {
		captured_snapshot = PieceSnapshot::FromPiece(*captured);
	}

	const bool castling_move = moving_piece->GetType() == KING
		&& std::abs(destination_col - source_col) == 2;
	std::optional<BoardPosition> rook_start;
	std::optional<BoardPosition> rook_end;
	if (castling_move) {
		rook_start.emplace(source_row, destination_col == 6 ? 7 : 0);
		rook_end.emplace(source_row, destination_col == 6 ? 5 : 3);
	}
	const MoveKind kind = promotes ? MoveKind::Promotion
		: en_passant_capture ? MoveKind::EnPassant
		: castling_move ? MoveKind::Castling
		: MoveKind::Normal;

	move_history_.emplace_back(
		moving_snapshot,
		source_row,
		source_col,
		destination_row,
		destination_col,
		captured_snapshot,
		kind,
		promotion,
		rook_start,
		rook_end,
		castling_,
		en_passant_,
		halfmove_clock_,
		fullmove_number_,
		turn_);

	if (en_passant_capture) {
		board_->TakePiece(captured_row, captured_col);
	}
	board_->MovePiece(
		source_row, source_col, destination_row, destination_col);
	if (castling_move) {
		board_->MovePiece(
			rook_start->GetRow(),
			rook_start->GetColumn(),
			rook_end->GetRow(),
			rook_end->GetColumn());
	}
	if (promotes) {
		board_->TakePiece(destination_row, destination_col);
		board_->PutPiece(
			MakePiece(*promotion, moving_snapshot.color, destination_row, destination_col),
			destination_row,
			destination_col);
	}

	UpdateCastlingRights(
		moving_snapshot, source_row, source_col, captured_snapshot);
	en_passant_.reset();
	if (moving_snapshot.type == PAWN
		&& std::abs(destination_row - source_row) == 2) {
		en_passant_.emplace((source_row + destination_row) / 2, source_col);
	}
	halfmove_clock_ = moving_snapshot.type == PAWN || captured_snapshot
		? 0 : halfmove_clock_ + 1;
	if (turn_ == BLACK) {
		++fullmove_number_;
	}
	turn_ = Opposite(turn_);
	current_piece_ = nullptr;
	valid_moves_.clear();
	undo_once_.reset();
	position_keys_.push_back(PositionKey());
	UpdateStatus();
}

void GameFacade::UpdateCastlingRights(
	const PieceSnapshot & moving,
	int source_row,
	int source_col,
	const std::optional<PieceSnapshot> & captured)
{
	if (moving.type == KING) {
		if (moving.color == WHITE) {
			castling_.white_king_side = false;
			castling_.white_queen_side = false;
		} else {
			castling_.black_king_side = false;
			castling_.black_queen_side = false;
		}
	}
	if (moving.type == ROOK) {
		if (moving.color == WHITE && source_row == 7 && source_col == 0) {
			castling_.white_queen_side = false;
		} else if (moving.color == WHITE && source_row == 7 && source_col == 7) {
			castling_.white_king_side = false;
		} else if (moving.color == BLACK && source_row == 0 && source_col == 0) {
			castling_.black_queen_side = false;
		} else if (moving.color == BLACK && source_row == 0 && source_col == 7) {
			castling_.black_king_side = false;
		}
	}
	if (captured && captured->type == ROOK) {
		if (captured->color == WHITE && captured->row == 7 && captured->column == 0) {
			castling_.white_queen_side = false;
		} else if (captured->color == WHITE && captured->row == 7
			&& captured->column == 7) {
			castling_.white_king_side = false;
		} else if (captured->color == BLACK && captured->row == 0
			&& captured->column == 0) {
			castling_.black_queen_side = false;
		} else if (captured->color == BLACK && captured->row == 0
			&& captured->column == 7) {
			castling_.black_king_side = false;
		}
	}
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
	const PieceHistory & move = *undo_once_;
	board_->TakePiece(move.Get_E_Row(), move.Get_E_Column());
	if (move.Kind() == MoveKind::Castling) {
		auto rook = board_->TakePiece(
			move.RookEnd()->GetRow(), move.RookEnd()->GetColumn());
		if (!rook || rook->GetType() != ROOK) {
			throw std::logic_error("cannot undo corrupt castling history");
		}
		board_->PutPiece(
			std::move(rook),
			move.RookStart()->GetRow(),
			move.RookStart()->GetColumn());
	}
	board_->PutPiece(
		MakePiece(move.GetMovingSnapshot()),
		move.Get_S_Row(),
		move.Get_S_Column());
	if (move.GetAttackSnapshot()) {
		board_->PutPiece(
			MakePiece(*move.GetAttackSnapshot()),
			move.GetAttackSnapshot()->row,
			move.GetAttackSnapshot()->column);
	}
	castling_ = move.CastlingBefore();
	en_passant_ = move.EnPassantBefore();
	halfmove_clock_ = move.HalfmoveBefore();
	fullmove_number_ = move.FullmoveBefore();
	turn_ = move.TurnBefore();
	if (position_keys_.size() > 1) {
		position_keys_.pop_back();
	} else {
		position_keys_.assign(1, PositionKey());
	}
	current_piece_ = nullptr;
	valid_moves_.clear();
	UpdateStatus();
	return &*undo_once_;
}

bool GameFacade::Check(int row, int col)
{
	if (!ChessBoard::IsValidPosition(row, col)) {
		throw std::out_of_range("board coordinates must be between 0 and 7");
	}
	return IsInCheck(turn_);
}

bool GameFacade::Mate(int row, int col)
{
	if (!ChessBoard::IsValidPosition(row, col)) {
		throw std::out_of_range("board coordinates must be between 0 and 7");
	}
	return !HasLegalMove(turn_);
}

void GameFacade::UpdateStatus()
{
	const bool check = IsInCheck(turn_);
	if (!HasLegalMove(turn_)) {
		status_ = check ? GameStatus::Checkmate : GameStatus::Stalemate;
		return;
	}
	if (halfmove_clock_ >= 100) {
		status_ = GameStatus::DrawFiftyMove;
		return;
	}
	const std::string key = PositionKey();
	if (std::count(position_keys_.begin(), position_keys_.end(), key) >= 3) {
		status_ = GameStatus::DrawThreefold;
		return;
	}
	if (IsInsufficientMaterial()) {
		status_ = GameStatus::DrawInsufficientMaterial;
		return;
	}
	status_ = check ? GameStatus::Check : GameStatus::Ongoing;
}

bool GameFacade::IsInsufficientMaterial() const
{
	std::vector<const Piece *> non_kings;
	for (int row = 0; row < ChessBoard::Size; ++row) {
		for (int col = 0; col < ChessBoard::Size; ++col) {
			const Piece * piece = board_->GetPiece(row, col);
			if (piece && piece->GetType() != KING) {
				non_kings.push_back(piece);
			}
		}
	}
	if (non_kings.empty()) {
		return true;
	}
	if (non_kings.size() == 1) {
		return non_kings.front()->GetType() == BISHOP
			|| non_kings.front()->GetType() == KNIGHT;
	}
	if (std::all_of(
			non_kings.begin(), non_kings.end(),
			[](const Piece * piece) { return piece->GetType() == BISHOP; })) {
		const int square_color =
			(non_kings.front()->GetRow() + non_kings.front()->GetColumn()) % 2;
		return std::all_of(
			non_kings.begin(), non_kings.end(),
			[square_color](const Piece * piece) {
				return (piece->GetRow() + piece->GetColumn()) % 2 == square_color;
			});
	}
	return false;
}

std::string GameFacade::PositionKey() const
{
	RequireBoard();
	std::ostringstream key;
	for (int row = 0; row < ChessBoard::Size; ++row) {
		for (int col = 0; col < ChessBoard::Size; ++col) {
			const Piece * piece = board_->GetPiece(row, col);
			key << (piece ? PieceCode(*piece) : '.');
		}
	}
	key << (turn_ == WHITE ? 'w' : 'b')
		<< (castling_.white_king_side ? 'K' : '-')
		<< (castling_.white_queen_side ? 'Q' : '-')
		<< (castling_.black_king_side ? 'k' : '-')
		<< (castling_.black_queen_side ? 'q' : '-');
	if (en_passant_) {
		key << en_passant_->GetRow() << en_passant_->GetColumn();
	} else {
		key << "--";
	}
	return key.str();
}

bool GameFacade::IsTerminal(GameStatus status) noexcept
{
	return status == GameStatus::Checkmate
		|| status == GameStatus::Stalemate
		|| status == GameStatus::DrawThreefold
		|| status == GameStatus::DrawFiftyMove
		|| status == GameStatus::DrawInsufficientMaterial;
}

void GameFacade::SaveGame(const std::string & file_name)
{
	RequireBoard();
	if (file_name.empty()) {
		throw std::invalid_argument("save path must not be empty");
	}
	Xml(file_name).WriteIntoFile(ExportState());
	file_name_ = file_name;
}

void GameFacade::SaveAs(const std::string & file_name)
{
	SaveGame(file_name);
}

bool GameFacade::LoadGame(const std::string & file_name)
{
	if (file_name.empty()) {
		throw std::invalid_argument("load path must not be empty");
	}
	SavedGameState state = Xml(file_name).ReadFromFile();
	ImportState(std::move(state));
	file_name_ = file_name;
	return true;
}

SavedGameState GameFacade::ExportState() const
{
	RequireBoard();
	SavedGameState state;
	for (int row = 0; row < ChessBoard::Size; ++row) {
		for (int col = 0; col < ChessBoard::Size; ++col) {
			if (const Piece * piece = board_->GetPiece(row, col)) {
				state.pieces.push_back(PieceSnapshot::FromPiece(*piece));
			}
		}
	}
	state.turn = turn_;
	state.castling = castling_;
	state.en_passant = en_passant_;
	state.halfmove_clock = halfmove_clock_;
	state.fullmove_number = fullmove_number_;
	state.history = move_history_;
	state.position_keys = position_keys_;
	return state;
}

void GameFacade::ImportState(SavedGameState state)
{
	if (state.version != 1 && state.version != 2) {
		throw std::invalid_argument("unsupported chess save version");
	}
	if (state.fullmove_number == 0) {
		throw std::invalid_argument("fullmove number must be positive");
	}

	const unsigned loaded_version = state.version;
	GameFacade candidate;
	candidate.board_->ClearBoard();
	std::array<std::array<bool, ChessBoard::Size>, ChessBoard::Size> occupied{};
	std::array<int, 2> kings{};
	for (const PieceSnapshot & piece : state.pieces) {
		if (!ChessBoard::IsValidPosition(piece.row, piece.column)) {
			throw std::invalid_argument("save contains an out-of-range piece");
		}
		if (occupied[piece.row][piece.column]) {
			throw std::invalid_argument("save contains overlapping pieces");
		}
		occupied[piece.row][piece.column] = true;
		if (piece.type == KING) {
			++kings.at(static_cast<std::size_t>(piece.color));
		}
		candidate.board_->PutPiece(MakePiece(piece), piece.row, piece.column);
	}
	if (kings[WHITE] != 1 || kings[BLACK] != 1) {
		throw std::invalid_argument("save must contain exactly one king per side");
	}
	candidate.turn_ = state.turn;
	candidate.castling_ = state.castling;
	candidate.en_passant_ = state.en_passant;
	candidate.halfmove_clock_ = state.halfmove_clock;
	candidate.fullmove_number_ = state.fullmove_number;
	candidate.move_history_ = std::move(state.history);
	candidate.position_keys_ = std::move(state.position_keys);
	ValidateHistory(*candidate.board_, candidate.move_history_);
	if (loaded_version == 2
		&& candidate.position_keys_.size() != candidate.move_history_.size() + 1) {
		throw std::invalid_argument(
			"repetition history length does not match move history");
	}
	if (candidate.position_keys_.empty()) {
		candidate.position_keys_.push_back(candidate.PositionKey());
	} else if (candidate.position_keys_.back() != candidate.PositionKey()) {
		throw std::invalid_argument(
			"repetition history does not match the saved position");
	}
	candidate.current_piece_ = nullptr;
	candidate.valid_moves_.clear();
	candidate.undo_once_.reset();
	candidate.file_name_.clear();
	candidate.UpdateStatus();
	*this = std::move(candidate);
}

void GameFacade::SetPosition(
	std::vector<PieceSnapshot> pieces,
	PieceColor turn,
	CastlingRights castling,
	std::optional<BoardPosition> en_passant,
	unsigned halfmove_clock,
	unsigned fullmove_number)
{
	SavedGameState state;
	state.version = 1;
	state.pieces = std::move(pieces);
	state.turn = turn;
	state.castling = castling;
	state.en_passant = en_passant;
	state.halfmove_clock = halfmove_clock;
	state.fullmove_number = fullmove_number;
	ImportState(std::move(state));
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

std::size_t GameFacade::HistorySize() const noexcept { return move_history_.size(); }

const ChessBoard & GameFacade::Board() const
{
	RequireBoard();
	return *board_;
}

PieceColor GameFacade::Turn() const noexcept { return turn_; }
GameStatus GameFacade::Status() const noexcept { return status_; }
bool GameFacade::IsGameOver() const noexcept { return IsTerminal(status_); }
const CastlingRights & GameFacade::Castling() const noexcept { return castling_; }
const std::optional<BoardPosition> & GameFacade::EnPassantTarget() const noexcept
{
	return en_passant_;
}
unsigned GameFacade::HalfmoveClock() const noexcept { return halfmove_clock_; }
unsigned GameFacade::FullmoveNumber() const noexcept { return fullmove_number_; }
const std::string & GameFacade::CurrentFile() const noexcept { return file_name_; }

bool GameFacade::Test(std::ostream & os)
{
	using std::endl;
	bool success = true;
	GameFacade game;
	TEST(game.GetPiece(6, 4, WHITE)->GetType() == PAWN);
	game.MovePiece(6, 4, 4, 4);
	TEST(game.Turn() == BLACK);
	TEST(game.HistorySize() == 1);
	TEST(game.Undo() != nullptr);
	TEST(game.Turn() == WHITE);
	TEST(game.Board().GetPiece(6, 4)->GetType() == PAWN);
	os << "\tGameFacade rule-state tests passed\n";
	return success;
}
