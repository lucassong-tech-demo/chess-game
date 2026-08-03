#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>

#include "ChessPresentation.h"
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

bool HasMove(const ChessSession & session, int row, int col)
{
	return session.LegalMoves().contains(BoardPosition(row, col));
}

void TestBoardLegend()
{
	const std::string_view legend = ChessPresentation::BoardLegend;
	const auto mentions = [legend](std::string_view text) {
		return legend.find(text) != std::string_view::npos;
	};
	Check(mentions("selected piece") && mentions("Gold"),
		"the board legend explains the selected-square treatment");
	Check(mentions("green squares") && mentions("legal moves"),
		"the board legend explains ordinary legal destinations");
	Check(mentions("red borders") && mentions("captures"),
		"the board legend explains capture destinations");
	Check(!mentions("highlighted square"),
		"the board legend avoids an unexplained highlighted-square instruction");
}

std::filesystem::path TempPath(const std::string & name)
{
	const auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
	return std::filesystem::temp_directory_path()
		/ ("chess-session-" + std::to_string(stamp) + "-" + name);
}

void TestSelectionTurnsUndoAndNew()
{
	ChessSession session;
	Check(session.Turn() == WHITE, "white starts");
	Check(
		session.SelectCell(6, 4) == ChessSession::Interaction::SelectionChanged,
		"clicking the current side selects a piece");
	Check(HasMove(session, 5, 4) && HasMove(session, 4, 4),
		"selection exposes only legal destinations");
	Check(session.Selected()
			&& session.Selected()->GetRow() == 6
			&& session.Selected()->GetColumn() == 4,
		"a selected piece retains its light-square source position");
	Check(!session.IsCaptureTarget(6, 4),
		"the selected source is not presented as a capture destination");

	Check(
		session.SelectCell(6, 1) == ChessSession::Interaction::SelectionChanged,
		"clicking another current-side piece changes the selection");
	Check(session.Selected()
			&& session.Selected()->GetRow() == 6
			&& session.Selected()->GetColumn() == 1,
		"a selected piece retains its dark-square source position");
	Check(!HasMove(session, 6, 1) && !session.IsCaptureTarget(6, 1),
		"the selected source remains distinct from legal and capture targets");

	session.SelectCell(6, 4);
	Check(
		session.SelectCell(1, 0) == ChessSession::Interaction::Ignored,
		"an invalid target is ignored");
	Check(!session.Selected() && session.LegalMoves().empty(),
		"an invalid target clears selection and highlights");

	session.SelectCell(6, 4);
	Check(
		session.SelectCell(4, 4) == ChessSession::Interaction::MoveCompleted,
		"a highlighted destination completes a move");
	Check(session.Turn() == BLACK, "a completed move changes turn");
	Check(!session.Selected(), "a completed move clears selection");

	session.SelectCell(1, 3);
	session.SelectCell(3, 3);
	session.SelectCell(4, 4);
	Check(session.IsCaptureTarget(3, 3), "capture destinations are marked");
	session.SelectCell(3, 3);
	Check(session.Board().GetPiece(3, 3)->GetColor() == WHITE,
		"capture updates the core board");
	Check(session.Undo(), "capture can be undone");
	Check(session.Turn() == WHITE, "undo restores the moving side's turn");
	Check(session.Board().GetPiece(4, 4)->GetColor() == WHITE
			&& session.Board().GetPiece(3, 3)->GetColor() == BLACK,
		"undo restores mover and victim");

	session.SelectCell(4, 4);
	session.NewGame();
	Check(session.Turn() == WHITE
			&& session.Board().GetPiece(6, 4)->GetColor() == WHITE,
		"new game restores the initial turn and board");
	Check(!session.Selected() && session.CurrentFile().empty(),
		"new game clears interaction and current file");
}

