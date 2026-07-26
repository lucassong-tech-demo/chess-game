#include "Xml.h"

#include <stdexcept>

namespace {

std::string TypeString(PieceType type)
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

std::string ColorString(PieceColor color)
{
	return color == WHITE ? "white" : "black";
}

} // namespace

void Xml::WriteIntoFile(
	const ChessBoard & board,
	const std::vector<PieceHistory> & history)
{
	std::ofstream file(file_name);
	if (!file.is_open()) {
		throw std::runtime_error("unable to open save file: " + file_name);
	}

	file << "-<chessgame>\n";
	WriteBoard(file, board);
	WriteHistory(file, history);
	file << " </chessgame>\n";
}

void Xml::WriteBoard(std::ofstream & file, const ChessBoard & board)
{
	file << " -<board>\n";
	for (int row = 0; row < ChessBoard::Size; ++row) {
		for (int col = 0; col < ChessBoard::Size; ++col) {
			const Piece * piece = board.GetPiece(row, col);
			if (piece) {
				file << "   <piece type=\"" << piece->Type_String()
					 << "\" color=\"" << piece->Color_String()
					 << "\" column=\"" << col << "\" row=\"" << row
					 << "\" />\n";
			}
		}
	}
	file << "  </board>\n";
}

void Xml::WriteHistory(
	std::ofstream & file,
	const std::vector<PieceHistory> & history)
{
	file << " -<history>\n";
	for (const PieceHistory & move : history) {
		const auto & moving = move.GetMovingSnapshot();
		file << "   <move>\n"
			 << "       <piece type=\"" << TypeString(moving.type)
			 << "\" color=\"" << ColorString(moving.color)
			 << "\" column=\"" << move.Get_S_Column()
			 << "\" row=\"" << move.Get_S_Row() << "\" />\n"
			 << "       <piece type=\"" << TypeString(moving.type)
			 << "\" color=\"" << ColorString(moving.color)
			 << "\" column=\"" << move.Get_E_Column()
			 << "\" row=\"" << move.Get_E_Row() << "\" />\n";

		if (const auto & attacked = move.GetAttackSnapshot()) {
			file << "       <piece type=\"" << TypeString(attacked->type)
				 << "\" color=\"" << ColorString(attacked->color)
				 << "\" column=\"" << move.Get_E_Column()
				 << "\" row=\"" << move.Get_E_Row() << "\" />\n";
		}
		file << "   </move>\n";
	}
	file << "  </history>\n";
}
