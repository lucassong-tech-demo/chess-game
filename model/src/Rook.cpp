#include "Rook.h"
#include "ChessBoard.h"

std::set<BoardPosition> & Rook::GetValidMove(ChessBoard * board) {
	if (movements != NULL)
		delete movements;

	movements = new std::set<BoardPosition>;

	Forward(board);
	Right(board);
	Left(board);
	Back(board);
	return *movements;

}

void Rook::Forward(ChessBoard * board) {
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

void Rook::Right(ChessBoard * board) {
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

void Rook::Left(ChessBoard * board) {
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

void Rook::Back(ChessBoard * board) {
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

