#include <chrono>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "Bishop.h"
#include "ChessBoard.h"
#include "GameFacade.h"
#include "King.h"
#include "Knight.h"
#include "Pawn.h"
#include "Queen.h"
#include "Rook.h"

#ifndef CHESS_SOURCE_DIR
#define CHESS_SOURCE_DIR "."
#endif

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

bool HasMove(const std::set<BoardPosition> & moves, int row, int col)
{
	return moves.contains(BoardPosition(row, col));
}

PieceSnapshot PieceAt(PieceType type, PieceColor color, int row, int col)
{
	return {type, color, row, col};
}

std::string BoardSignature(const GameFacade & game)
{
	std::string result;
	for (int row = 0; row < 8; ++row) {
		for (int col = 0; col < 8; ++col) {
			const Piece * piece = game.Board().GetPiece(row, col);
			if (!piece) {
				result += '.';
				continue;
			}
			result += static_cast<char>('A' + piece->GetType());
			result += piece->GetColor() == WHITE ? 'w' : 'b';
		}
	}
	result += game.Turn() == WHITE ? 'w' : 'b';
	result += std::to_string(game.HistorySize());
	return result;
}

std::filesystem::path TempPath(const std::string & name)
{
	const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
	return std::filesystem::temp_directory_path()
		/ ("chess-phase4-" + std::to_string(stamp) + "-" + name);
}

void WriteText(const std::filesystem::path & path, const std::string & text)
{
	std::ofstream file(path);
	file << text;
}

void TestPieceBoundaries()
{
	ChessBoard board;
	board.ClearBoard();
	board.PutPiece(std::make_unique<Rook>(ROOK, WHITE, 4, 4), 4, 4);
	board.PutPiece(std::make_unique<Pawn>(PAWN, WHITE, 2, 4), 2, 4);
	board.PutPiece(std::make_unique<Pawn>(PAWN, BLACK, 4, 6), 4, 6);
	auto moves = board.GetPiece(4, 4)->GetValidMove(board);
	Check(moves.size() == 10, "rook rays stop at board edges and blockers");
	Check(!HasMove(moves, 2, 4), "rook cannot capture a friendly blocker");
	Check(HasMove(moves, 4, 6) && !HasMove(moves, 4, 7),
		"rook can capture one enemy blocker");

	board.ClearBoard();
	board.PutPiece(std::make_unique<Bishop>(BISHOP, WHITE, 0, 0), 0, 0);
	Check(board.GetPiece(0, 0)->GetValidMove(board).size() == 7,
		"bishop has seven diagonal moves from a corner");

	board.ClearBoard();
	board.PutPiece(std::make_unique<Queen>(QUEEN, WHITE, 4, 4), 4, 4);
	Check(board.GetPiece(4, 4)->GetValidMove(board).size() == 27,
		"queen combines all unobstructed rays");

	board.ClearBoard();
	board.PutPiece(std::make_unique<Knight>(KNIGHT, WHITE, 0, 0), 0, 0);
	Check(board.GetPiece(0, 0)->GetValidMove(board).size() == 2,
		"knight is clipped correctly in a corner");

	board.ClearBoard();
	board.PutPiece(std::make_unique<King>(KING, WHITE, 0, 0), 0, 0);
	Check(board.GetPiece(0, 0)->GetValidMove(board).size() == 3,
		"king is clipped correctly in a corner");

	board.ClearBoard();
	board.PutPiece(std::make_unique<Pawn>(PAWN, WHITE, 6, 3), 6, 3);
	moves = board.GetPiece(6, 3)->GetValidMove(board);
	Check(HasMove(moves, 5, 3) && HasMove(moves, 4, 3),
		"white pawn has its initial one- and two-square advances");
	board.PutPiece(std::make_unique<Pawn>(PAWN, BLACK, 5, 2), 5, 2);
	board.PutPiece(std::make_unique<Pawn>(PAWN, BLACK, 5, 4), 5, 4);
	moves = board.GetPiece(6, 3)->GetValidMove(board);
	Check(HasMove(moves, 5, 2) && HasMove(moves, 5, 4),
		"pawn captures diagonally at both board-safe offsets");
}

