#include "GameFacade.h"



GameFacade::GameFacade() {
	//std::cout<<"**** game facade ****"<<std::endl;
	board = '\0'; 
	cur_Piece = '\0';
	attack_Piece = '\0';
	valid_moves = '\0';
	undo_once = '\0';
}

GameFacade::~GameFacade() {
	Clear_Board();
	ClearUndo();   //firstly, clear undo_once pointer, to avoid memory leak
	ClearHistory();  // then clear the history, because using undo_once point to get each object
	//std::cout<<"GameFacade ~destructor"<<std::endl;
}

void GameFacade::Clear_Board() {
	if (board != NULL){
		delete board;
		board = '\0';
	}

	if (valid_moves != NULL) {
		delete valid_moves;
		valid_moves = '\0';
	}
}

void GameFacade::ClearUndo() {
	if (undo_once != NULL) {
		delete undo_once;
		undo_once = '\0';
	}
}

void GameFacade::ClearHistory() {
	while (move_history.size() != 0) {
		undo_once = move_history.back();
		move_history.pop_back();
		delete undo_once;
		undo_once = '\0';
	}
}

void GameFacade::NewGame() {
	Clear_Board();
	ClearUndo();
	ClearHistory();
	cur_Piece = '\0';
	attack_Piece = '\0';
	board = new ChessBoard();

}
/*
void GameFacade::LoadGame() {
	Clear_Board();
	ClearUndo();
	ClearHistory();
	cur_Piece = '\0';
	attack_Piece = '\0';
	board = new ChessBoard();
}
*/
bool GameFacade::isValidMove(int row, int col) {

	bool check = false;
	for (std::set< BoardPosition > :: const_iterator iter = valid_moves->begin();
		iter != valid_moves->end(); ++iter) {

		int r = iter->GetRow();
		int c = iter->GetColumn();
		if (row == r && col == c) {
			check = true;
			break;
		}
	}
	return check;
}

bool GameFacade::isCellTaken(int row, int col) {

	attack_Piece = board->GetPiece(row, col);

	if (attack_Piece != NULL) 
	//	if (attack_Piece->GetColor() != cur_Piece->GetColor())
		return true;

	return false;
}

Piece * GameFacade::GetPiece(int row, int col, PieceColor color) {
	//std::cout<<"looking for piece..."<<std::endl;
//	cur_Piece = Get_Piece_Fron_Board(row, col);
	cur_Piece = board->GetPiece(row, col);    //cur_Piece is change to the item of current cell
	
	//std::cout<<"I got it...."<<std::endl;
	if (cur_Piece != NULL && cur_Piece->GetColor() != color) {
		cur_Piece = '\0';
		//std::cout<<"NO, it's wrong..."<<std::endl;
	}
	
	return cur_Piece;
}

std::set<BoardPosition> & GameFacade::GetValidMoves() {

	if (valid_moves != NULL)
		delete valid_moves;

	valid_moves = new std::set<BoardPosition>;
	Look_for_Moves();
	//std::cout<<"pick up valid move....."<<std::endl;
	return * valid_moves;
}

void GameFacade::Look_for_Moves() {
	//std::cout<<"facade check the movements.."<<std::endl;
	assert(cur_Piece != NULL);
	assert(board != NULL);

	//valid_moves = &(cur_Piece->GetValidMove(board));
	std::set<BoardPosition> * temp_moves = &(cur_Piece->GetValidMove(board));
	Moves_Filter(temp_moves);    // avoid King in Check
}

