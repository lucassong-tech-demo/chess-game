#ifndef CHESS_BOARD_VIEW_H
#define CHESS_BOARD_VIEW_H

#include <array>

#include <gtkmm.h>

#include "ChessSession.h"

class ChessBoardView : public Gtk::AspectFrame
{
public:
	ChessBoardView();

	void Refresh(const ChessSession & session);
	sigc::signal<void(int, int)> & signal_cell_selected();

private:
	static constexpr int CellCount = ChessBoard::Size * ChessBoard::Size;

	Gtk::Grid grid_;
	std::array<Gtk::Box *, CellCount> cells_{};
	std::array<Gtk::Picture *, CellCount> pictures_{};
	sigc::signal<void(int, int)> cell_selected_;

	static int Index(int row, int col) noexcept;
	static std::string PieceResource(const Piece & piece);
};

#endif