void TestPromotionWorkflow()
{
	const auto setup_path = TempPath("promotion.xml");
	GameFacade setup;
	setup.SetPosition({
		{KING, WHITE, 7, 4},
		{KING, BLACK, 0, 4},
		{PAWN, WHITE, 1, 0},
	}, WHITE, {false, false, false, false});
	setup.SaveAs(setup_path.string());

	ChessSession session(0);
	session.Load(setup_path.string());
	session.SelectCell(1, 0);
	Check(
		session.SelectCell(0, 0) == ChessSession::Interaction::PromotionRequired,
		"reaching the final rank requests an explicit promotion choice");
	Check(session.HasPendingPromotion()
			&& session.Board().GetPiece(1, 0)->GetType() == PAWN,
		"the board is unchanged while promotion choice is pending");
	session.Promote(ROOK);
	Check(!session.HasPendingPromotion()
			&& session.Board().GetPiece(0, 0)->GetType() == ROOK,
		"the selected promotion type is applied");
	Check(session.Undo(), "a human promotion can be undone");
	Check(session.Board().GetPiece(1, 0)
			&& session.Board().GetPiece(1, 0)->GetType() == PAWN
			&& session.Board().GetPiece(0, 0) == nullptr,
		"promotion undo restores the pawn and clears the promotion square");

	session.Load(setup_path.string());
	session.SetPlayers(
		ChessSession::PlayerKind::Computer,
		ChessSession::PlayerKind::Human);
	Check(session.AdvanceComputer(), "a computer can explicitly promote");
	Check(session.Board().GetPiece(0, 0)
			&& session.Board().GetPiece(0, 0)->GetType() == QUEEN,
		"the computer promotion creates one queen");
	Check(session.Undo(), "a computer promotion can be undone");
	Check(session.Board().GetPiece(1, 0)
			&& session.Board().GetPiece(1, 0)->GetType() == PAWN
			&& session.Board().GetPiece(0, 0) == nullptr,
		"computer-promotion undo remains at the restored pawn position");
	std::filesystem::remove(setup_path);
}

void TestSaveLoadPathSemantics()
{
	const auto save_path = TempPath("save.xml");
	const auto bad_path = TempPath("bad.xml");
	ChessSession session;
	session.SelectCell(6, 4);
	CheckThrows<std::logic_error>(
		[&] { session.Save(); },
		"Save without a current path requests Save As");
	Check(!session.Selected(), "Save clears selection even when it needs a path");

	session.SaveAs(save_path.string());
	Check(session.CurrentFile() == save_path.string(),
		"Save As records the chosen path");
	session.SelectCell(6, 4);
	session.SelectCell(4, 4);
	session.Save();
	Check(session.CurrentFile() == save_path.string(),
		"Save reuses the most recent save path");

	ChessSession loaded;
	loaded.Load(save_path.string());
	Check(loaded.Board().GetPiece(4, 4)
			&& loaded.Board().GetPiece(4, 4)->GetType() == PAWN,
		"Load replaces the current board from the chosen path");
	Check(loaded.Undo() && loaded.Board().GetPiece(6, 4),
		"Load restores the saved history, which remains undoable");

	std::ofstream(bad_path) << "<broken>";
	const PieceColor turn_before = loaded.Turn();
	const std::size_t history_before =
		loaded.Board().GetPiece(6, 4) ? 1U : 0U;
	CheckThrows<std::invalid_argument>(
		[&] { loaded.Load(bad_path.string()); },
		"malformed Load reports an error");
	Check(loaded.Turn() == turn_before
			&& (loaded.Board().GetPiece(6, 4) ? 1U : 0U) == history_before,
		"failed session Load preserves the active game");

	std::filesystem::remove(save_path);
	std::filesystem::remove(bad_path);
}

void TestComputerModes()
{
	ChessSession session;
	session.SetPlayers(
		ChessSession::PlayerKind::Computer,
		ChessSession::PlayerKind::Human);
	Check(
		session.SelectCell(6, 4) == ChessSession::Interaction::Ignored,
		"human clicks are ignored on a computer turn");
	Check(session.AdvanceComputer(), "computer chooses and makes a legal move");
	Check(session.Turn() == BLACK, "computer move advances the turn");

	session.SetPlayers(
		ChessSession::PlayerKind::Human,
		ChessSession::PlayerKind::Computer);
	Check(session.AdvanceComputer(), "black computer can make a legal move");
	Check(session.Turn() == WHITE, "black computer move returns turn to white");

	session.SetPlayers(
		ChessSession::PlayerKind::Computer,
		ChessSession::PlayerKind::Computer);
	Check(session.AdvanceComputer(), "computer-vs-computer white advances");
	Check(session.AdvanceComputer(), "computer-vs-computer black advances");
}

