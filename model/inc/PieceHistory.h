#ifndef PIECE_HISTORY_H
#define PIECE_HISTORY_H

#include <string>
#include <iostream>
#include "Piece.h"

/**
 *  PieceLocation is used for movement history link.
 *  it records each moving piece's information
 */

class PieceHistory
{
private:
	Piece * moving_piece;
	int start_row;
	int start_column;
	int end_row;
	int end_column;
	Piece * attacked_piece;

public:
	// attacked_type and color is initialized by null
	//PieceHistory(PieceType type, PieceColor color, int s_row, int s_col, int end_r, int end_c);
	PieceHistory(Piece * m_Piece, int s_row, int s_col, int end_r, int end_c);
	// if take a piece, then assign the value
	void Take_Piece(Piece * t_Piece);	

	
	PieceType GetType_Moving() const;
	
	PieceColor GetColor_Moving() const;

	bool IsAttackPieceHere() const;

	PieceType GetType_Attack() const;
	
	PieceColor GetColor_Attack() const;	

	Piece * Get_Moving_Piece() const;
	
	Piece * Get_Attack_Piece() const;
	int Get_S_Row() const;

	int Get_S_Column() const;

	int Get_E_Row() const;

	int Get_E_Column() const;


};

#endif