void TestChessBoardOwnership()
{
	ChessBoard board;
	Check(board.GetPiece(1, 0)
			&& board.GetPiece(1, 0)->GetType() == PAWN
			&& board.GetPiece(1, 0)->GetColor() == BLACK,
		"the initial board contains the expected black pawn");

	auto captured = board.MovePiece(1, 0, 2, 1);
	Check(!captured && board.GetPiece(1, 0) == nullptr
			&& board.GetPiece(2, 1)->GetType() == PAWN,
		"moving to an empty square transfers the pawn without a capture");

	captured = board.MovePiece(0, 1, 2, 1);
	Check(captured && captured->GetType() == PAWN
			&& board.GetPiece(2, 1)->GetType() == KNIGHT,
		"a capture transfers ownership of the displaced piece");
	board.RestoreMove(0, 1, 2, 1, std::move(captured));
	Check(board.GetPiece(0, 1)->GetType() == KNIGHT
			&& board.GetPiece(2, 1)->GetType() == PAWN,
		"restoring a capture returns both pieces to their original squares");

	board.ClearBoard();
	Check(board.GetPiece(2, 1) == nullptr,
		"clearing the board releases all owned pieces");
}

void TestTurnLegalityAndCheckFilter()
{
	GameFacade game;
	Check(game.Turn() == WHITE, "white starts");
	CheckThrows<std::invalid_argument>(
		[&] { game.MovePiece(1, 4, 3, 4); },
		"black cannot move on white's turn");
	CheckThrows<std::invalid_argument>(
		[&] { game.MovePiece(6, 4, 3, 4); },
		"a pawn cannot move three squares");

	game.SetPosition({
		PieceAt(KING, WHITE, 7, 4),
		PieceAt(ROOK, WHITE, 6, 4),
		PieceAt(ROOK, BLACK, 0, 4),
		PieceAt(KING, BLACK, 0, 0),
	}, WHITE, {false, false, false, false});
	const auto pinned_moves = game.LegalMoves(6, 4);
	Check(!HasMove(pinned_moves, 6, 3) && HasMove(pinned_moves, 5, 4),
		"a pinned rook may stay on the checking line but not expose its king");

	game.SetPosition({
		PieceAt(KING, WHITE, 7, 4),
		PieceAt(ROOK, BLACK, 7, 0),
		PieceAt(KING, BLACK, 0, 4),
	}, WHITE, {false, false, false, false});
	const auto king_moves = game.LegalMoves(7, 4);
	Check(!HasMove(king_moves, 7, 3),
		"a king may not move onto an attacked square");
}

void TestCheckmateStalemateAndTerminalUndo()
{
	GameFacade game;
	game.MovePiece(6, 5, 5, 5);
	game.MovePiece(1, 4, 3, 4);
	game.MovePiece(6, 6, 4, 6);
	game.MovePiece(0, 3, 4, 7);
	Check(game.Status() == GameStatus::Checkmate, "Fool's mate is detected");
	Check(game.IsGameOver(), "checkmate stops the game");
	CheckThrows<std::logic_error>(
		[&] { game.MovePiece(6, 0, 5, 0); },
		"moves are rejected after checkmate");
	Check(game.Undo() != nullptr, "undo remains available after checkmate");
	Check(!game.IsGameOver() && game.Turn() == BLACK,
		"terminal undo restores a playable turn");

	game.SetPosition({
		PieceAt(KING, BLACK, 0, 0),
		PieceAt(KING, WHITE, 2, 2),
		PieceAt(QUEEN, WHITE, 1, 2),
	}, BLACK, {false, false, false, false});
	Check(game.Status() == GameStatus::Stalemate, "known stalemate is detected");
}

