#include "GameController.h"



GameController::GameController(int option) {

	std::cout<<"Game STart ... "<<option<<std::endl;
	gui_View = '\0';
	player_one = '\0';
	player_two = '\0';
	w_turn = true;    //white go firstly
	row_selected = -1;
	col_selected = -1;
	piece_selected = false;
	game_start = false;
	game_mode = option;
	move_choices = '\0';
	fileName = "";
	is_On = false;
}

GameController::~GameController() {
	ResetPlayer();

	//std::cout<<"GameController ~destructor "<<std::endl;
//	return;
}

void GameController::ResetPlayer() {
	if(player_one != NULL && player_two != NULL) {
		delete player_one;
		delete player_two;

		player_one = '\0';
		player_two = '\0';
	}
}

void GameController::SetView(ChessView* view) {

	gui_View = view;
}

void GameController::Setup_Mode(int option) {
	ResetPlayer();
	
	if (option == 1) {
		gui_View->SetStatusBar("Human V.S. Human");
		player_one = new HumanPlayer(WHITE, game);
		player_two = new HumanPlayer(BLACK, game);
	}
	else if (option == 2) {
		gui_View->SetStatusBar("Human V.S. PC");
		player_one = new HumanPlayer(WHITE, game);
		player_two = new ComputerPlayer(BLACK, game);
	} 
	else if (option == 3) {
		gui_View->SetStatusBar("PC V.S. Human");
		player_one = new ComputerPlayer(WHITE, game);
		player_two = new HumanPlayer(BLACK, game);
	}
	else {
		gui_View->SetStatusBar("PC V.S. PC");
		player_one = new ComputerPlayer(WHITE, game);
		player_two = new ComputerPlayer(BLACK, game);
	}

}	
	/**
	 * Indicate to the player that the user clicked on the given
	 * row and column with the mouse.
	 */
void GameController::on_CellSelected(int row, int col, int button) {
	//std::cout<<"press...."<<std::endl;
 
	if (game_start) {
		if (w_turn)
			HumanPlayer_Move(row, col, button, player_one);
		else
			HumanPlayer_Move(row, col, button, player_two);
	}

}
// Change it to HumanPlayer_Move(.. ) add the player type.... HumanPlayer* 
void GameController::HumanPlayer_Move(int row, int col, int button, ChessPlayer* player) {
	HumanPlayer * people = dynamic_cast<HumanPlayer* >(player);
	if (people != NULL) {
		if (move_choices != NULL)
			Undo_Highlight();

		people->on_CellSelected(row, col, button);

		if (people->isMyPiece()) {
			//std::cout<<"Light on...."<<std::endl;
			Light_On(row, col, people);
			piece_selected = true;
			row_selected = row;
			col_selected = col;		
		}
		else if (piece_selected) {
			Move_Or_Attack(row, col, people);
			//Undo_Highlight();
			piece_selected = false;  // unselect !!! only one move...
			row_selected = -1;
			col_selected = -1;
			move_choices = '\0'; //change back
		}
	}
}

void GameController::Move_Or_Attack(int row, int col, ChessPlayer* player) {
	HumanPlayer * people = dynamic_cast<HumanPlayer* >(player);
	if (people != NULL) {
		if (people->isValidMove(row, col)) {
			if (people->isTaken(row, col)) {
				gui_View->ClearPiece(row, col);
			}
			people->PlacePiece(row_selected, col_selected, row, col);  // updata game facade
			Update_GUI(row, col, people->Piece_Type(), people->Piece_Color());
			
			if (game.Check(row, col)) {
				if (game.Mate(row, col)) {
					gui_View->WriteMessageArea("Congratulation!!! CheckMate...");
					game_start = false;
				}
				else {
					if (w_turn)
						gui_View->WriteMessageArea("White say: In Check...\n");
					else
						gui_View->WriteMessageArea("Black say: In Check...\n");
				}
			}
			else if (game.Mate(row, col)){
					gui_View->WriteMessageArea("Stalemate....");
					game_start = false;
			}

			if (game_start)
				SwitchPlayer();
			//do not change black turn!!, until white piece move successfully
				}
	}
	else {
		ComputerPlayer * pc = dynamic_cast<ComputerPlayer* >(player);
		if (pc != NULL) {
			if (pc->isValidMove(row, col)) {
				if (pc->isTaken(row, col)) {
					gui_View->ClearPiece(row, col);
				}
				pc->PlacePiece(row_selected, col_selected, row, col);
				Update_GUI(row, col, pc->Piece_Type(), pc->Piece_Color());
		
				if (game.Check(row, col)) {
					if (game.Mate(row, col)) {
						gui_View->WriteMessageArea("Congratulation!!! CheckMate...");
						game_start = false;
					}
					else {
						if (w_turn)
							gui_View->WriteMessageArea("White say: In Check...\n");
						else
							gui_View->WriteMessageArea("Black say: In Check...\n");
					}
				}
				else if (game.Mate(row, col)){
						gui_View->WriteMessageArea("Stalemate....");
						game_start = false;
				}

				if (game_start)
					SwitchPlayer();
				//do not change black turn!!, until white piece move successfully
			}
		}
			
	}
		//Update_GUI(row, col, player->Piece_Type(), player->Piece_Color());

}

