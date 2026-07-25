// Author: Owen Merkling,,, <omerkling@gmail.com>, (C) 2008
// Contributor:  Students Name
//
// Copyright: For use by the students of CS 240 at BYU
//
//

#include <set>
#include <iostream>
using namespace std;

#include "SelectDialog.h"
#include "Chess.h"

#include "ChessController.h"


Chess::Chess(std::string gladefile):gui(0),logId(0)
{
	InitializeChessGui(gladefile);
	controller = 0;
	
	timeoutMilliseconds = 500;
}

Chess::~Chess()
{
	CleanupChessGui();
}



/********Implement These*****************************/


void Chess::on_CellSelected(int row, int col, int button)
{
	g_debug("Chess::on_CellSelected (%d,%d)",row,col);
	/*
	  Each square of the chess board is reffered to in the GUI code as a cell.
	This Function is called whenever the uses clicks and releases the mous button over
	a cell without initiating a drag. Row and Column coordinates begin in the top left corner.
	The button paramter tells which mouse button was clicked
	(1 for left, 2 for middle, 3 for right).
	You do not need to worry about wich button was used to complete the project.
	*/
	
	if(controller != 0)
	{
		controller->on_CellSelected(row, col, button);
	}
}

void Chess::on_DragStart(int row,int col)
{
	g_debug("Chess::on_DragStart (%d,%d)",row,col);
	/*
	  When a drag is initiated, this function will be called instead of on_CellSelected().
	The paramaters row and col are the coordinates of the cell where the drag was initiated.
	All three buttons may initiate the drag, but for our purposes can be treated
	the same and so	that paramater is not included.
	*/
	
	if(controller != 0)
	{
		controller->on_DragStart(row, col);
	}
}
bool Chess::on_DragEnd(int row,int col)
{
	g_debug("Chess::on_DragEnd (%d,%d)",row,col);
	/*
	  Same as on_DragStart() except the coordinates represent the ending cell of
	the drag. If the drag terminates off the playing board, this will be called with
	the initial coordinates of the drag.
	*/

	if(controller != 0)
	{
		return controller->on_DragEnd(row, col);
	}
	else
	{
		//by convention, this should return a boolean value indicating if the drag was accepted or not.
		return false;		
	}
}


void Chess::on_NewGame()
{
	g_debug("Chess::on_NewGame");
	/*
	Called when someone selects 'New' from the toolbar, 'Game' menu, or presses 'Ctrl-N'.
	*/
	
	if(controller != 0)
	{
		controller->on_NewGame();
	}
}

void Chess::on_SaveGame()
{
	g_debug("Chess::on_SaveGame");
	/*
	Called when someone selects 'Save' from the toolbar, 'Game' menu, or presses 'Ctrl-S'.
	*/
	
	if(controller != 0)
	{
		controller->on_SaveGame();
	}
}

void Chess::on_SaveGameAs()
{
	g_debug("Chess::on_SaveGameAs");
	/*
	Called when someone selects 'Save As' from the 'Game' menu, or presses 'Shift-Ctrl-S'.
	*/
	
	if(controller != 0)
	{
		controller->on_SaveGameAs();
	}
}
void Chess::on_LoadGame()
{
	g_debug("Chess::on_LoadGame");
	/*
	Called when someone selects 'Open' from the toolbar, 'Game' menu, or presses 'Ctrl-O'.
	*/
	
	if(controller != 0)
	{
		controller->on_LoadGame();
	}
}

void Chess::on_UndoMove()
{
	g_debug("Chess::on_UndoMove");
	/*
	Called when someone selects 'Undo' from the toolbar, 'Game' menu, or presses 'Ctrl-Z'.
	*/
	
	if(controller != 0)
	{
		controller->on_UndoMove();
	}
}


void Chess::on_QuitGame()
{
	g_debug("Chess::on_QuitGame");
	/*It is not required to implement this function*/

	/*
		Called when someone selects 'Quit' from the toolbar,
		'Game' menu, presses 'Ctrl-Q', or closes the window.<br>

	on_QuitGame() does not need to be implemented to fulfill the
	requirements of the project, but is available for your
	use.
	*/
	
	if(controller != 0)
	{
		controller->on_QuitGame();
	}
}

bool Chess::on_TimerEvent()
{
	//g_debug("Chess::on_TenthSecond");
	
	if(controller != 0)
	{
		controller->on_TimerEvent();
	}
	
	// Do not change this return statement to false.  It must return true, or else this timer
	// function will only happen once.
	return true;
}


/**********You Do Not Need To Touch These Functions***********************************/
void Chess::SetTopLabel(const std::string & text)
{
	gui->SetTopLabel(text);
}

void Chess::SetBottomLabel(const std::string & text)
{
	gui->SetBottomLabel(text);
}

void Chess::ClearMessageArea()
{
	gui->ClearMessageArea();
}

