#include "Piece.h"


Piece::Piece(PieceType type, PieceColor color, int row, int column) {

	this->type = type;
	this->color = color;
	this->row = row;
	this->column = column;

}

PieceType Piece::GetType()const {
	return type;
}

PieceColor Piece::GetColor()const {
	return color;
}

int Piece::GetRow()const {
	return row;
}

int Piece::GetColumn()const {
	return column;
}


std::string Piece::Type_String() {
	std::string s_type = "";

	switch(type) {
	case KING:
			s_type = "king";
			break;
	case QUEEN:
			s_type = "queen";
			break;
	case KNIGHT:
			s_type = "knight";
			break;
	case BISHOP:
			s_type = "bishop";
			break;
	case ROOK:
			s_type = "rook";
			break;
	case PAWN:
			s_type = "pawn";
			break;
	
	}
	
	assert(s_type != "");
	
	//std::cout<<"type: "<<s_type<<std::endl;
	return s_type;
}

std::string Piece::Color_String() {
	std::string s_color = "";

	if (color == WHITE)
		s_color = "white";
	else
		s_color = "black";
	
	assert(s_color != "");
	//std::cout<<"color: "<<s_color<<std::endl;
	return s_color;

}

void Piece::Update_location(const int r, const int col) {
	row = r;
	column = col;
}