void GameController::SwitchPlayer() {
	if (w_turn) {
		w_turn = false;  
		gui_View->SetTopLabel("Black turn");
		gui_View->SetBottomLabel("");
	}
	else {
		w_turn = true;
		gui_View->SetTopLabel("");
		gui_View->SetBottomLabel("White turn");
	}
}

void GameController::Light_On(int row, int col, HumanPlayer* player) {
	if (piece_selected)
		Undo_Highlight();
	
	move_choices = &(player->GetValidMoves());
	Highlight_Cells(row, col);	

}

void GameController::Highlight_Cells(int row, int col) {

	gui_View->HighlightSquare(row, col, GREEN_SQUARE);

	for (std::set< BoardPosition > :: const_iterator iter = move_choices->begin();
		iter != move_choices->end(); ++iter) {

		int r = iter->GetRow();
		int c = iter->GetColumn();
		gui_View->HighlightSquare(r, c, YELLOW_SQUARE);
	}

}
void GameController::Undo_Highlight() {

	gui_View->UnHighlightSquare(row_selected, col_selected);
	
	for (std::set< BoardPosition > :: const_iterator iter = move_choices->begin();
		iter != move_choices->end(); ++iter) {
		
		int r = iter->GetRow();
		int c = iter->GetColumn();
		gui_View->UnHighlightSquare(r, c);
	}

}

void GameController::Update_GUI(int row, int col, PieceType type, PieceColor color) {
	
	gui_View->ClearPiece(row_selected, col_selected);

	ImageName piece_image;	
	if (color == WHITE) {
		piece_image = White_Piece(type);
	}
	else {
		piece_image = Black_Piece(type);
	}

	gui_View->PlacePiece(row, col, piece_image);
	
}

ImageName GameController::White_Piece(PieceType type) {
	std::string msg = "";
	ImageName image;
	if (type == PAWN) {
		image = W_PAWN;
	}
	else if(type == ROOK) {
		image = W_ROOK;
	}
	else if(type == BISHOP) {
		image = W_BISHOP;
	}
	else if(type == KNIGHT) {
		image = W_KNIGHT;
	}
	else if(type == QUEEN) {
		image = W_QUEEN;
	}
	else if(type == KING) {
		image = W_KING;
	}

	return image;
}

ImageName GameController::Black_Piece(PieceType type) {

	ImageName image;
	if (type == PAWN)
		image = B_PAWN;
	else if(type == ROOK)
		image = B_ROOK;
	else if(type == BISHOP)
		image = B_BISHOP;
	else if(type == KNIGHT)
		image = B_KNIGHT;
	else if(type == QUEEN)
		image = B_QUEEN;
	else if(type == KING)
		image = B_KING;

	return image;
}

	///@param row where drag began
	///@param col where drag began
void GameController::on_DragStart(int row,int col) {

}

	///@param row where drag ended
	///@param col where drag ended
	///@return true if the drag resulted in a successful drop
bool GameController::on_DragEnd(int row,int col) {
	return true;
}


	/**
	 * Handle when the user selected the new game button.
	 */
