#include "HumanPlayer.h"


HumanPlayer::HumanPlayer(PieceColor color, GameFacade & gf) {
	this->color = color;  // white = 0, black = 1;
	game = &gf;
	move_choices = '\0';
	myPiece = '\0';
}

HumanPlayer::~HumanPlayer() {
	
}
	
void HumanPlayer::on_CellSelected(int row, int col, int button) {

	myPiece = game->GetPiece(row, col, color);
	
	if (myPiece != NULL) {
		//std::cout<<"it is.. "<<myPiece->GetType()<<" & "
				//<<"color: "<<myPiece->GetColor()<<std::endl;
		type = myPiece->GetType();
		move_choices = &(game->GetValidMoves());
		//std::cout<<"Oh I got movessss!"<<std::endl;
	}
	
}

void HumanPlayer::on_TimerEvent() {
	return;
}

std::set<BoardPosition> & HumanPlayer::GetValidMoves() {
	return * move_choices;
}

bool HumanPlayer::isMyPiece()const {
	if (myPiece == NULL)
		return false;
	return true;
}

bool HumanPlayer::isValidMove(int row, int col) {
	//move_choices = '\0';
	return game->isValidMove(row, col);
}

PieceType HumanPlayer::Piece_Type()const {
	return type;
}

PieceColor HumanPlayer::Piece_Color()const {
	return color;
}
	
bool HumanPlayer::isTaken(int row, int col) {
	return game->isCellTaken(row, col);
}

void HumanPlayer::PlacePiece(int row_selected, int col_selected, int row, int col) {
	game->MovePiece(row_selected, col_selected, row, col);
}


