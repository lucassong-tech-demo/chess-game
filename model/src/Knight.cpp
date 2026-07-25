#include "Knight.h"
#include "ChessBoard.h"

std::set<BoardPosition> & Knight::GetValidMove(ChessBoard * board) {
	if (movements != NULL)
		delete movements;

	movements = new std::set<BoardPosition>;
	
	Forward_left(board);

	Forward_right(board);

	Left_forward(board);

	Right_forward(board);

	Back_left(board);

	Back_right(board);

	Left_back(board);

	Right_back(board);

	return *movements;

}

void Knight::Forward_left(ChessBoard * board) {
	if (row-2 >= 0 && column-1 >= 0) {
		Piece * piece = board->GetPiece(row-2, column-1);
		if ( piece != NULL) {
			if ( piece->GetColor() != color) {
				BoardPosition bp(row-2, column-1);
				movements->insert(bp);
			}
		}
		else {
			BoardPosition bp(row-2, column-1);
			movements->insert(bp);
		}
	}
}

void Knight::Forward_right(ChessBoard * board) {
	if (row-2 >= 0 && column+1 <= 7) {
		Piece * piece = board->GetPiece(row-2, column+1);
		if ( piece != NULL) {
			if ( piece->GetColor() != color) {
				BoardPosition bp(row-2, column+1);
				movements->insert(bp);
			}
		}
		else {
			BoardPosition bp(row-2, column+1);
			movements->insert(bp);
		}
	}
}

void Knight::Left_forward(ChessBoard * board) {
	if (row-1 >= 0 && column-2 >= 0) {
		Piece * piece = board->GetPiece(row-1, column-2);
		if ( piece != NULL) {
			if ( piece->GetColor() != color) {
				BoardPosition bp(row-1, column-2);
				movements->insert(bp);
			}
		}
		else {
			BoardPosition bp(row-1, column-2);
			movements->insert(bp);
		}
	}
}

void Knight::Right_forward(ChessBoard * board) {
	if (row-1 >= 0 && column+2 <= 7) {
		Piece * piece = board->GetPiece(row-1, column+2);
		if ( piece != NULL) {
			if ( piece->GetColor() != color) {
				BoardPosition bp(row-1, column+2);
				movements->insert(bp);
			}
		}
		else {
			BoardPosition bp(row-1, column+2);
			movements->insert(bp);
		}
	}
}

void Knight::Back_left(ChessBoard * board) {
	if (row+2 <= 7 && column-1 >= 0) {
		Piece * piece = board->GetPiece(row+2, column-1);
		if ( piece != NULL) {
			if ( piece->GetColor() != color) {
				BoardPosition bp(row+2, column-1);
				movements->insert(bp);
			}
		}
		else {
			BoardPosition bp(row+2, column-1);
			movements->insert(bp);
		}
	}
}

void Knight::Back_right(ChessBoard * board) {
	if (row+2 <= 7 && column+1 <= 7) {
		Piece * piece = board->GetPiece(row+2, column+1);
		if ( piece != NULL) {
			if ( piece->GetColor() != color) {
				BoardPosition bp(row+2, column+1);
				movements->insert(bp);
			}
		}
		else {
			BoardPosition bp(row+2, column+1);
			movements->insert(bp);
		}
	}
}

void Knight::Left_back(ChessBoard * board) {
	if (row+1 <= 7 && column-2 >= 0) {
		Piece * piece = board->GetPiece(row+1, column-2);
		if ( piece != NULL) {
			if ( piece->GetColor() != color) {
				BoardPosition bp(row+1, column-2);
				movements->insert(bp);
			}
		}
		else {
			BoardPosition bp(row+1, column-2);
			movements->insert(bp);
		}
	}
}

void Knight::Right_back(ChessBoard * board) {
	if (row+1 <= 7 && column+2 <= 7) {
		Piece * piece = board->GetPiece(row+1, column+2);
		if ( piece != NULL) {
			if ( piece->GetColor() != color) {
				BoardPosition bp(row+1, column+2);
				movements->insert(bp);
			}
		}
		else {
			BoardPosition bp(row+1, column+2);
			movements->insert(bp);
		}
	}
}