void GameController::on_NewGame() {
	gui_View->ClearMessageArea();
	gui_View->WriteMessageArea("******* New Game *******\n Ready\n Set\n Go!\n ");

	if (row_selected != -1 && col_selected != -1)	
		Undo_Highlight();

	Setup_Mode(game_mode);

	w_turn = true;    //white go firstly
	row_selected = -1;
	col_selected = -1;
	piece_selected = false;
	game_start = false;
	fileName = "";
	move_choices = '\0';
	is_On = false;
	game.NewGame();
	Clear_Board();
	place_BlackPiece();
	place_WhitePiece();
	
	game_start = true;
	gui_View->SetBottomLabel("White Start");
}

void GameController::Clear_Board() {

	for (int i=0; i<8; i++) {
		for (int j=0; j<8; j++) {
			gui_View->ClearPiece(i, j);
		}
	}

}

void GameController::place_BlackPiece() {

	gui_View->PlacePiece(0, 0, B_ROOK);
	gui_View->PlacePiece(0, 1, B_KNIGHT);
	gui_View->PlacePiece(0, 2, B_BISHOP);
	gui_View->PlacePiece(0, 3, B_QUEEN);
	gui_View->PlacePiece(0, 4, B_KING);
	gui_View->PlacePiece(0, 5, B_BISHOP);
	gui_View->PlacePiece(0, 6, B_KNIGHT);
	gui_View->PlacePiece(0, 7, B_ROOK);

	for (int j=0; j<8; j++) {
			
		gui_View->PlacePiece(1, j, B_PAWN);
	}

}

void GameController::place_WhitePiece() {

	for (int j=0; j<8; j++) {
			
		gui_View->PlacePiece(6, j, W_PAWN);
	}
	gui_View->PlacePiece(7, 0, W_ROOK);
	gui_View->PlacePiece(7, 1, W_KNIGHT);
	gui_View->PlacePiece(7, 2, W_BISHOP);
	gui_View->PlacePiece(7, 3, W_QUEEN);
	gui_View->PlacePiece(7, 4, W_KING);
	gui_View->PlacePiece(7, 5, W_BISHOP);
	gui_View->PlacePiece(7, 6, W_KNIGHT);
	gui_View->PlacePiece(7, 7, W_ROOK);

}

	/**
	 * Handle when the user selected the save game button.
	 */
void GameController::on_SaveGame() {
	if (fileName == "")
		on_SaveGameAs();
	else {
		gui_View->WriteMessageArea("SaveGame: " + fileName);
		game.SaveGame(fileName);
	}
}

	/**
	 * Handle when the user selected the save game as button.
	 */
void GameController::on_SaveGameAs() {
	fileName = gui_View->SelectSaveFile();
	if (fileName != "")
		on_SaveGame();
}

	/**
	 * Handle when the user selected the load game button.
	 */
void GameController::on_LoadGame() {

	std::string name = "";
	name = gui_View->SelectLoadFile();
	if (name == "")
		return;
	fileName = "file://";
	fileName += name;
	
	try
	{
/*
		std::ifstream file(fileName.c_str());

		if (!file.is_open()) {
			gui_View->WriteMessageArea("This does not exist!!!");
			throw FileException(std::string("could not open file ") + fileName);
		}
		file.close();
*/
		URLInputStream stream (fileName);
	    
		HTMLTokenizer token (&stream);	

		if (row_selected != -1 && col_selected != -1)	
		Undo_Highlight();

		Setup_Mode(game_mode);

		w_turn = true;    //white go firstly
		row_selected = -1;
		col_selected = -1;
		piece_selected = false;
		game_start = false;
		
		move_choices = '\0';
		game.NewGame();
		Clear_Board();
		place_BlackPiece();
		place_WhitePiece();
	
		game_start = true;

		Redo_Game(token);
	}
	catch (std::exception &e)
	{
		std::cout << "Exception Occurred 1:" << e.what() << std::endl;
		return;
	}
	catch (CS240Exception &e) 
	{
		std::cout << "Exception Occurred 2: " << e.GetMessage() << std::endl;
		
	}
	catch (...)
	{
		std::cout << "Unknown Exception Occurred" << std::endl;
		return;
	}			
}

void GameController::Redo_Game(HTMLTokenizer & token) {
	
	while (token.HasNextToken()) {
		HTMLToken curToken = token.GetNextToken();
		std::string tag_value = StringUtil::ToLowerCopy(curToken.GetValue());
		if (curToken.GetType() == TAG_START && tag_value == "history") {
			XmlReader(token);
			break;
		}
	}
}

