#include "ChessBoardView.h"

#include <stdexcept>

namespace {

bool IsPosition(
	const std::optional<BoardPosition> & position,
	int row,
	int col)
{
	return position
		&& position->GetRow() == row
		&& position->GetColumn() == col;
}

bool HasMove(const std::set<BoardPosition> & moves, int row, int col)
{
	return moves.find(BoardPosition(row, col)) != moves.end();
}

} // namespace

ChessBoardView::ChessBoardView()
	: Gtk::AspectFrame(Gtk::Align::CENTER, Gtk::Align::CENTER, 1.0F, false)
{
	add_css_class("board-frame");
	set_hexpand(true);
	set_vexpand(true);
	grid_.set_row_homogeneous(true);
	grid_.set_column_homogeneous(true);
	grid_.add_css_class("chess-board");
	set_child(grid_);

	for (int row = 0; row < ChessBoard::Size; ++row) {
		for (int col = 0; col < ChessBoard::Size; ++col) {
			auto * cell = Gtk::make_managed<Gtk::Button>();
			auto * picture = Gtk::make_managed<Gtk::Picture>();
			cell->add_css_class("board-cell");
			cell->add_css_class((row + col) % 2 == 0 ? "light" : "dark");
			cell->set_hexpand(true);
			cell->set_vexpand(true);
			cell->set_focus_on_click(false);
			cell->set_tooltip_text(
				std::string(1, static_cast<char>('a' + col))
				+ std::to_string(ChessBoard::Size - row));

			picture->set_can_shrink(true);
			picture->set_content_fit(Gtk::ContentFit::CONTAIN);
			picture->set_hexpand(true);
			picture->set_vexpand(true);
			cell->set_child(*picture);

			auto click = Gtk::GestureClick::create();
			click->set_button(1);
			click->signal_released().connect(
				[this, row, col](int, double, double) {
					cell_selected_.emit(row, col);
				});
			cell->add_controller(click);

			grid_.attach(*cell, col, row);
			cells_[Index(row, col)] = cell;
			pictures_[Index(row, col)] = picture;
		}
	}
}

void ChessBoardView::Refresh(const ChessSession & session)
{
	for (int row = 0; row < ChessBoard::Size; ++row) {
		for (int col = 0; col < ChessBoard::Size; ++col) {
			Gtk::Button & cell = *cells_[Index(row, col)];
			Gtk::Picture & picture = *pictures_[Index(row, col)];
			cell.remove_css_class("selected");
			cell.remove_css_class("legal");
			cell.remove_css_class("capture");

			if (IsPosition(session.Selected(), row, col)) {
				cell.add_css_class("selected");
			} else if (HasMove(session.LegalMoves(), row, col)) {
				cell.add_css_class(
					session.IsCaptureTarget(row, col) ? "capture" : "legal");
			}

			const Piece * piece = session.Board().GetPiece(row, col);
			if (piece) {
				picture.set_resource(PieceResource(*piece));
				picture.set_visible(true);
			} else {
				picture.set_visible(false);
			}
		}
	}
}

sigc::signal<void(int, int)> & ChessBoardView::signal_cell_selected()
{
	return cell_selected_;
}

int ChessBoardView::Index(int row, int col) noexcept
{
	return row * ChessBoard::Size + col;
}

std::string ChessBoardView::PieceResource(const Piece & piece)
{
	const std::string color = piece.GetColor() == WHITE ? "w" : "b";
	switch (piece.GetType()) {
	case KING: return "/io/github/chess_game/pieces/" + color + "king.png";
	case QUEEN: return "/io/github/chess_game/pieces/" + color + "queen.png";
	case KNIGHT: return "/io/github/chess_game/pieces/" + color + "knight.png";
	case BISHOP: return "/io/github/chess_game/pieces/" + color + "bishop.png";
	case ROOK: return "/io/github/chess_game/pieces/" + color + "rook.png";
	case PAWN: return "/io/github/chess_game/pieces/" + color + "pawn.png";
	}
	throw std::logic_error("unknown piece type");
}
