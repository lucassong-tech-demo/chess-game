#include "Bishop.h"
#include "ChessBoard.h"


std::set<BoardPosition> & Bishop::GetValidMove(ChessBoard * board) {
	if (movements != NULL)
		delete movements;

	movements = new std::set<BoardPosition>;
	//std::cout<<"************** ("<<row<<","<<column<<") **************"<<std::endl;
	Forward_left(board);
	Forward_right(board);
	Back_left(board);
	Back_right(board);

	return *movements;

}

void Bishop::Forward_left(ChessBoard * board) {
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

void Bishop::Forward_right(ChessBoard * board) {
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

void Bishop::Back_left(ChessBoard * board) {
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

void Bishop::Back_right(ChessBoard * board) {
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