void GameController::XmlReader(HTMLTokenizer & token) {

	std::string tag_value;		

	while (token.HasNextToken()) {
		HTMLToken curToken = token.GetNextToken();
		tag_value = StringUtil::ToLowerCopy(curToken.GetValue());

		if (curToken.GetType() == TAG_END && tag_value == "history")	
			break;
		
		if (curToken.GetType() == TAG_START && tag_value == "move") {
			Movements(token);
		}
	}

	if (tag_value != "history")
		throw IllegalStateException(string("Xml file is not ended by </history> tag"));
}

void GameController::Movements(HTMLTokenizer & token) {
	while (token.HasNextToken()) {
		HTMLToken curToken = token.GetNextToken();
		std::string tag_value = StringUtil::ToLowerCopy(curToken.GetValue());
		if (curToken.GetType() == TAG_END && tag_value == "move")
			break;
		
		if (curToken.GetType() == TAG_START && tag_value == "piece") {
			int col = atoi((curToken.GetAttribute("column")).c_str());
			int row = atoi((curToken.GetAttribute("row")).c_str());
			AutoPlay(row, col);
		}
	}
}

void GameController::AutoPlay(int row, int col){
	if (w_turn)
			HumanPlayer_Move(row, col, 1, player_one);
		else
			HumanPlayer_Move(row, col, 1, player_two);

}
	/**
	 * Handle when the user selected the undo move button.
	 */
void GameController::on_UndoMove() {
	PieceHistory * history = game.Undo();
	if (history != NULL) {
		
		int s_row = history->Get_S_Row();
		int s_col = history->Get_S_Column();
		int end_row = history->Get_E_Row();
		int end_col = history->Get_E_Column();
		Piece * m_piece = history->Get_Moving_Piece();		
		
		assert(m_piece != NULL);

		if (move_choices != NULL)
			Undo_Highlight();	
		// reverse order --- from the end to begining!
		row_selected = end_row;
		col_selected = end_col;

		Update_GUI(s_row, s_col, history->GetType_Moving(), history->GetColor_Moving());

		if (history->IsAttackPieceHere())
			Update_GUI(end_row, end_col, history->GetType_Attack(), history->GetColor_Attack());
		
		SwitchPlayer();
		row_selected = -1;
		col_selected = -1;
		if (w_turn)
			gui_View->WriteMessageArea("Undo -- Black Cheating!\n");
		else
			gui_View->WriteMessageArea("Undo -- White Cheating!\n");
	}
	game_start = true;
}
		
	/**
	 * Handle when the user selects to quit the game, either through the
	 * quit button, the close X button, or the file menu.
	 */
void GameController::on_QuitGame() {
	gui_View->WriteMessageArea("Quit\n");
	game.Quit();

}
		
	/**
	 * Handle when a timer event has been signaled.
	 */
void GameController::on_TimerEvent() {
	
	if (game_start) {
		if (w_turn) {
			ComputerPlayer_Move(player_one);
			std::cout<<"hello world..."<<std::endl;
		}
		else {
			ComputerPlayer_Move(player_two);
			std::cout<<"done..."<<std::endl;
		}
	}
}
	
void GameController::ComputerPlayer_Move(ChessPlayer* player) {
	ComputerPlayer * pc = dynamic_cast<ComputerPlayer* >(player);
	if (pc != NULL && !is_On) {
		is_On = true;
		pc->on_TimerEvent();
		move_choices = &(pc->GetValidMoves());

		BoardPosition bp;
		for (std::set< BoardPosition > :: const_iterator iter = move_choices->begin();
		iter != move_choices->end(); ++iter) {
			bp = *iter;
			int row = bp.GetRow();
			int col = bp.GetColumn();
			std::cout<<"( "<<row<<" , "<<col<<" )"<<std::endl;
			break;
		}
		int row = bp.GetRow();
		int col = bp.GetColumn();
	
		row_selected = pc->GetRow();
		col_selected = pc->GetColumn();
		
		Move_Or_Attack(row, col, player);
		row_selected = -1;
		col_selected = -1;
		move_choices = '\0'; //change back
		is_On = false;
	}	
	
}



