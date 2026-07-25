#include "PieceHistory.h"


PieceHistory::PieceHistory(Piece * m_Piece, int s_row, int s_col, int end_r, int end_c) {
	moving_piece = m_Piece;
	start_row = s_row;
	start_column = s_col;
	end_row = end_r;
	end_column = end_c;
	attacked_piece = '\0';
}


void PieceHistory::Take_Piece(Piece * t_Piece) {
	attacked_piece = t_Piece;
}

PieceType PieceHistory::GetType_Moving() const {
	return moving_piece->GetType();
}
	
PieceColor PieceHistory::GetColor_Moving() const {
	return moving_piece->GetColor();
}

PieceType PieceHistory::GetType_Attack() const {
	return attacked_piece->GetType();
}

PieceColor PieceHistory::GetColor_Attack() const {
	return attacked_piece->GetColor();
}

Piece * PieceHistory::Get_Moving_Piece() const {
	return moving_piece;
}

Piece * PieceHistory::Get_Attack_Piece() const {
	return attacked_piece;
}

bool PieceHistory::IsAttackPieceHere() const {
	if (attacked_piece == NULL)
		return false;
	return true;
}

int PieceHistory::Get_S_Row() const {
	 return start_row;
}

int PieceHistory::Get_S_Column() const {
	return start_column;
}

int PieceHistory::Get_E_Row() const {
	return end_row;
}

int PieceHistory::Get_E_Column() const {
	return end_column;
}


