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

	file_label_.set_xalign(0.0F);
	file_label_.set_ellipsize(Pango::EllipsizeMode::MIDDLE);
	file_label_.add_css_class("help");
	layout_.append(file_label_);

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
	load_dialog_ = Gtk::FileDialog::create();
	load_dialog_->set_title("Load Game");

	promotion_dialog_.set_transient_for(*this);
	promotion_dialog_.set_modal(true);
	promotion_dialog_.set_title("Promote Pawn");
	promotion_dialog_.add_button("Queen", static_cast<int>(QUEEN));
	promotion_dialog_.add_button("Rook", static_cast<int>(ROOK));
	promotion_dialog_.add_button("Bishop", static_cast<int>(BISHOP));
	promotion_dialog_.add_button("Knight", static_cast<int>(KNIGHT));
	promotion_dialog_.signal_response().connect(
		sigc::mem_fun(*this, &ChessWindow::OnPromotionResponse));

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
	AddAction(*this, "save-as", sigc::mem_fun(*this, &ChessWindow::OnSaveAs));
	AddAction(*this, "load", sigc::mem_fun(*this, &ChessWindow::OnLoad));
	AddAction(
		*this,
		"human-human",
		[this] {
			SetPlayerMode(
				ChessSession::PlayerKind::Human,
				ChessSession::PlayerKind::Human);
		});
	AddAction(
		*this,
		"human-computer",
		[this] {
			SetPlayerMode(
				ChessSession::PlayerKind::Human,
				ChessSession::PlayerKind::Computer);
		});
	AddAction(
		*this,
		"computer-human",
		[this] {
			SetPlayerMode(
				ChessSession::PlayerKind::Computer,
				ChessSession::PlayerKind::Human);
		});
	AddAction(
		*this,
		"computer-computer",
		[this] {
			SetPlayerMode(
				ChessSession::PlayerKind::Computer,
				ChessSession::PlayerKind::Computer);
		});
	AddAction(*this, "about", sigc::mem_fun(*this, &ChessWindow::OnAbout));
	AddAction(*this, "quit", sigc::mem_fun(*this, &Gtk::Window::close));

	application->set_accel_for_action("win.new", "<Primary>n");
	application->set_accel_for_action("win.undo", "<Primary>z");
	application->set_accel_for_action("win.save", "<Primary>s");
	application->set_accel_for_action("win.save-as", "<Primary><Shift>s");
	application->set_accel_for_action("win.load", "<Primary>o");
	application->set_accel_for_action("win.quit", "<Primary>q");
}

void ChessWindow::ConfigureMenu()
{
	auto menu = Gio::Menu::create();
	menu->append("_New Game", "win.new");
	menu->append("_Undo", "win.undo");
	menu->append("_Save…", "win.save");
	menu->append("Save _As…", "win.save-as");
	menu->append("_Load…", "win.load");
	auto players = Gio::Menu::create();
	players->append("Human vs Human", "win.human-human");
	players->append("Human vs Computer", "win.human-computer");
	players->append("Computer vs Human", "win.computer-human");
	players->append("Computer vs Computer", "win.computer-computer");
	menu->append_submenu("_Players", players);
	menu->append("_About Chess", "win.about");
	menu->append("_Quit", "win.quit");
	menu_button_.set_menu_model(menu);
}

void ChessWindow::Refresh()
{
	board_view_.Refresh(session_);
	ShowStatus(session_.Status());
	file_label_.set_text(
		session_.CurrentFile().empty()
			? "Current file: (unsaved)"
			: "Current file: " + session_.CurrentFile());
}

void ChessWindow::OnCellSelected(int row, int col)
{
	try {
		const auto interaction = session_.SelectCell(row, col);
		Refresh();
		if (interaction == ChessSession::Interaction::PromotionRequired) {
			promotion_dialog_.present();
		} else if (interaction == ChessSession::Interaction::MoveCompleted) {
			ScheduleComputer();
		}
	} catch (const std::exception & error) {
		ShowStatus(error.what(), true);
	}
}

void ChessWindow::OnNewGame()
{
	session_.NewGame();
	Refresh();
	ScheduleComputer();
}

void ChessWindow::OnUndo()
{
	if (computer_timer_.connected()) {
		computer_timer_.disconnect();
	}
	session_.Undo();
	Refresh();
}

void ChessWindow::OnSave()
{
	if (!session_.CurrentFile().empty()) {
		try {
			session_.Save();
			Refresh();
		} catch (const std::exception & error) {
			ShowStatus(error.what(), true);
		}
		return;
	}
	OnSaveAs();
}

void ChessWindow::OnSaveAs()
{
	session_.ClearInteraction();
	Refresh();
	save_dialog_->save(
		*this,
		sigc::mem_fun(*this, &ChessWindow::OnSaveFinished));
}

void ChessWindow::OnSaveFinished(
	const Glib::RefPtr<Gio::AsyncResult> & result)
{
	try {
		const auto file = save_dialog_->save_finish(result);
		if (!file || file->get_path().empty()) {
			throw std::runtime_error("the selected save location has no local path");
		}
		session_.SaveAs(file->get_path());
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
	session_.ClearInteraction();
	Refresh();
	load_dialog_->open(
		*this,
		sigc::mem_fun(*this, &ChessWindow::OnLoadFinished));
}

void ChessWindow::OnLoadFinished(
	const Glib::RefPtr<Gio::AsyncResult> & result)
{
	try {
		const auto file = load_dialog_->open_finish(result);
		if (!file || file->get_path().empty()) {
			throw std::runtime_error("the selected file has no local path");
		}
		session_.Load(file->get_path());
		Refresh();
		ScheduleComputer();
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

void ChessWindow::OnPromotionResponse(int response)
{
	promotion_dialog_.hide();
	try {
		session_.Promote(static_cast<PieceType>(response));
		Refresh();
		ScheduleComputer();
	} catch (const std::exception & error) {
		ShowStatus(error.what(), true);
	}
}

void ChessWindow::SetPlayerMode(
	ChessSession::PlayerKind white,
	ChessSession::PlayerKind black)
{
	session_.SetPlayers(white, black);
	Refresh();
	ScheduleComputer();
}

void ChessWindow::ScheduleComputer()
{
	if (computer_timer_.connected()) {
		computer_timer_.disconnect();
	}
	if (session_.PlayerFor(session_.Turn()) == ChessSession::PlayerKind::Computer) {
		computer_timer_ = Glib::signal_timeout().connect(
			sigc::mem_fun(*this, &ChessWindow::OnComputerTimer), 350);
	}
}

bool ChessWindow::OnComputerTimer()
{
	try {
		session_.AdvanceComputer();
		Refresh();
	} catch (const std::exception & error) {
		ShowStatus(error.what(), true);
		return false;
	}
	return session_.PlayerFor(session_.Turn()) == ChessSession::PlayerKind::Computer
		&& session_.GameState() != GameStatus::Checkmate
		&& session_.GameState() != GameStatus::Stalemate
		&& session_.GameState() != GameStatus::DrawThreefold
		&& session_.GameState() != GameStatus::DrawFiftyMove
		&& session_.GameState() != GameStatus::DrawInsufficientMaterial;
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
