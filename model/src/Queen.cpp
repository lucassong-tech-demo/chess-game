#include "Queen.h"
#include "ChessBoard.h"

std::set<BoardPosition> & Queen::GetValidMove(ChessBoard * board) {
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

void Queen::Forward(ChessBoard * board) {
	int i = 1;
	while (row-i >= 0) {
		Piece * piece = board->GetPiece(row-i, column);
		if ( piece != NULL) {
			if ( piece->GetColor() != color) {
				BoardPosition bp(row-i, column);
				movements->insert(bp);
			}
			break;
		}
		BoardPosition bp(row-i, column);
		movements->insert(bp);
		i++;
	}
}

void Queen::Right(ChessBoard * board) {
	int i = 1;
	while (column+i <=7) {
		Piece * piece = board->GetPiece(row, column+i);
		if ( piece != NULL) {
			if ( piece->GetColor() != color) {
				BoardPosition bp(row, column+i);
				movements->insert(bp);
			}
			break;
		}
		BoardPosition bp(row, column+i);
		movements->insert(bp);
		i++;
	}
}

void Queen::Left(ChessBoard * board) {
	int i = 1;
	while (column-i >= 0) {
		Piece * piece = board->GetPiece(row, column-i);
		if ( piece != NULL) {
			if ( piece->GetColor() != color) {
				BoardPosition bp(row, column-i);
				movements->insert(bp);
			}
			break;
		}
		BoardPosition bp(row, column-i);
		movements->insert(bp);
		i++;
	}
}

void Queen::Back(ChessBoard * board) {
	int i = 1;
	while (row+i <= 7) {
		Piece * piece = board->GetPiece(row+i, column);
		if ( piece != NULL) {
			if ( piece->GetColor() != color) {
				BoardPosition bp(row+i, column);
				movements->insert(bp);
			}
			break;
		}
		BoardPosition bp(row+i, column);
		movements->insert(bp);
		i++;
	}
}


void Queen::Forward_left(ChessBoard * board) {
	int i = 1;
	while (row - i >= 0 && column - i >= 0) {
		Piece * piece = board->GetPiece(row-i, column-i);
		if ( piece != NULL) {
			if ( piece->GetColor() != color) {
				BoardPosition bp(row-i, column-i);
				movements->insert(bp);
			}
			break;
		}
		BoardPosition bp(row-i, column-i);
		movements->insert(bp);
		i++;
	}
}

void Queen::Forward_right(ChessBoard * board) {
	int i = 1;
	while (row - i >= 0 && column + i <= 7) {
		Piece * piece = board->GetPiece(row-i, column+i);
		if ( piece != NULL) {
			if ( piece->GetColor() != color) {
				BoardPosition bp(row-i, column+i);
				movements->insert(bp);
			}
			break;
		}
		BoardPosition bp(row-i, column+i);
		movements->insert(bp);
		i++;
	}
}

void Queen::Back_left(ChessBoard * board) {
	int i = 1;
	while (row + i <= 7 && column - i >= 0) {
		Piece * piece = board->GetPiece(row+i, column-i);
		if ( piece != NULL) {
			if ( piece->GetColor() != color) {
				BoardPosition bp(row+i, column-i);
				movements->insert(bp);
			}
			break;
		}
		BoardPosition bp(row+i, column-i);
		movements->insert(bp);
		i++;
	}
}

void Queen::Back_right(ChessBoard * board) {
	int i = 1;
	while (row + i <= 7 && column + i <= 7) {
		Piece * piece = board->GetPiece(row+i, column+i);
		if ( piece != NULL) {
			if ( piece->GetColor() != color) {
				BoardPosition bp(row+i, column+i);
				movements->insert(bp);
			}
			break;
		}
		BoardPosition bp(row+i, column+i);
		movements->insert(bp);
		i++;
	}
}