void Chess::WriteMessageArea(const std::string & message)
{
	gui->WriteMessageArea(message);
}

void Chess::SetStatusBar(const std::string & status)
{
	gui->SetStatusBar(status);
}

std::string Chess::SelectSaveFile()
{
	return gui->SelectSaveFile();
}
std::string Chess::SelectLoadFile()
{
	return gui->SelectLoadFile();
}


void Chess::HighlightSquare(signed int row,signed int col,guint32  color)
{
	gui->HighlightSquare(row, col, color);
}

void Chess::UnHighlightSquare(signed int row,signed int col)
{
	gui->UnHighlightSquare(row, col);
}

void Chess::PlacePiece(signed int row,signed int col, ImageName piece)
{
	gui->PlacePiece(row, col, piece);
}

void Chess::ClearPiece(signed int row,signed int col)
{
	gui->ClearPiece(row, col);
}

void Chess::SetController(ChessController* controller)
{
	this->controller = controller;
}

void Chess::SetTimeoutMilliseconds(int milliseconds)
{
	timeoutMilliseconds = milliseconds;
}

void Chess::run(Gtk::Main & app)
{
	InitializeTimer();
	on_NewGame();
	app.run(*gui);
}

void Chess::InitializeChessGui(const std::string& gladefile)
{
	Glib::RefPtr<Gnome::Glade::Xml> chessXml = Gnome::Glade::Xml::create(gladefile);
	chessXml->get_widget_derived("Chess_Main",gui);
	
	gui->signal_cell_selected().connect(sigc::mem_fun(*this,&Chess::on_CellSelected));

	gui->signal_new().connect(sigc::mem_fun(*this,&Chess::on_NewGame));
	gui->signal_save().connect(sigc::mem_fun(*this,&Chess::on_SaveGame));
	gui->signal_save_as().connect(sigc::mem_fun(*this,&Chess::on_SaveGameAs));
	gui->signal_load().connect(sigc::mem_fun(*this,&Chess::on_LoadGame));
	gui->signal_undo().connect(sigc::mem_fun(*this,&Chess::on_UndoMove));
	gui->signal_quit().connect(sigc::mem_fun(*this,&Chess::on_QuitGame));


	gui->signal_drag_start().connect(sigc::mem_fun(*this,&Chess::on_DragStart));
	gui->signal_drag_end().connect(sigc::mem_fun(*this,&Chess::on_DragEnd));

	//Set g_log to print to the message area
	logId = g_log_set_handler(0,
							  GLogLevelFlags(G_LOG_LEVEL_MASK|
											 G_LOG_FLAG_FATAL|
											 G_LOG_FLAG_RECURSION),
							  log_handler,gui);	
}

void Chess::CleanupChessGui()
{
	g_log_remove_handler(0,logId);//remove reference to data
	delete gui;
	gui=0;
}


std::string format_log(GLogLevelFlags log_level,std::string & message)
{
	//if a level is set in the LOG_LEVEL_HIDE_MASK it will fall through to the
	//default case and so will not be printed
	int level = log_level & (~LOG_LEVEL_HIDE_MASK & G_LOG_LEVEL_MASK);
	std::string output;
	switch(level)
	{
		case G_LOG_LEVEL_DEBUG:
				output = std::string("DEBUG::")+message;
			break;
		case G_LOG_LEVEL_ERROR:
			output = std::string("ERROR::")+message;
			break;
		case G_LOG_LEVEL_CRITICAL:
			output = std::string("CRITICAL::")+message;
			break;
		case G_LOG_LEVEL_WARNING:
			output = std::string("WARNING::")+message;
			break;
		case G_LOG_LEVEL_MESSAGE:
			output = std::string("MESSAGE::")+message;
			break;
		case G_LOG_LEVEL_INFO:
			output = std::string("INFO::")+message;
			break;
		default:
			output="";
	}
	return output;
}

void Chess::InitializeTimer()
{
	Glib::signal_timeout().connect( sigc::mem_fun(*this, &Chess::on_TimerEvent), timeoutMilliseconds );
}

void log_handler(const gchar *log_domain,
				 GLogLevelFlags log_level,
				 const gchar *message, gpointer user_data)
{
    ///@todo Provide more info
	std::string output= std::string(message?message:"");
	bool fatal=false;

	if(log_level & G_LOG_FLAG_FATAL) fatal=true;

	output = format_log(log_level,output);

	if(fatal)
	{
		std::cerr<<"FATAL ERROR::TERMINATING::"<<output<<std::endl;
		exit(1);
	}
	else if(output != "")
	{
		if(user_data)
		{
			//user data must be ChessGui!!!
			static_cast<ChessGui *>(user_data)->WriteMessageArea(output+"\n");
		}
		else
		{
			std::cerr<<output<<std::endl;
		}
	}
	else //do nothing

	return;
}
/**************************************************************************/
