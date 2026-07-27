#include <gtkmm.h>

#include "ChessWindow.h"

int main(int argc, char * argv[])
{
	auto application = Gtk::Application::create("io.github.chess_game");
	return application->make_window_and_run<ChessWindow>(
		argc, argv, application);
}
