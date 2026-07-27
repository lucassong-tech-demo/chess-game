#include "Piece.h"

#include <stdexcept>

#include "ChessBoard.h"

Piece::Piece(PieceType piece_type, PieceColor piece_color, int piece_row, int piece_column)
	: type(piece_type),
	  color(piece_color),
	  row(piece_row),
	  column(piece_column)
{
	if (!ChessBoard::IsValidPosition(row, column)) {
		throw std::out_of_range("piece coordinates must be between 0 and 7");
	}
}

PieceType Piece::GetType() const noexcept
{
	return type;
}

PieceColor Piece::GetColor() const noexcept
{
	return color;
}

int Piece::GetRow() const noexcept
{
	return row;
}

int Piece::GetColumn() const noexcept
{
	return column;
}

std::string Piece::Type_String() const
{
	switch (type) {
	case KING: return "king";
	case QUEEN: return "queen";
	case KNIGHT: return "knight";
	case BISHOP: return "bishop";
	case ROOK: return "rook";
	case PAWN: return "pawn";
	}
	throw std::logic_error("unknown piece type");
}

std::string Piece::Color_String() const
{
	switch (color) {
	case WHITE: return "white";
	case BLACK: return "black";
	}
	throw std::logic_error("unknown piece color");
}

void Piece::Update_location(int new_row, int new_column)
{
	if (!ChessBoard::IsValidPosition(new_row, new_column)) {
		throw std::out_of_range("piece coordinates must be between 0 and 7");
	}
	row = new_row;
	column = new_column;
}
