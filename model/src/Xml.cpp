#include "Xml.h"


void Xml::WriteIntoFile(ChessBoard * board, std::vector<PieceHistory* > & history) {
	std::ofstream file(file_name.c_str());

	if (!file.is_open())
		std::cout<<"open fail!!!"<<std::endl;

	file <<"-<chessgame>\n";

	WriteBoard(file, board);
	WriteHistory(file, history);

	file <<" </chessgame>\n";

	file.close();
}

void Xml::WriteBoard(std::ofstream & file, ChessBoard * board) {
	file <<" -<board>\n";
	for (int i=0; i<8; i++) {
		for (int j=0; j<8; j++) {
			Piece * temp = board->GetPiece(i, j);
			if (temp != NULL) {
				std::string type = temp->Type_String();
				std::string color = temp->Color_String();
				file <<"   <piece type=\""<<type<<"\" color=\""<<color<<"\" column=\""<<j<<"\" row=\""<<i<<"\" />"<<std::endl;	
			}
		}
	}

	file <<"  </board>\n";	
}


void Xml::WriteHistory(std::ofstream & file, std::vector<PieceHistory *> & history){
	PieceHistory * temp = '\0';
	Piece * piece = '\0';

	file <<" -<history>\n";
	for (int i = 0; i<history.size(); i++) {
		temp = history.at(i);
		piece = temp->Get_Moving_Piece();

		std::string type = piece->Type_String();
		std::string color = piece->Color_String();
		int s_row = temp->Get_S_Row();
		int s_col = temp->Get_S_Column();
		int e_row = temp->Get_E_Row();
		int e_col = temp->Get_E_Column();
		file <<"   <move>"<<std::endl;
		file <<"       <piece type=\""<<type<<"\" color=\""<<color<<"\" column=\""<<s_col<<"\" row=\""<<s_row<<"\" />"<<std::endl;	
		file <<"       <piece type=\""<<type<<"\" color=\""<<color<<"\" column=\""<<e_col<<"\" row=\""<<e_row<<"\" />"<<std::endl;	

		if (temp->IsAttackPieceHere()) {
			piece = temp->Get_Attack_Piece();
			std::string a_type = piece->Type_String();
			std::string a_color = piece->Color_String();
			file <<"       <piece type=\""<<a_type<<"\" color=\""<<a_color<<"\" column=\""<<e_col<<"\" row=\""<<e_row<<"\" />"<<std::endl;	
		}

		file <<"   </move>"<<std::endl;				
	}	

	file <<"  </history>\n";	
}



