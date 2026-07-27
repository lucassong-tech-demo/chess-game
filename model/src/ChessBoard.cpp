#include "ChessBoard.h"

#include <stdexcept>
#include <utility>

#include "Bishop.h"
#include "King.h"
#include "Knight.h"
#include "Pawn.h"
#include "Queen.h"
#include "Rook.h"
#include "UnitTest.h"

namespace {

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

} // namespace

ChessBoard::ChessBoard()
{
	InitializePieces();
}

ChessBoard::ChessBoard(const ChessBoard & other)
{
	ClearBoard();
	for (int row = 0; row < Size; ++row) {
		for (int col = 0; col < Size; ++col) {
			if (const Piece * piece = other.GetPiece(row, col)) {
				board_[row][col] = MakePiece(
					piece->GetType(), piece->GetColor(), row, col);
			}
		}
	}
}

ChessBoard & ChessBoard::operator=(const ChessBoard & other)
{
	if (this != &other) {
		ChessBoard copy(other);
		board_ = std::move(copy.board_);
	}
	return *this;
}

bool ChessBoard::IsValidPosition(int row, int col) noexcept
{
	return row >= 0 && row < Size && col >= 0 && col < Size;
}

void ChessBoard::ValidatePosition(int row, int col)
{
	if (!IsValidPosition(row, col)) {
		throw std::out_of_range("board coordinates must be between 0 and 7");
	}
}

void ChessBoard::ClearBoard() noexcept
{
	for (auto & board_row : board_) {
		for (auto & piece : board_row) {
			piece.reset();
		}
	}
}

void ChessBoard::Reset()
{
	ClearBoard();
	InitializePieces();
}

void ChessBoard::InitializePieces()
{
	constexpr std::array<PieceType, Size> back_rank{
		ROOK, KNIGHT, BISHOP, QUEEN, KING, BISHOP, KNIGHT, ROOK
	};

	for (int col = 0; col < Size; ++col) {
		board_[0][col] = MakePiece(back_rank[col], BLACK, 0, col);
		board_[1][col] = MakePiece(PAWN, BLACK, 1, col);
		board_[6][col] = MakePiece(PAWN, WHITE, 6, col);
		board_[7][col] = MakePiece(back_rank[col], WHITE, 7, col);
	}
}

Piece * ChessBoard::GetPiece(int row, int col)
{
	return const_cast<Piece *>(std::as_const(*this).GetPiece(row, col));
}

const Piece * ChessBoard::GetPiece(int row, int col) const
{
	ValidatePosition(row, col);
	return board_[row][col].get();
}

Piece * ChessBoard::GetKing(PieceColor color)
{
	return const_cast<Piece *>(std::as_const(*this).GetKing(color));
}

const Piece * ChessBoard::GetKing(PieceColor color) const
{
	for (const auto & board_row : board_) {
		for (const auto & piece : board_row) {
			if (piece && piece->GetType() == KING && piece->GetColor() == color) {
				return piece.get();
			}
		}
	}
	return nullptr;
}

std::unique_ptr<Piece> ChessBoard::MovePiece(
	int source_row,
	int source_col,
	int destination_row,
	int destination_col)
{
	ValidatePosition(source_row, source_col);
	ValidatePosition(destination_row, destination_col);
	if (source_row == destination_row && source_col == destination_col) {
		throw std::invalid_argument("source and destination must differ");
	}
	auto & source = board_[source_row][source_col];
	if (!source) {
		throw std::invalid_argument("cannot move from an empty board cell");
	}

	auto captured_piece = std::move(board_[destination_row][destination_col]);
	board_[destination_row][destination_col] = std::move(source);
	board_[destination_row][destination_col]->Update_location(destination_row, destination_col);
	return captured_piece;
}

void ChessBoard::RestoreMove(
	int source_row,
	int source_col,
	int destination_row,
	int destination_col,
	std::unique_ptr<Piece> captured_piece)
{
	ValidatePosition(source_row, source_col);
	ValidatePosition(destination_row, destination_col);
	if (source_row == destination_row && source_col == destination_col) {
		throw std::invalid_argument("source and destination must differ");
	}
	auto & destination = board_[destination_row][destination_col];
	if (!destination) {
		throw std::invalid_argument("cannot restore a move from an empty destination");
	}
	if (board_[source_row][source_col]) {
		throw std::invalid_argument("cannot restore a move onto an occupied source");
	}

	board_[source_row][source_col] = std::move(destination);
	board_[source_row][source_col]->Update_location(source_row, source_col);
	if (captured_piece) {
		captured_piece->Update_location(destination_row, destination_col);
	}
	destination = std::move(captured_piece);
}

void ChessBoard::PutPiece(std::unique_ptr<Piece> piece, int row, int col)
{
	ValidatePosition(row, col);
	if (!piece) {
		throw std::invalid_argument("cannot put a null piece on the board");
	}
	piece->Update_location(row, col);
	board_[row][col] = std::move(piece);
}

std::unique_ptr<Piece> ChessBoard::TakePiece(int row, int col)
{
	ValidatePosition(row, col);
	return std::move(board_[row][col]);
}

bool ChessBoard::Test(std::ostream & os)
{
	using std::endl;
	bool success = true;
	ChessBoard board;
	TEST(board.GetPiece(1, 0)->GetType() == PAWN);
	TEST(board.GetPiece(1, 0)->GetColor() == BLACK);
	TEST(board.GetPiece(2, 1) == nullptr);

	auto captured = board.MovePiece(1, 0, 2, 1);
	TEST(captured == nullptr);
	TEST(board.GetPiece(1, 0) == nullptr);
	TEST(board.GetPiece(2, 1)->GetType() == PAWN);

	captured = board.MovePiece(0, 1, 2, 1);
	TEST(captured != nullptr);
	TEST(captured->GetType() == PAWN);
	TEST(board.GetPiece(2, 1)->GetType() == KNIGHT);
	board.RestoreMove(0, 1, 2, 1, std::move(captured));
	TEST(board.GetPiece(0, 1)->GetType() == KNIGHT);
	TEST(board.GetPiece(2, 1)->GetType() == PAWN);

	board.ClearBoard();
	TEST(board.GetPiece(2, 1) == nullptr);
	os << "\tChessBoard ownership tests passed\n";
	return success;
}
