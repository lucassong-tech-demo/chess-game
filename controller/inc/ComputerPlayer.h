#ifndef COMPUTER_PLAYER_H
#define COMPUTER_PLAYER_H

#include <iostream>
#include <string>
#include <set>
#include <cstdlib>
#include "ChessPlayer.h"
#include "BoardPosition.h"
#include "GameFacade.h"
#include "Piece.h"


class ComputerPlayer : public ChessPlayer
{
private:
	GameFacade * game;
	std::set<BoardPosition> * move_choices;
	PieceColor color;
	Piece * myPiece;
	PieceType type;   // current moving piece type, so that the GUI can be updated.
					  // return the type to GameController
	int row;
	int col;

public:
		
	ComputerPlayer(PieceColor color, GameFacade & gf);
	~ComputerPlayer();

	void on_CellSelected(int row, int col, int button);

	void on_TimerEvent();

	int GetRow();

	int GetColumn();
	
	std::set<BoardPosition> & GetValidMoves();

	bool isMyPiece() const;
	
	bool isValidMove(int row, int col);

	/**
	 * the piece type should be return to GameController, so that
	 * The GUI can be updated correctly.
	 */
	PieceType Piece_Type()const;
	
	PieceColor Piece_Color()const;

	/**
	 *	check if the cell is taken by other piece. If it is taken, capture it
	 * 	Since GetValidMove(...) only return a set of available cells,
	 *  which includes the opponent piece, it doesn't have to check the 
	 *  type and color.
	 *  @param row the cell row coordinate.
	 *  @param col the cell column coordinate.
	 */	
	bool isTaken(int row, int col);

	void Attackit(int row, int col);

	void PlacePiece(int row_selected, int col_selected, int row, int col);


};

#endif
