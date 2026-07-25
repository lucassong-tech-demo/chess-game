#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H

#include <iostream>
#include <string>
#include <set>
#include <cassert>
#include "ChessView.h"
#include "ChessGuiDefines.h"
#include "ChessController.h"
#include "HumanPlayer.h"
#include "ChessPlayer.h"
#include "ComputerPlayer.h"
#include "PieceHistory.h"
#include "BoardPosition.h"
#include "GameFacade.h"
#include "Piece.h"
#include "HTTPInputStream.h"
#include "HTMLTokenizer.h"
#include "StringUtil.h"
#include "CS240Exception.h"

/*
 * GameController class implements the ChessController interface. It works with GUI 
 * on the UI layer. Its main jobs are to pass any actions from GUI 
 * (a click on chess board, click the newGame button....) to GameFacade 
 * and update the viewer data.
 *
 */

class GameController : public ChessController
{
private:
	ChessView * gui_View;    // update the GUI
	GameFacade game;
	Piece * piece;           // the Piece selected by player
	ChessPlayer * player_one;
	ChessPlayer * player_two;
	std::set< BoardPosition > * move_choices;

	int row_selected;		// the moving piece position
	int col_selected;
	bool piece_selected;	// tack if the piece is selected
	bool game_start;
	bool w_turn;                // True is White turn 
							  // False is black turn
	int game_mode;            //
	bool is_On;
	std::string fileName;

public:

	/*
	 * This constructor will setup the game mode base on option value. And
	 * it starts the game.
	 * @param option it has four possible values, 1: game is in Human v.s. Human
	 *        mode, 2: Human v.s. PC mode, 3: PC v.s. PC mode, 4: PC v.s. Human
	 * 		  mode
	 */
	GameController(int option);

	~GameController();

	void HighLight_Cell();

	void Print_Message();

	void Print_Status();

	void Setup_Mode(int option);

	void ResetPlayer();

	void Clear_Board();
	
	void SwitchPlayer();

	void Redo_Game(HTMLTokenizer & token);

	void XmlReader(HTMLTokenizer & token);

	void Movements(HTMLTokenizer & token);

	void AutoPlay(int row, int col);
	/**
	 * Indicate to the player that the user clicked on the given
	 * row and column with the mouse.
	 */
	void on_CellSelected(int row, int col, int button);

	void HumanPlayer_Move(int row, int col, int button, ChessPlayer* player);

	void ComputerPlayer_Move(ChessPlayer* player);

	void PlayerTwo_Move(int row, int col, int button);

	/**
	 * if movement set is not NULL, then undo highlight all cells in
	 * the movement set. If movement set is NULL, then get the new set.
	 * High light the selected piece and available cells.
	 */
	void Light_On(int row, int col, HumanPlayer* player);
	
	void Highlight_Cells(int row, int col);

	/**
	 *  remove the highlight cells
	 *	@param r_selected, the row of the cell where selected piece,
	 *  @param c_selected, the column of the cell where selected piece.
	 */
	void Undo_Highlight();

	/** move the piece from one cell to another
	 * 	@param row, col the row and column of the cell which the piece is moving to
	 *  @param color the color of current player's piece
	 */
	void Update_GUI(int row, int col, PieceType type, PieceColor color);
	/**
	 *  move piece or attack opponent
	 *  @param row the row of the cell moving to
	 *  @param col the column of the cell moving to
	 *  @param color the color of current player's piece
	 */
	void Move_Or_Attack(int row, int col, ChessPlayer* player);
		
	ImageName White_Piece(PieceType type);

	ImageName Black_Piece(PieceType type);

	///@param row where drag began
	///@param col where drag began
	void on_DragStart(int row,int col);

	///@param row where drag ended
	///@param col where drag ended
	///@return true if the drag resulted in a successful drop
	bool on_DragEnd(int row,int col);

	/**
	 * Handle when the user selected the new game button.
	 */
	void on_NewGame();

	void place_BlackPiece();

	void place_WhitePiece();

	/**
	 * Handle when the user selected the save game button.
	 */
	void on_SaveGame();

	/**
	 * Handle when the user selected the save game as button.
	 */
	void on_SaveGameAs();

	/**
	 * Handle when the user selected the load game button.
	 */
	void on_LoadGame();

	/**
	 * Handle when the user selected the undo move button.
	 */
	void on_UndoMove();
		
	/**
	 * Handle when the user selects to quit the game, either through the
	 * quit button, the close X button, or the file menu.
	 */
	void on_QuitGame();
		
	/**
	 * Handle when a timer event has been signaled.
	 */
	void on_TimerEvent();
	
	/**
	 * Set the ChessView that this ChessController will handle inputs for.
	 */
	void SetView(ChessView* view);
};

#endif