void GameFacade::Moves_Filter(std::set<BoardPosition> * moves) {

	for (std::set< BoardPosition > :: const_iterator iter = moves->begin();
		iter != moves->end(); ++iter) {
		int r = iter->GetRow();
		int c = iter->GetColumn();

		// check if cur piece move there, and my King is in daugrous!!!
		std::set<BoardPosition> * allMoves = &(Board_Check(cur_Piece->GetRow(), cur_Piece->GetColumn(), r, c));
		//get my king
		Piece * king = board->GetKing(cur_Piece->GetColor());
		bool wrong_move = false;

		for (std::set< BoardPosition > :: const_iterator iter_m = allMoves->begin();
			iter_m != allMoves->end(); ++iter_m) {
			int possible_r = iter_m->GetRow();
			int possible_c = iter_m->GetColumn();
			//std::cout<<"king is here: "<<king->GetRow()<<", "<<king->GetColumn()<<std::endl;
			//std::cout<<"so...." <<possible_r<<", "<<possible_c<<std::endl;
			if (cur_Piece == king) {
				if ( possible_r == r && possible_c == c) {
					wrong_move = true;
					break;
				}
			}
			else if (king->GetRow() == possible_r && king->GetColumn() == possible_c) {
				wrong_move = true;
				break;
			}
		}

		if (!wrong_move) {
			valid_moves->insert(*iter);
		}
		
		delete allMoves;	
		
	}	

}

std::set<BoardPosition> & GameFacade::Board_Check(int row, int col, int new_r, int new_c) {
	//if (allMoves != NULL)
		//delete allMoves;	
	std::set<BoardPosition> * allMoves = new std::set<BoardPosition>;

	Piece * temp ='\0';
	// if there is piece taken that cell
	if (board->GetPiece(new_r, new_c) != NULL)
		temp = board->GetPiece(new_r, new_c);
	PieceColor color = cur_Piece->GetColor();

	board->MovePiece(row, col, new_r, new_c);

	for (int i =0; i<8; i++) {
		for (int j=0; j<8; j++){
			Piece * each_piece = board->GetPiece(i,j);
			if (each_piece != NULL && each_piece->GetColor() != color) {
				std::set<BoardPosition> * moves = &(each_piece->GetValidMove(board));
				Build_Move_Set(moves, allMoves);
			}
		}
	}	

	board->Reset_Piece(row, col, new_r, new_c);

	if (temp != NULL)
		board->PutBack_Piece(temp, new_r, new_c);

	return *allMoves;
				
}

void GameFacade::Build_Move_Set(std::set<BoardPosition> * moves, std::set<BoardPosition> * allMoves) {
	for (std::set< BoardPosition > :: const_iterator iter = moves->begin();
		iter != moves->end(); ++iter) {
		allMoves->insert(*iter);
	}

}

void GameFacade::MovePiece(int r_selected, int col_selected, int row, int col) {
	
	//std::cout<<"\tfacade MovePiece..."<<std::endl;
	cur_Piece = board->GetPiece(r_selected, col_selected);   // change cur_Piece back.
	assert(cur_Piece != NULL);
	PieceHistory * history = new PieceHistory(cur_Piece, r_selected, col_selected, row, col);

	if (attack_Piece != NULL) {
		history->Take_Piece(attack_Piece);
	}
	move_history.push_back(history);
	board->MovePiece(r_selected, col_selected, row, col);
	// finally, delete the valid movents set.... don't forget!!!
	// but it should be happened in the Gamecontroller, because we need undo hight cells
	
}

PieceHistory * GameFacade::Undo(){
	// Since undo_once pointer points to the lastest privous undo Move, firstly
	// check if it point to NULL, if not , delete it.
	ClearUndo();
	
	if (move_history.size() != 0) {
		undo_once = move_history.back();
		move_history.pop_back();
		
		int s_row = undo_once->Get_S_Row();
		int s_col = undo_once->Get_S_Column();
		int end_row = undo_once->Get_E_Row();
		int end_col = undo_once->Get_E_Column();
		//std::cout<<"\tfrom ("<<s_row<<","<<s_col<<") to ("<<end_row<<","<<end_col<<")"<<std::endl;
		board->Reset_Piece(s_row, s_col, end_row, end_col);

		if (undo_once->IsAttackPieceHere()) {
			Piece * attacked_p = undo_once->Get_Attack_Piece();
			board->PutBack_Piece(attacked_p, end_row, end_col);
		}
	}
	//the poped PieceHistory object will be deleted after GUI update by ChessController.
	return undo_once;  
}