void TestCastlingAndUndo()
{
	GameFacade game;
	game.SetPosition({
		PieceAt(KING, WHITE, 7, 4),
		PieceAt(ROOK, WHITE, 7, 0),
		PieceAt(ROOK, WHITE, 7, 7),
		PieceAt(KING, BLACK, 0, 4),
	}, WHITE);
	const auto moves = game.LegalMoves(7, 4);
	Check(HasMove(moves, 7, 6) && HasMove(moves, 7, 2),
		"both unobstructed castling destinations are legal");
	game.MovePiece(7, 4, 7, 6);
	Check(game.Board().GetPiece(7, 6)->GetType() == KING
			&& game.Board().GetPiece(7, 5)->GetType() == ROOK,
		"king-side castling moves both king and rook");
	Check(!game.Castling().white_king_side
			&& !game.Castling().white_queen_side,
		"moving the king clears both castling rights");
	game.Undo();
	Check(game.Board().GetPiece(7, 4)->GetType() == KING
			&& game.Board().GetPiece(7, 7)->GetType() == ROOK,
		"castling undo restores both pieces");
	Check(game.Castling().white_king_side && game.Castling().white_queen_side,
		"castling undo restores rights");

	game.SetPosition({
		PieceAt(KING, WHITE, 7, 4),
		PieceAt(ROOK, WHITE, 7, 7),
		PieceAt(KING, BLACK, 0, 0),
		PieceAt(ROOK, BLACK, 5, 5),
	}, WHITE);
	Check(!HasMove(game.LegalMoves(7, 4), 7, 6),
		"castling through an attacked square is illegal");
}

void TestEnPassantAndUndo()
{
	GameFacade game;
	game.SetPosition({
		PieceAt(KING, WHITE, 7, 4),
		PieceAt(KING, BLACK, 0, 4),
		PieceAt(PAWN, WHITE, 3, 4),
		PieceAt(PAWN, BLACK, 1, 3),
	}, BLACK, {false, false, false, false});
	game.MovePiece(1, 3, 3, 3);
	Check(game.EnPassantTarget()
			&& *game.EnPassantTarget() == BoardPosition(2, 3),
		"a double pawn move records the en passant target");
	Check(HasMove(game.LegalMoves(3, 4), 2, 3),
		"the adjacent pawn may capture en passant immediately");
	game.MovePiece(3, 4, 2, 3);
	Check(game.Board().GetPiece(3, 3) == nullptr
			&& game.Board().GetPiece(2, 3)->GetColor() == WHITE,
		"en passant removes the bypassed pawn");
	game.Undo();
	Check(game.Board().GetPiece(3, 4)->GetColor() == WHITE
			&& game.Board().GetPiece(3, 3)->GetColor() == BLACK,
		"en passant undo restores both pawns");
	Check(game.EnPassantTarget()
			&& *game.EnPassantTarget() == BoardPosition(2, 3),
		"en passant undo restores the transient target");
}

void TestPromotionAndUndo()
{
	GameFacade game;
	game.SetPosition({
		PieceAt(KING, WHITE, 7, 4),
		PieceAt(KING, BLACK, 0, 4),
		PieceAt(PAWN, WHITE, 1, 0),
	}, WHITE, {false, false, false, false});
	CheckThrows<std::invalid_argument>(
		[&] { game.MovePiece(1, 0, 0, 0); },
		"promotion cannot silently choose a piece");
	Check(game.Board().GetPiece(1, 0)->GetType() == PAWN
			&& game.HistorySize() == 0,
		"failed promotion leaves board and history unchanged");
	game.MovePiece(1, 0, 0, 0, KNIGHT);
	Check(game.Board().GetPiece(0, 0)->GetType() == KNIGHT,
		"the requested promotion piece is created");
	game.Undo();
	Check(game.Board().GetPiece(1, 0)->GetType() == PAWN
			&& game.Board().GetPiece(0, 0) == nullptr,
		"promotion undo restores the pawn");
}

