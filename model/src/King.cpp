#include "King.h"
#include "ChessBoard.h"

std::set<BoardPosition> & King::GetValidMove(ChessBoard * board) {
	
	if (movements != NULL)
		delete movements;

	movements = new std::set<BoardPosition>;
	
	Forward(board);
	Forward_left(board);
	Forward_right(board);
	Right(board);
	Left(board);
	Back(board);
	Back_left(board);
	Back_right(board);

	return *movements;
}

void King::Forward(ChessBoard * board) {
	if (color == WHITE && row-1 >= 0) {
		Piece * piece1 = board->GetPiece(row-1, column);
		if ( piece1 == NULL || piece1->GetColor() != color) {
			BoardPosition bp1(row-1, column);
			movements->insert(bp1);
		}
	}
	else if (color == BLACK && row+1 <= 7) {
		Piece * piece1 = board->GetPiece(row+1, column);
		if ( piece1 == NULL || piece1->GetColor() != color) {
			BoardPosition bp1(row+1, column);
			movements->insert(bp1);
		}
	}
}

void King::Forward_left(ChessBoard * board) {
	if (color == WHITE ) {
		if (row-1 >= 0 && column-1 >= 0) {
			Piece * piece1 = board->GetPiece(row-1, column-1);
			if ( piece1 == NULL || piece1->GetColor() != color) {
				BoardPosition bp1(row-1, column-1);
				movements->insert(bp1);
			}
		}
	}
	else {
		if (row+1 <= 7 && column+1 <= 7) {
			Piece * piece1 = board->GetPiece(row+1, column+1);
			if ( piece1 == NULL || piece1->GetColor() != color) {
				BoardPosition bp1(row+1, column+1);
				movements->insert(bp1);
			}
		}
	}
}

void King::Forward_right(ChessBoard * board) {
	if (color == WHITE) {
		if (row-1 >= 0 && column+1 <= 7) {
			Piece * piece1 = board->GetPiece(row-1, column+1);
			if ( piece1 == NULL || piece1->GetColor() != color) {
				BoardPosition bp1(row-1, column+1);
				movements->insert(bp1);
			}
		}
	}
	else {
		if (row+1 <= 7 && column-1 >= 0) {
			Piece * piece1 = board->GetPiece(row+1, column-1);
			if ( piece1 == NULL || piece1->GetColor() != color) {
				BoardPosition bp1(row+1, column-1);
				movements->insert(bp1);
			}
		}
	}
}

void King::Right(ChessBoard * board) {
	if (color == WHITE && column+1 <= 7) {
		Piece * piece1 = board->GetPiece(row, column+1);
		if ( piece1 == NULL || piece1->GetColor() != color) {
			BoardPosition bp1(row, column+1);
			movements->insert(bp1);
		}
	}
	else if (color == BLACK && column-1 >= 0){
		Piece * piece1 = board->GetPiece(row, column-1);
		if ( piece1 == NULL || piece1->GetColor() != color) {
			BoardPosition bp1(row, column-1);
			movements->insert(bp1);
		}
	}
}

void King::Left(ChessBoard * board) {
	if (color == WHITE && column-1 >= 0) {
		Piece * piece1 = board->GetPiece(row, column-1);
		if ( piece1 == NULL || piece1->GetColor() != color) {
			BoardPosition bp1(row, column-1);
			movements->insert(bp1);
		}
	}
	else if (color == BLACK && column+1 <= 7){
		Piece * piece1 = board->GetPiece(row, column+1);
		if ( piece1 == NULL || piece1->GetColor() != color) {
			BoardPosition bp1(row, column+1);
			movements->insert(bp1);
		}
	}
}

void King::Back(ChessBoard * board) {
	if (color == WHITE && row+1 <= 7) {
		Piece * piece1 = board->GetPiece(row+1, column);
		if ( piece1 == NULL || piece1->GetColor() != color) {
			BoardPosition bp1(row+1, column);
			movements->insert(bp1);
		}
	}
	else if (color == BLACK && row-1 >= 0) {
		Piece * piece1 = board->GetPiece(row-1, column);
		if ( piece1 == NULL || piece1->GetColor() != color) {
			BoardPosition bp1(row-1, column);
			movements->insert(bp1);
		}
	}
}

void King::Back_left(ChessBoard * board) {
	if (color == WHITE) {
		if (row+1 <= 7 && column-1 >= 0) {
			Piece * piece1 = board->GetPiece(row+1, column-1);
			if ( piece1 == NULL || piece1->GetColor() != color) {
				BoardPosition bp1(row+1, column-1);
				movements->insert(bp1);
			}
		}
	}
	else {
		if (row-1 >= 0 && column+1 <= 7) {
			Piece * piece1 = board->GetPiece(row-1, column+1);
			if ( piece1 == NULL || piece1->GetColor() != color) {
				BoardPosition bp1(row-1, column+1);
				movements->insert(bp1);
			}
		}
	}
}

void King::Back_right(ChessBoard * board) {
	if (color == WHITE) {
		if (row+1 <= 7 && column+1 <= 7) {
			Piece * piece1 = board->GetPiece(row+1, column+1);
			if ( piece1 == NULL || piece1->GetColor() != color) {
				BoardPosition bp1(row+1, column+1);
				movements->insert(bp1);
			}
		}
	}
	else {
		if (row-1 >= 0 && column-1 >= 0) {
			Piece * piece1 = board->GetPiece(row-1, column-1);
			if ( piece1 == NULL || piece1->GetColor() != color) {
				BoardPosition bp1(row-1, column-1);
				movements->insert(bp1);
			}
		}
	}
}