void GameFacade::SaveGame(const std::string & fileName) {
	Xml xml(fileName);
	xml.WriteIntoFile(board, move_history);

}

bool GameFacade::Check(int row, int col) {
	assert(cur_Piece != NULL);
	assert(board != NULL);
	std::set<BoardPosition> * temp_moves = &(cur_Piece->GetValidMove(board));

	Piece * king = '\0';

	if ( cur_Piece->GetColor() == WHITE)
		king = board->GetKing(BLACK);
	else
		king = board->GetKing(WHITE);

	for (std::set< BoardPosition > :: const_iterator iter = temp_moves->begin();
		iter != temp_moves->end(); ++iter) {
		int r = iter->GetRow();
		int c = iter->GetColumn();
	
		if (king->GetRow() == r && king->GetColumn() == c)
			return true;
	}
	
	return false;
}

// Check current status is in stuck
bool GameFacade::Mate(int row, int col) {
	assert(cur_Piece != NULL);
	assert(board != NULL);

	PieceColor color = cur_Piece->GetColor();
	//std::cout<<"current piece color: "<<cur_Piece->GetColor()<<std::endl;
	for (int i =0; i<8; i++) {
		for (int j=0; j<8; j++){
			cur_Piece = board->GetPiece(i,j);
			if (cur_Piece != NULL && cur_Piece->GetColor() != color) {
				GetValidMoves();
				//std::cout<<"???????????????? size: "<<valid_moves->size()<<std::endl;
				if (valid_moves->size() != 0){
					//std::cout<<"who ;; "<<cur_Piece->Type_String()<<" "<<cur_Piece->GetRow()<<" , "<<cur_Piece->GetColumn()<<std::endl;
					return false;
				}
			}
		}
	}
		
	return true;

}

void GameFacade::Quit() {
	std::cout<<"Quit Facade ... "<<std::endl;	
	//Clear_Board();
}
	