void TestBeginnerComputerRandomizesPieceChoice()
{
	std::set<BoardPosition> opening_sources;
	for (std::uint32_t seed = 0; seed < 32; ++seed) {
		ChessSession session(seed);
		session.SetPlayers(
			ChessSession::PlayerKind::Computer,
			ChessSession::PlayerKind::Human);
		Check(session.AdvanceComputer(), "seeded Beginner computer moves");

		int moved_sources = 0;
		for (int row = 6; row < ChessBoard::Size; ++row) {
			for (int col = 0; col < ChessBoard::Size; ++col) {
				if (session.Board().GetPiece(row, col) != nullptr) {
					continue;
				}
				++moved_sources;
				opening_sources.emplace(row, col);

				const int destination_row = row == 6 ? 4 : 5;
				const int destination_col = row == 6 ? col : col == 1 ? 0 : 5;
				const Piece * destination =
					session.Board().GetPiece(destination_row, destination_col);
				Check(destination && destination->GetColor() == WHITE,
					"Beginner computer uses the chosen piece's first ordered move");
			}
		}
		Check(moved_sources == 1, "Beginner computer moves exactly one piece");
	}

	Check(opening_sources.size() > 1,
		"Beginner computer randomly selects among movable pieces");
}

void TestModeSwitchContinuousPromotionUndo()
{
	const auto setup_path = TempPath("computer-promotion-undo.xml");
	GameFacade setup;
	setup.SetPosition({
		{KING, WHITE, 7, 4},
		{KING, BLACK, 0, 4},
		{PAWN, WHITE, 1, 2},
	}, WHITE, {false, false, false, false});
	setup.SaveAs(setup_path.string());

	ChessSession session(0);
	session.Load(setup_path.string());
	session.SetPlayers(
		ChessSession::PlayerKind::Computer,
		ChessSession::PlayerKind::Computer);
	Check(session.AdvanceComputer(), "computer-vs-computer promotes the white pawn");
	Check(session.Board().GetPiece(0, 2)
			&& session.Board().GetPiece(0, 2)->GetType() == QUEEN,
		"computer-vs-computer promotion creates one white queen");
	Check(session.AdvanceComputer(), "black computer replies after promotion");

	session.SetPlayers(
		ChessSession::PlayerKind::Human,
		ChessSession::PlayerKind::Computer);
	Check(session.Undo(), "first undo after switching modes removes the black reply");
	Check(session.Undo(), "second undo after switching modes removes the promotion");
	Check(session.Board().GetPiece(1, 2)
			&& session.Board().GetPiece(1, 2)->GetType() == PAWN
			&& session.Board().GetPiece(1, 2)->GetColor() == WHITE,
		"continuous undo restores the original white pawn");
	Check(session.Board().GetPiece(0, 2) == nullptr,
		"continuous undo leaves no promoted queen on c8");

	int white_queens = 0;
	for (int row = 0; row < ChessBoard::Size; ++row) {
		for (int col = 0; col < ChessBoard::Size; ++col) {
			const Piece * piece = session.Board().GetPiece(row, col);
			if (piece && piece->GetColor() == WHITE
				&& piece->GetType() == QUEEN) {
				++white_queens;
			}
		}
	}
	Check(white_queens == 0,
		"mode-switch continuous undo does not duplicate white queens");
	std::filesystem::remove(setup_path);
}

} // namespace

int main()
{
	TestBoardLegend();
	TestSelectionTurnsUndoAndNew();
	TestPromotionWorkflow();
	TestSaveLoadPathSemantics();
	TestComputerModes();
	TestBeginnerComputerRandomizesPieceChoice();
	TestModeSwitchContinuousPromotionUndo();

	if (failures != 0) {
		std::cerr << failures << " session test assertion(s) failed\n";
		return 1;
	}
	std::cout << "All phase-four chess session tests passed\n";
	return 0;
}
