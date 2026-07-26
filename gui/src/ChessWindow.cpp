#include "ChessWindow.h"

#include <exception>

namespace {

Glib::RefPtr<Gio::SimpleAction> AddAction(
	Gtk::ApplicationWindow & window,
	const Glib::ustring & name,
	const sigc::slot<void()> & callback)
{
	auto action = Gio::SimpleAction::create(name);
	action->signal_activate().connect(
		[callback](const Glib::VariantBase &) { callback(); });
	window.add_action(action);
	return action;
}

} // namespace

ChessWindow::ChessWindow(
	const Glib::RefPtr<Gtk::Application> & application)
	: Gtk::ApplicationWindow(application)
{
	set_title("Chess");
	set_default_size(720, 780);
	set_size_request(430, 510);

	title_label_.add_css_class("title");
	header_bar_.set_title_widget(title_label_);
	menu_button_.set_icon_name("open-menu-symbolic");
	menu_button_.set_tooltip_text("Main menu");
	header_bar_.pack_end(menu_button_);
	set_titlebar(header_bar_);

	layout_.set_spacing(12);
	layout_.set_margin(16);
	layout_.append(board_view_);

	status_label_.set_xalign(0.0F);
	status_label_.set_wrap(true);
	status_label_.add_css_class("status");
	layout_.append(status_label_);

	help_label_.set_xalign(0.0F);
	help_label_.set_wrap(true);
	help_label_.add_css_class("help");
	layout_.append(help_label_);
	set_child(layout_);

	board_view_.signal_cell_selected().connect(
		sigc::mem_fun(*this, &ChessWindow::OnCellSelected));

	ConfigureActions(application);
	ConfigureMenu();

	about_dialog_.set_transient_for(*this);
	about_dialog_.set_modal(true);
	about_dialog_.set_program_name("Chess");
	about_dialog_.set_version("0.1.0");
	about_dialog_.set_comments(
		"A GTKmm 4 interface backed by the portable chess core.");

	save_dialog_ = Gtk::FileDialog::create();
	save_dialog_->set_title("Save Game");
	save_dialog_->set_initial_name("chess-game.xml");

	auto css = Gtk::CssProvider::create();
	css->load_from_resource("/io/github/chess_game/chess.css");
	Gtk::StyleContext::add_provider_for_display(
		get_display(), css, GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);

	Refresh();
}

void ChessWindow::ConfigureActions(
	const Glib::RefPtr<Gtk::Application> & application)
{
	AddAction(*this, "new", sigc::mem_fun(*this, &ChessWindow::OnNewGame));
	AddAction(*this, "undo", sigc::mem_fun(*this, &ChessWindow::OnUndo));
	AddAction(*this, "save", sigc::mem_fun(*this, &ChessWindow::OnSave));
	AddAction(*this, "load", sigc::mem_fun(*this, &ChessWindow::OnLoad));
	AddAction(*this, "about", sigc::mem_fun(*this, &ChessWindow::OnAbout));
	AddAction(*this, "quit", sigc::mem_fun(*this, &Gtk::Window::close));

	application->set_accel_for_action("win.new", "<Primary>n");
	application->set_accel_for_action("win.undo", "<Primary>z");
	application->set_accel_for_action("win.save", "<Primary>s");
	application->set_accel_for_action("win.load", "<Primary>o");
	application->set_accel_for_action("win.quit", "<Primary>q");
}

void ChessWindow::ConfigureMenu()
{
	auto menu = Gio::Menu::create();
	menu->append("_New Game", "win.new");
	menu->append("_Undo", "win.undo");
	menu->append("_Save…", "win.save");
	menu->append("_Load…", "win.load");
	menu->append("_About Chess", "win.about");
	menu->append("_Quit", "win.quit");
	menu_button_.set_menu_model(menu);
}

void ChessWindow::Refresh()
{
	board_view_.Refresh(session_);
	ShowStatus(session_.Status());
}

void ChessWindow::OnCellSelected(int row, int col)
{
	try {
		session_.SelectCell(row, col);
		Refresh();
	} catch (const std::exception & error) {
		ShowStatus(error.what(), true);
	}
}

void ChessWindow::OnNewGame()
{
	session_.NewGame();
	Refresh();
}

void ChessWindow::OnUndo()
{
	session_.Undo();
	Refresh();
}

void ChessWindow::OnSave()
{
	save_dialog_->save(
		*this,
		sigc::mem_fun(*this, &ChessWindow::OnSaveFinished));
}

void ChessWindow::OnSaveFinished(
	const Glib::RefPtr<Gio::AsyncResult> & result)
{
	try {
		const auto file = save_dialog_->save_finish(result);
		session_.Save(file->get_path());
		Refresh();
	} catch (const Gtk::DialogError & error) {
		if (error.code() != Gtk::DialogError::DISMISSED) {
			ShowStatus(error.what(), true);
		}
	} catch (const Glib::Error & error) {
		ShowStatus(error.what(), true);
	} catch (const std::exception & error) {
		ShowStatus(error.what(), true);
	}
}

void ChessWindow::OnLoad()
{
	ShowStatus(
		"Load is unavailable: the current chess core does not implement "
		"GameFacade::LoadGame().",
		true);
}

void ChessWindow::OnAbout()
{
	about_dialog_.present();
}

void ChessWindow::ShowStatus(const Glib::ustring & message, bool error)
{
	status_label_.set_text(message);
	status_label_.remove_css_class("error");
	if (error) {
		status_label_.add_css_class("error");
	}
}