bool GameFacade::Test(std::ostream & os) {
	bool success = true;
	GameFacade game;
	game.NewGame();
	
	Piece * p1 = game.GetPiece(0, 4, BLACK);
	TEST(p1->GetType() == KING);
	if (success) {
		TEST(p1->GetColor() == BLACK);
		if (success)
			std::cout<<"\tGet Piece from board testing success."<<std::endl;	
		else
			std::cout<<"\tGet Piece test failed!!!"<<std::endl;	
	}
	else
		std::cout<<"\tGet Piece test failed!!!"<<std::endl;	
	
//************************************************

	game.GetPiece(7, 6, WHITE);  // w_knight
	game.isCellTaken(1,1);    // must check if the cell is token by 
	game.MovePiece(7, 6, 1, 1);	     // w_knight -> b_pawn
	TEST((game.board)->GetPiece(7,6) == NULL);
	if (success) {
		Piece* q1 = (game.board)->GetPiece(1,1);
		TEST(q1->GetType() == KNIGHT);
		if (success) {
			TEST(q1->GetColor() == WHITE);
			if (success)
				std::cout<<"\tMove Piece testing success."<<std::endl;
			else	
				std::cout<<"\tMove Piece testing failed!!!!"<<std::endl;	
		}
		else	
			std::cout<<"\tMove Piece testing failed!!!!"<<std::endl;	
	}
	else
		std::cout<<"\tMove Piece testing failed!!!!"<<std::endl;	
//**************************************

	game.GetPiece(0,2,BLACK);   // b_bishop
	game.isCellTaken(2,1);       // must check if the cell is token by 
	game.MovePiece(0,2, 2, 1);    // black bishop doesn't attack white knight
	TEST((game.move_history).size() == 2);
	if (success) {
		std::cout<<"\tMove_History size testing success."<<std::endl;
		PieceHistory * q2 = (game.move_history).front();
		TEST(q2->GetType_Attack() == PAWN);
		if (success) {
			std::cout<<"\tMove_History Attacked piece record testing success. --- front."<<std::endl;
			TEST(q2->GetType_Moving() == KNIGHT);
			if (success)
				std::cout<<"\tMove_History Moving piece record testing success."<<std::endl;
			else
				std::cout<<"\tMove_History Moving piece record testing Failed!!"<<std::endl;
		}
		else
			std::cout<<"\tMove_History Attacked piece record testing Failed!!"<<std::endl;

		PieceHistory * q3 = (game.move_history).back();
		
		//TEST(q3->GetType_Attack() == KNIGHT);
		TEST(q3->Get_Attack_Piece() == NULL);
		if (success) {
			std::cout<<"\tMove_History Attacked piece record testing success. --- back."<<std::endl;
			TEST(q3->GetType_Moving() == BISHOP);
			if (success)
				std::cout<<"\tMove_History Moving piece record testing success."<<std::endl;
			else
				std::cout<<"\tMove_History Moving piece record testing Failed!!"<<std::endl;
		}
		else
			std::cout<<"\tMove_History Attacked piece record testing Failed!!"<<std::endl;
	}
	else
		std::cout<<"\tMove_History size testing failed!!!"<<std::endl;
//******************************************************
	Piece * p4 = (game.board)->GetPiece(0,2);
	TEST(p4 == NULL);
	if (success) {
		game.Undo();
		TEST((game.move_history).size() == 1);
		if (success)	
			std::cout<<"\tMove_History Undo size testing success."<<std::endl;
		else
			std::cout<<"\tMove_History Undo size testing Failed!!"<<std::endl;
		p4 = (game.board)->GetPiece(0,2);
		TEST(p4 != NULL);
		if (success) {
			TEST(p4->GetType() == BISHOP);
			if (success) {
				TEST(p4->GetColor() == BLACK);
				if (success) {
					TEST((game.board)->GetPiece(2, 1) == NULL);
					if (success) 
						std::cout<<"\tFirst Undo testing passed!!!"<<std::endl;
					else
						std::cout<<"\tFirst Undo testing failed!!!, since current loc is not NULL"<<std::endl;
				}
				else
					std::cout<<"\tFirst Undo testing failed!!!, since piece color is not right."<<std::endl;
				
			}
		}
		else
			std::cout<<"\tUndo testing failed!!!, since bishop is not moved back."<<std::endl;
	}
	else
		std::cout<<"\tUndo testing failed!!!, since (0,2) is not NULL."<<std::endl;

	game.Undo();
	Piece * p5 = (game.board)->GetPiece(1,1);
	Piece * p6 = (game.board)->GetPiece(7,6);
	TEST(p5->GetType() == PAWN);
	if (success)
		std::cout<<"\tPawn Undo testing passed!!!"<<std::endl;
	else
		std::cout<<"\tPawn Undo testing failed!!!, since (1,1) should change back to B_Pawn"<<std::endl;
	TEST(p6->GetType() == KNIGHT);
	if (success)
		std::cout<<"\tKnight Undo testing passed!!!"<<std::endl;
	else
		std::cout<<"\tKnight Undo testing failed!!!, since (1,1) should change back to W_knight"<<std::endl;
	TEST((game.move_history).size() == 0);
	if (success)
		std::cout<<"\tFinally Undo testing passed!!!"<<std::endl;
	else
		std::cout<<"\tUndo testing failed!!!, since move_history size is not zero."<<std::endl;

//*******************************************************
	game.GetPiece(7, 6, WHITE);  // w_knight
	game.isCellTaken(1,1);    // must check if the cell is token by 
	game.MovePiece(7, 6, 1, 1);	     // w_knight -> b_pawn

	game.GetPiece(0,2,BLACK);   // b_bishop
	game.isCellTaken(2,1);       // must check if the cell is token by 
	game.MovePiece(0,2, 2, 1);    // black bishop doesn't attack white knight
	TEST((game.move_history).size() == 2);
	if (success) {
		game.ClearHistory();
		TEST((game.move_history).size() == 0);
		if (success)
			std::cout<<"\tClear Move History testing passed!!!"<<std::endl;
		else
			std::cout<<"\tClear Move History testing failed!!!!"<<std::endl;
	}
	else
		std::cout<<"\tAdd Move history testing failed!!!"<<std::endl;
	return success;
}