void TestDrawRules()
{
	GameFacade game;
	for (int repetition = 0; repetition < 2; ++repetition) {
		game.MovePiece(7, 6, 5, 5);
		game.MovePiece(0, 6, 2, 5);
		game.MovePiece(5, 5, 7, 6);
		game.MovePiece(2, 5, 0, 6);
	}
	Check(game.Status() == GameStatus::DrawThreefold,
		"threefold repetition is detected from full position keys");
	Check(game.Undo() != nullptr && !game.IsGameOver(),
		"threefold draw can be undone");

	game.SetPosition({
		PieceAt(KING, WHITE, 7, 4),
		PieceAt(ROOK, WHITE, 7, 0),
		PieceAt(KING, BLACK, 0, 4),
		PieceAt(ROOK, BLACK, 0, 0),
	}, WHITE, {false, false, false, false}, std::nullopt, 99, 50);
	game.MovePiece(7, 0, 6, 0);
	Check(game.Status() == GameStatus::DrawFiftyMove,
		"one hundred halfmoves without pawn move or capture triggers a draw");

	game.SetPosition({
		PieceAt(KING, WHITE, 7, 4),
		PieceAt(KING, BLACK, 0, 4),
	}, WHITE, {false, false, false, false});
	Check(game.Status() == GameStatus::DrawInsufficientMaterial,
		"king versus king is insufficient material");
}

void TestSaveRoundTripAndAtomicFailures()
{
	const auto save_path = TempPath("roundtrip.xml");
	const auto en_passant_path = TempPath("en-passant.xml");
	const auto promotion_path = TempPath("promotion.xml");
	const auto malformed_path = TempPath("malformed.xml");
	const auto truncated_path = TempPath("truncated.xml");
	const auto unknown_path = TempPath("unknown.xml");
	GameFacade game;
	game.SetPosition({
		PieceAt(KING, WHITE, 7, 4),
		PieceAt(ROOK, WHITE, 7, 7),
		PieceAt(KING, BLACK, 0, 4),
	}, WHITE);
	game.MovePiece(7, 4, 7, 6);
	game.SaveAs(save_path.string());

	std::ifstream written(save_path);
	std::string first_line;
	std::getline(written, first_line);
	Check(first_line.starts_with("<?xml"), "save output is legal XML, not legacy '-<' text");

	GameFacade loaded;
	loaded.MovePiece(6, 4, 4, 4);
	loaded.LoadGame(save_path.string());
	Check(BoardSignature(loaded) == BoardSignature(game),
		"round-trip restores board, turn, and history");
	Check(loaded.CurrentFile() == save_path.string(),
		"load records the current file path");
	loaded.Undo();
	Check(loaded.Board().GetPiece(7, 4)->GetType() == KING
			&& loaded.Board().GetPiece(7, 7)->GetType() == ROOK,
		"loaded special-move history remains undoable");

	GameFacade en_passant;
	en_passant.SetPosition({
		PieceAt(KING, WHITE, 7, 4),
		PieceAt(KING, BLACK, 0, 4),
		PieceAt(PAWN, WHITE, 3, 4),
		PieceAt(PAWN, BLACK, 1, 3),
	}, BLACK, {false, false, false, false});
	en_passant.MovePiece(1, 3, 3, 3);
	en_passant.SaveAs(en_passant_path.string());
	GameFacade loaded_en_passant;
	loaded_en_passant.LoadGame(en_passant_path.string());
	Check(loaded_en_passant.EnPassantTarget()
			&& *loaded_en_passant.EnPassantTarget() == BoardPosition(2, 3),
		"round-trip restores the transient en passant target");
	loaded_en_passant.MovePiece(3, 4, 2, 3);
	loaded_en_passant.SaveGame(loaded_en_passant.CurrentFile());
	GameFacade reloaded_en_passant;
	reloaded_en_passant.LoadGame(en_passant_path.string());
	reloaded_en_passant.Undo();
	Check(reloaded_en_passant.Board().GetPiece(3, 4)
			&& reloaded_en_passant.Board().GetPiece(3, 3),
		"round-trip preserves en passant history for exact undo");

	GameFacade promotion;
	promotion.SetPosition({
		PieceAt(KING, WHITE, 7, 4),
		PieceAt(KING, BLACK, 0, 4),
		PieceAt(PAWN, WHITE, 1, 0),
	}, WHITE, {false, false, false, false});
	promotion.MovePiece(1, 0, 0, 0, BISHOP);
	promotion.SaveAs(promotion_path.string());
	GameFacade loaded_promotion;
	loaded_promotion.LoadGame(promotion_path.string());
	Check(loaded_promotion.Board().GetPiece(0, 0)->GetType() == BISHOP,
		"round-trip retains the chosen promotion type");
	loaded_promotion.Undo();
	Check(loaded_promotion.Board().GetPiece(1, 0)->GetType() == PAWN,
		"round-trip preserves promotion history for exact undo");

	const std::string before_failure = BoardSignature(loaded);
	WriteText(malformed_path, "<chessgame><board></chessgame>");
	WriteText(truncated_path, "<?xml version=\"1.0\"?><chessgame version=\"2\">");
	WriteText(
		unknown_path,
		"<?xml version=\"1.0\"?><chessgame version=\"99\"></chessgame>");
	for (const auto & bad : {malformed_path, truncated_path, unknown_path}) {
		CheckThrows<std::invalid_argument>(
			[&] { loaded.LoadGame(bad.string()); },
			"malformed, truncated, or unknown-version XML is rejected");
		Check(BoardSignature(loaded) == before_failure,
			"failed load atomically preserves the current game");
	}
	CheckThrows<std::runtime_error>(
		[&] { loaded.LoadGame(TempPath("missing.xml").string()); },
		"a missing load path reports an error");
	Check(BoardSignature(loaded) == before_failure,
		"missing-file load preserves the current game");
	CheckThrows<std::runtime_error>(
		[&] {
			loaded.SaveAs(
				(TempPath("missing-parent") / "game.xml").string());
		},
		"an unwritable/nonexistent save parent reports an error");
	Check(BoardSignature(loaded) == before_failure,
		"failed save does not damage the game");

	std::filesystem::remove(save_path);
	std::filesystem::remove(en_passant_path);
	std::filesystem::remove(promotion_path);
	std::filesystem::remove(malformed_path);
	std::filesystem::remove(truncated_path);
	std::filesystem::remove(unknown_path);
}

