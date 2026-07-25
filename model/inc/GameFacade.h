#ifndef GAME_FACADE_H
#define GAME_FACADE_H

#include <cassert>
#include <iostream>
#include <string>
#include <vector>
#include <set>
#include "ChessBoard.h"
#include "Piece.h"
#include "King.h"
#include "Queen.h"
#include "Rook.h"
#include "Bishop.h"
#include "Knight.h"
#include "Pawn.h"
#include "Xml.h"
#include "PieceHistory.h"
#include "BoardPosition.h"
#include "UnitTest.h"


using namespace std;

/**
 * GameFacade class reponse all ChessController function calls. 
 * It includes update the chess board, look for valid path, coordiate 
 * each player, record chess movement history, undo moves, save game, 
 * check if it is checkmate or stalemate.
 */

class GameFacade
{
private:
	ChessBoard * board;       // Chess board
	Piece * cur_Piece;        // The current moving piece	
	Piece * attack_Piece;     // if the cell is token
	PieceHistory * undo_once;  // undo a move, this pointer is for temp
	std::string file_name;    // for saving a game or loading a game
	std::set<BoardPosition> * valid_moves;   // it's on the stack, do not need to delete
	std::vector<PieceHistory *> move_history;	// it's on the heap, need to delete in destructor

public:

	// constructor,
	GameFacade();
	~GameFacade();

	/**
	 *	create new chess board, and initialize all data
	 */
	void NewGame();

	/**
	 * retrieve the piece object from chess board, it could be NULL.
	 * @param color looking for the piece with the indicated color 
	 * @return a piece object pointer, if current piece's color matching
	 * otherwise return NULL.
	 */
	Piece * GetPiece(int row, int col, PieceColor color);

	std::set<BoardPosition> & GetValidMoves();	

	bool isCellTaken(int row, int col);

	void Quit();

	void Clear_Board();

	/** check if the undo_once pointer points a privious undo object.
	 *  if true, then delete it, set it points to NULL.
	 */
	void ClearUndo();

	void ClearHistory();

	// Check current status is in check
	bool Check(int row, int col);

	// Check current status is in stuck
	bool Mate(int row, int col);

	void Moves_Filter(std::set<BoardPosition> * moves);

	void Look_for_Moves();

	std::set<BoardPosition> & Board_Check(int row, int col, int new_r, int new_c);
	
	void Build_Move_Set(std::set<BoardPosition> * moves, std::set<BoardPosition> * allMoves);
	/**
	 * 	Move the piece to the cell located in (row, column). Update the 
	 *  Each move will be recoded into a PieceLocation class and insert it 
	 *  into vector.
	 *  @param row this is the position of row on the chess board.
 	 *  	   column this is the position of column on the chess board.
	 */
	void MovePiece(int r_selected, int col_selected, int row, int column);

	/**
	 *  Check if current cell selection is in the scope of 
	 *  move_choices set<>
	 *  @param row the row of cell on chess board;
	 *  @param col the column of cell on chess board;
	 */
	bool isValidMove(int row, int col);

	/**
	 * turn back one step(move) after each Undo function.
	 * board_history stack will pop once
	 * @return a pointer to the poped history
	 * 		   it will be deleted by GameController, after update GUI
	 */
	PieceHistory * Undo();
 
	/**
	 * Create an Xml object. Passing the board_history stack and chess board point. Open the existing .xml file. 
	 * Write the History and current board position into the file.
	 * If file_name is empty, then invoke SaveAs() function.
	 */
	void SaveGame(const std::string & fileName);

	/**
	 * create a Xml object, Open the file and write chess History data and current board positions into the file
	 */
	void SaveAs(const std::string & filename);

	/**
	 * load the exist game by open the xml file read data from it. Reload the chess board, and 
	 * refresh the board_history stack with the loaded data
	 *
	 * @param filename the file will be open and data will be reloaded.
	 * @return true/false, true when succeed reload data, but false will show a warning dialog
	 */
	bool LoadGame(const std::string & filename);

	/**
	 * Add moves into  board_history link
	 * @param moves, a vector container contains PieceHistory object which includes
	 * 		  the piece type, color and position.
	 */
	void UpdateMoveHistory(std::vector<PieceHistory> & moves);

	/**
	 * board_history stack will pop a vector container. restore the chess board
	 */
	void ReadMoveHistory();

	/**
	 * for the new game, pop all elements of history stack
	 */
	void Clean_History();
	
	/**
	 * for the new game, reset the chess board
	 */
	void Reset_Chess_Board();	
	
	static bool Test(std::ostream & os);
};


#endif
