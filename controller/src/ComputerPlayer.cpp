#include "ComputerPlayer.h"


ComputerPlayer::ComputerPlayer(PieceColor color, GameFacade & gf) {
	this->color = color;  // white = 0, black = 1;
	game = &gf;
	move_choices = '\0';
	myPiece = '\0';
	row = -1;
	col = -1;
	
}

ComputerPlayer::~ComputerPlayer() {

}
	
void ComputerPlayer::on_CellSelected(int row, int col, int button) {

	return;
}

void ComputerPlayer::on_TimerEvent() {
	//std::cout<<color<<" turn:"<<std::endl;
	srand ( time(NULL) );
	do {
		row = rand()%8;
		col = rand()%8;
		
		myPiece = game->GetPiece(row, col, color);  //get random piece, which has available moves
		
		if (myPiece != NULL) {
			type = myPiece->GetType();
			move_choices = &(game->GetValidMoves());
		
			if (move_choices->size() == 0) {
				move_choices = '\0';
			}
		}
	
	} while (myPiece == NULL || move_choices == NULL);
}

bool ComputerPlayer::isMyPiece()const {
	if (myPiece == NULL)
		return false;
	return true;
}

std::set<BoardPosition> & ComputerPlayer::GetValidMoves() {
	
	return * move_choices;
}

bool ComputerPlayer::isValidMove(int row, int col) {
	
	return game->isValidMove(row, col);
}

int ComputerPlayer::GetRow() {
	
	return row;
}

int ComputerPlayer::GetColumn() {
	return col;
}

PieceType ComputerPlayer::Piece_Type()const {
	return type;
}

PieceColor ComputerPlayer::Piece_Color()const {
	return color;
}
	
bool ComputerPlayer::isTaken(int row, int col) {
	
	return game->isCellTaken(row, col);
}

void ComputerPlayer::PlacePiece(int row_selected, int col_selected, int row, int col) {
	
	game->MovePiece(row_selected, col_selected, row, col);
}