void TestLegacyFixtureAndNewGame()
{
	GameFacade game;
	game.LoadGame(std::string(CHESS_SOURCE_DIR) + "/try.xml");
	Check(game.HistorySize() == 4, "legacy repository XML history is loaded");
	Check(game.Board().GetPiece(4, 7)
			&& game.Board().GetPiece(4, 7)->GetType() == QUEEN,
		"legacy repository XML board is loaded");
	game.Undo();
	Check(game.Board().GetPiece(0, 3)
			&& game.Board().GetPiece(0, 3)->GetType() == QUEEN,
		"legacy history can be undone");
	game.NewGame();
	Check(game.Turn() == WHITE && game.HistorySize() == 0
			&& game.CurrentFile().empty()
			&& game.Board().GetPiece(6, 4)->GetType() == PAWN,
		"new game fully resets board, turn, history, and file");
}

} // namespace

int main()
{
	TestPieceBoundaries();
	TestChessBoardOwnership();
	TestTurnLegalityAndCheckFilter();
	TestCheckmateStalemateAndTerminalUndo();
	TestCastlingAndUndo();
	TestEnPassantAndUndo();
	TestPromotionAndUndo();
	TestDrawRules();
	TestSaveRoundTripAndAtomicFailures();
	TestLegacyFixtureAndNewGame();

	if (failures != 0) {
		std::cerr << failures << " core test assertion(s) failed\n";
		return 1;
	}
	std::cout << "All phase-four core tests passed\n";
	return 0;
}
