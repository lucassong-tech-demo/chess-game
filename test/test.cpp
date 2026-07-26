#include <cstddef>
#include <exception>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>

#include "Bishop.h"
#include "ChessBoard.h"
#include "GameFacade.h"
#include "King.h"
#include "Knight.h"
#include "Pawn.h"
#include "Queen.h"
#include "Rook.h"

namespace {

int failures = 0;

void Check(bool condition, const std::string & message)
{
	if (!condition) {
		++failures;
		std::cerr << "FAIL: " << message << '\n';
	}
}

template<typename Exception, typename Function>
void CheckThrows(Function && function, const std::string & message)
{
	try {
		function();
		Check(false, message + " (no exception)");
	} catch (const Exception &) {
		// Expected.
	} catch (const std::exception & exception) {
		Check(false, message + " (wrong exception: " + exception.what() + ")");
	}
}

std::size_t CountPieces(const ChessBoard & board)
{
	std::size_t count = 0;
	for (int row = 0; row < ChessBoard::Size; ++row) {
		for (int col = 0; col < ChessBoard::Size; ++col) {
			if (board.GetPiece(row, col)) {
				++count;
			}
		}
	}
	return count;
}

bool HasMove(const std::set<BoardPosition> & moves, int row, int col)
{
	return moves.find(BoardPosition(row, col)) != moves.end();
}

void TestLegacyPieceRules()
{
	ChessBoard board;
	board.ClearBoard();

	board.PutPiece(std::make_unique<Rook>(ROOK, WHITE, 4, 4), 4, 4);
	board.PutPiece(std::make_unique<Pawn>(PAWN, WHITE, 2, 4), 2, 4);
	board.PutPiece(std::make_unique<Pawn>(PAWN, BLACK, 4, 6), 4, 6);
	auto moves = board.GetPiece(4, 4)->GetValidMove(board);
	Check(moves.size() == 10, "rook keeps the legacy orthogonal ray rules");
	Check(!HasMove(moves, 2, 4), "rook cannot capture a friendly blocker");
	Check(HasMove(moves, 4, 6) && !HasMove(moves, 4, 7),
		"rook captures an enemy blocker and stops");

	board.ClearBoard();
	board.PutPiece(std::make_unique<Bishop>(BISHOP, WHITE, 4, 4), 4, 4);
	board.PutPiece(std::make_unique<Pawn>(PAWN, WHITE, 2, 2), 2, 2);
	board.PutPiece(std::make_unique<Pawn>(PAWN, BLACK, 2, 6), 2, 6);
	moves = board.GetPiece(4, 4)->GetValidMove(board);
	Check(moves.size() == 9, "bishop keeps the legacy diagonal ray rules");
	Check(!HasMove(moves, 2, 2), "bishop cannot capture a friendly blocker");
	Check(HasMove(moves, 2, 6) && !HasMove(moves, 1, 7),
		"bishop captures an enemy blocker and stops");

	board.ClearBoard();
	board.PutPiece(std::make_unique<Queen>(QUEEN, WHITE, 4, 4), 4, 4);
	moves = board.GetPiece(4, 4)->GetValidMove(board);
	Check(moves.size() == 27, "queen keeps the combined rook and bishop rules");

	board.ClearBoard();
	board.PutPiece(std::make_unique<Knight>(KNIGHT, WHITE, 4, 4), 4, 4);
	board.PutPiece(std::make_unique<Pawn>(PAWN, WHITE, 2, 3), 2, 3);
	board.PutPiece(std::make_unique<Pawn>(PAWN, BLACK, 2, 5), 2, 5);
	moves = board.GetPiece(4, 4)->GetValidMove(board);
	Check(moves.size() == 7, "knight keeps all eight legacy offsets except friendly cells");
	Check(!HasMove(moves, 2, 3) && HasMove(moves, 2, 5),
		"knight preserves friendly blocking and enemy capture behavior");

	board.ClearBoard();
	board.PutPiece(std::make_unique<King>(KING, WHITE, 4, 4), 4, 4);
	board.PutPiece(std::make_unique<Pawn>(PAWN, WHITE, 3, 3), 3, 3);
	board.PutPiece(std::make_unique<Pawn>(PAWN, BLACK, 3, 4), 3, 4);
	moves = board.GetPiece(4, 4)->GetValidMove(board);
	Check(moves.size() == 7, "king keeps the legacy one-square neighborhood");
	Check(!HasMove(moves, 3, 3) && HasMove(moves, 3, 4),
		"king preserves friendly blocking and enemy capture behavior");

	board.ClearBoard();
	board.PutPiece(std::make_unique<Pawn>(PAWN, WHITE, 6, 3), 6, 3);
	moves = board.GetPiece(6, 3)->GetValidMove(board);
	Check(moves.size() == 2 && HasMove(moves, 5, 3) && HasMove(moves, 4, 3),
		"white pawn keeps the legacy initial one-or-two-step move");
	board.PutPiece(std::make_unique<Pawn>(PAWN, WHITE, 5, 3), 5, 3);
	board.PutPiece(std::make_unique<Pawn>(PAWN, BLACK, 5, 2), 5, 2);
	board.PutPiece(std::make_unique<Pawn>(PAWN, BLACK, 5, 4), 5, 4);
	moves = board.GetPiece(6, 3)->GetValidMove(board);
	Check(moves.size() == 2 && HasMove(moves, 5, 2) && HasMove(moves, 5, 4),
		"white pawn keeps diagonal captures and forward blocking");

	board.ClearBoard();
	board.PutPiece(std::make_unique<Pawn>(PAWN, BLACK, 1, 3), 1, 3);
	moves = board.GetPiece(1, 3)->GetValidMove(board);
	Check(moves.size() == 2 && HasMove(moves, 2, 3) && HasMove(moves, 3, 3),
		"black pawn keeps the legacy initial one-or-two-step move");
}

void TestBoardOwnership()
{
	ChessBoard board;
	Check(CountPieces(board) == 32, "a board starts with 32 owned pieces");
	Check(board.GetKing(WHITE) == board.GetPiece(7, 4), "white king is discovered on the board");
	Check(board.GetKing(BLACK) == board.GetPiece(0, 4), "black king is discovered on the board");

	auto captured = board.MovePiece(7, 6, 5, 5);
	Check(!captured, "an ordinary move does not transfer a captured piece");
	Check(board.GetPiece(7, 6) == nullptr, "ordinary move clears source");
	Check(board.GetPiece(5, 5)->GetType() == KNIGHT, "ordinary move transfers piece ownership");
	board.RestoreMove(7, 6, 5, 5);
	Check(board.GetPiece(7, 6)->GetType() == KNIGHT, "restore reverses an ordinary move");

	captured = board.MovePiece(7, 6, 1, 1);
	Check(captured && captured->GetType() == PAWN, "capture transfers victim ownership");
	Check(CountPieces(board) == 31, "captured piece is no longer board-owned");
	board.RestoreMove(7, 6, 1, 1, std::move(captured));
	Check(CountPieces(board) == 32, "restore returns captured ownership to board");

	board.ClearBoard();
	Check(CountPieces(board) == 0, "ClearBoard releases every piece");
	Check(board.GetKing(WHITE) == nullptr, "cleared board has no stale king observer");
	board.ClearBoard();
	Check(CountPieces(board) == 0, "ClearBoard is idempotent");
	board.Reset();
	Check(CountPieces(board) == 32, "Reset repopulates a cleared board");
}

void TestMoveValues()
{
	ChessBoard board;
	const Piece * knight = board.GetPiece(7, 6);
	auto first = knight->GetValidMove(board);
	auto second = knight->GetValidMove(board);
	Check(first.size() == 2 && second.size() == 2, "repeated move generation returns complete sets");
	first.clear();
	Check(second.size() == 2, "move sets have independent value lifetimes");

	GameFacade game;
	game.NewGame();
	Check(game.GetPiece(7, 6, WHITE) != nullptr, "facade selects an owned piece");
	auto & facade_first = game.GetValidMoves();
	Check(facade_first.size() == 2, "facade exposes generated moves");
	auto * stable_address = &facade_first;
	auto & facade_second = game.GetValidMoves();
	Check(&facade_second == stable_address, "facade move storage has a stable member lifetime");
	Check(facade_second.size() == 2, "facade move storage refreshes without allocation");
}

void TestMovesCapturesAndUndo()
{
	GameFacade game;
	game.NewGame();
	game.MovePiece(7, 6, 5, 5);
	Check(game.HistorySize() == 1, "ordinary move is recorded by value");
	const PieceHistory * ordinary = game.Undo();
	Check(ordinary != nullptr, "ordinary move can be undone");
	Check(!ordinary->IsAttackPieceHere(), "ordinary history has no capture snapshot");
	Check(game.Board().GetPiece(7, 6)->GetType() == KNIGHT, "ordinary undo restores mover");
	Check(game.Board().GetPiece(5, 5) == nullptr, "ordinary undo clears destination");
	Check(game.HistorySize() == 0, "ordinary undo pops history");
	Check(game.Undo() == nullptr, "undo on empty history is safe");

	game.MovePiece(7, 6, 1, 1);
	Check(game.HistorySize() == 1, "capture is recorded");
	const PieceHistory * capture = game.Undo();
	Check(capture && capture->IsAttackPieceHere(), "capture history owns stable victim data");
	Check(capture->GetType_Moving() == KNIGHT, "capture snapshot records mover type");
	Check(capture->GetType_Attack() == PAWN, "capture snapshot records victim type");
	Check(game.Board().GetPiece(7, 6)->GetType() == KNIGHT, "capture undo restores mover");
	Check(game.Board().GetPiece(1, 1)->GetType() == PAWN, "capture undo reconstructs victim");

	// The same mover captures twice. History must retain both independent victims.
	game.MovePiece(7, 6, 1, 1);
	game.MovePiece(1, 1, 0, 3);
	Check(game.HistorySize() == 2, "continuous captures create two history values");
	Check(game.Board().GetPiece(0, 3)->GetType() == KNIGHT, "second capture moves the same owner");
	const PieceHistory * second_capture = game.Undo();
	Check(second_capture->GetType_Attack() == QUEEN, "latest capture retains queen snapshot");
	Check(game.Board().GetPiece(1, 1)->GetType() == KNIGHT, "first undo restores intermediate square");
	Check(game.Board().GetPiece(0, 3)->GetType() == QUEEN, "first undo restores second victim");
	const PieceHistory * first_capture = game.Undo();
	Check(first_capture->GetType_Attack() == PAWN, "earlier capture retains pawn snapshot");
	Check(game.Board().GetPiece(7, 6)->GetType() == KNIGHT, "second undo restores original square");
	Check(game.Board().GetPiece(1, 1)->GetType() == PAWN, "second undo restores first victim");

	game.MovePiece(7, 6, 5, 5);
	game.MovePiece(0, 1, 2, 2);
	Check(game.HistorySize() == 2, "history accumulates moves");
	game.ClearHistory();
	Check(game.HistorySize() == 0, "ClearHistory releases all history values");
	Check(game.Undo() == nullptr, "cleared history cannot be undone");
}

void TestNewGameAndDestruction()
{
	GameFacade game;
	game.NewGame();
	game.MovePiece(7, 6, 1, 1);
	game.NewGame();
	Check(CountPieces(game.Board()) == 32, "NewGame replaces board after a capture");
	Check(game.HistorySize() == 0, "NewGame clears old history");
	Check(game.Board().GetPiece(7, 6)->GetType() == KNIGHT, "NewGame restores initial position");
	game.NewGame();
	game.NewGame();
	Check(CountPieces(game.Board()) == 32, "repeated NewGame remains idempotent");

	game.Clear_Board();
	CheckThrows<std::logic_error>(
		[&game] { static_cast<void>(game.Board()); },
		"cleared facade reports missing board");
	game.NewGame();
	Check(CountPieces(game.Board()) == 32, "NewGame works after explicit board clear");

	for (int iteration = 0; iteration < 50; ++iteration) {
		GameFacade scoped_game;
		scoped_game.NewGame();
		scoped_game.MovePiece(7, 6, 1, 1);
		scoped_game.MovePiece(1, 1, 0, 3);
	}
	Check(true, "facade destruction paths completed");
}

void TestCoordinateValidation()
{
	ChessBoard board;
	Check(!ChessBoard::IsValidPosition(-1, 0), "negative row is invalid");
	Check(!ChessBoard::IsValidPosition(0, 8), "column eight is invalid");
	CheckThrows<std::out_of_range>(
		[&board] { static_cast<void>(board.GetPiece(-1, 0)); },
		"board rejects negative coordinates");
	CheckThrows<std::out_of_range>(
		[&board] { static_cast<void>(board.GetPiece(8, 0)); },
		"board rejects coordinates above range");
	CheckThrows<std::out_of_range>(
		[&board] { board.MovePiece(7, 6, 8, 6); },
		"board move validates destination");
	CheckThrows<std::invalid_argument>(
		[&board] { board.MovePiece(4, 4, 3, 4); },
		"board rejects an empty source");
	CheckThrows<std::invalid_argument>(
		[&board] { board.MovePiece(7, 6, 7, 6); },
		"board rejects identical source and destination");
	CheckThrows<std::out_of_range>(
		[] { static_cast<void>(BoardPosition(0, -1)); },
		"BoardPosition enforces the same coordinate range");

	GameFacade game;
	game.NewGame();
	CheckThrows<std::out_of_range>(
		[&game] { static_cast<void>(game.GetPiece(0, 8, WHITE)); },
		"facade selection propagates coordinate validation");
	CheckThrows<std::out_of_range>(
		[&game] { static_cast<void>(game.isCellTaken(-1, 0)); },
		"facade occupancy query validates coordinates");
	CheckThrows<std::out_of_range>(
		[&game] { static_cast<void>(game.isValidMove(8, 0)); },
		"facade move query validates coordinates");
	CheckThrows<std::out_of_range>(
		[&game] { static_cast<void>(game.Check(-1, 0)); },
		"facade check query validates coordinates");
}

} // namespace

int main()
{
	TestLegacyPieceRules();
	TestBoardOwnership();
	TestMoveValues();
	TestMovesCapturesAndUndo();
	TestNewGameAndDestruction();
	TestCoordinateValidation();

	if (failures != 0) {
		std::cerr << failures << " test assertion(s) failed\n";
		return 1;
	}
	std::cout << "All ownership and behavior tests passed\n";
	return 0;
}
