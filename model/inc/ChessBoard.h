#ifndef CHESS_BOARD_H
#define CHESS_BOARD_H

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <set>
#include <cassert>
#include "Piece.h"
#include "King.h"
#include "Queen.h"
#include "Rook.h"
#include "Bishop.h"
#include "Knight.h"
#include "Pawn.h"
#include "BoardPosition.h"
#include "UnitTest.h"

using namespace std;

/**
 * ChessBoard class define the chess board and all pieces
 */
class ChessBoard
{

private:
	Piece * board[8][8];
	Piece * w_King;
	Piece * b_King;
	std::set<BoardPosition> * allMoves;

public:
	ChessBoard();
	~ChessBoard();

	void ClearBoard();
	void ClearPieceStack();

	void Init_Board();
	void Init_White_Piece();
	void Init_Black_Piece();
	/**
	 *	put a piece on a cell, it is good for undo or move a piece
	 */
	void Reset_Piece (const int s_row, const int s_col, const int end_r, const int end_col);
	

	/**
	 * Reset the chess board, for a new game
	 */
	void Reset();

	/**
	 * set each cell on the board to zero, put all pieces into either 
	 * off_white_pieces vector, or off_black_pieces vector.
	 */
	void Clear_board();

	Piece * Get_A_Piece(const int row, const int col);
	
	/**
	 * After undo, if a piece was attacked, then put it back where it is from.
	 *  @param row and col a piece original location, 
	 *  @param piece the pointer of the piece attacked.
	 */
	void PutBack_Piece(Piece* piece, const int row, const int col);

	Piece * Get_W_King();
	
	Piece * Get_B_King();

	Piece * GetKing(PieceColor color);
	
	
	/**
	 * return the selected piece to Game Facade for set up the GUI
	 */
	Piece* GetPiece(int row, int col) const;

	void MovePiece(int r_selected, int c_selected, int row, int col);


	bool Piece_Stack_Empty();

	void Undo();


	static bool Test(std::ostream & os);

};

#endif
