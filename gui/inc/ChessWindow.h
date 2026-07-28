#ifndef CHESS_WINDOW_H
#define CHESS_WINDOW_H

#include <gtkmm.h>

#include "ChessBoardView.h"
#include "ChessPresentation.h"
#include "ChessSession.h"

class ChessWindow : public Gtk::ApplicationWindow
{
public:
	explicit ChessWindow(const Glib::RefPtr<Gtk::Application> & application);

private:
	ChessSession session_;
	Gtk::Box layout_{Gtk::Orientation::VERTICAL};
	Gtk::HeaderBar header_bar_;
	Gtk::Label title_label_{"Chess"};
	Gtk::MenuButton menu_button_;
	ChessBoardView board_view_;
	Gtk::Label status_label_;
	Gtk::Label file_label_;
	Gtk::Label help_label_{ChessPresentation::BoardLegend};
	Gtk::AboutDialog about_dialog_;
	Glib::RefPtr<Gtk::FileDialog> save_dialog_;
	Glib::RefPtr<Gtk::FileDialog> load_dialog_;
	Gtk::Dialog promotion_dialog_;
	sigc::connection computer_timer_;

	void ConfigureActions(const Glib::RefPtr<Gtk::Application> & application);
	void ConfigureMenu();
	void Refresh();
	void OnCellSelected(int row, int col);
	void OnNewGame();
	void OnUndo();
	void OnSave();
	void OnSaveAs();
	void OnSaveFinished(const Glib::RefPtr<Gio::AsyncResult> & result);
	void OnLoad();
	void OnLoadFinished(const Glib::RefPtr<Gio::AsyncResult> & result);
	void OnPromotionResponse(int response);
	void SetPlayerMode(
		ChessSession::PlayerKind white,
		ChessSession::PlayerKind black);
	void ScheduleComputer();
	bool OnComputerTimer();
	void OnAbout();
	void ShowStatus(const Glib::ustring & message, bool error = false);
};

#endif
