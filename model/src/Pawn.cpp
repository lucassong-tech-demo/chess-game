#include "Pawn.h"
#include "ChessBoard.h"

std::set<BoardPosition> & Pawn::GetValidMove(ChessBoard * board) {
	
	if (movements != NULL)
		delete movements;

	movements = new std::set<BoardPosition>;
	Forward(board);
	Forward_left(board);
	Forward_right(board);
	return *movements;

}

void Pawn::Forward(ChessBoard * board) {
	if (color == WHITE) {
		Piece * piece1 = board->GetPiece(row-1, column);
		if ( piece1 == NULL) {
			BoardPosition bp1(row-1, column);
			movements->insert(bp1);
			if (row == 6) {
				Piece * piece2 = board->GetPiece(row-2, column);
				if (piece2 == NULL) {
					BoardPosition bp2(row-2, column);
					movements->insert(bp2);
				}
			}
		}
	}
	else {
		Piece * piece1 = board->GetPiece(row+1, column);
		if ( piece1 == NULL) {
			BoardPosition bp1(row+1, column);
			movements->insert(bp1);
			if (row == 1) {
				Piece * piece2 = board->GetPiece(row+2, column);
				if (piece2 == NULL) {
					BoardPosition bp2(row+2, column);
					movements->insert(bp2);
				}
			}
		}
	}
}

void Pawn::Forward_left(ChessBoard * board) {
	if (color == WHITE) {
		Piece * piece1 = board->GetPiece(row-1, column-1);
		if ( piece1 != NULL && piece1->GetColor() != color) {
			BoardPosition bp1(row-1, column-1);
			movements->insert(bp1);
		}
	}
	else {
		Piece * piece1 = board->GetPiece(row+1, column+1);
		if ( piece1 != NULL && piece1->GetColor() != color) {
			BoardPosition bp1(row+1, column+1);
			movements->insert(bp1);
		}
	}
}

void Pawn::Forward_right(ChessBoard * board) {
	if (color == WHITE) {
		Piece * piece1 = board->GetPiece(row-1, column+1);
		if ( piece1 != NULL && piece1->GetColor() != color) {
			BoardPosition bp1(row-1, column+1);
			movements->insert(bp1);
		}
	}
	else {
		Piece * piece1 = board->GetPiece(row+1, column-1);
		if ( piece1 != NULL && piece1->GetColor() != color) {
			BoardPosition bp1(row+1, column-1);
			movements->insert(bp1);
		}
	}
}


