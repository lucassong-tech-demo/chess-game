#include "ChessBoard.h"


ChessBoard::ChessBoard() {
	
	w_King = '\0';
	b_King = '\0';
	allMoves = '\0';
	memset(board, 0, sizeof(Piece));
	
	Init_Board();
	Init_White_Piece();
	Init_Black_Piece();

}


ChessBoard::~ChessBoard() {

	ClearBoard();

	w_King = '\0';
	b_King = '\0';

	if (allMoves != NULL)
		delete allMoves;	
}

void ChessBoard::ClearBoard() {

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {

			if (board[i][j] != NULL) {

				delete board[i][j];
				board[i][j] = '\0';
			}
		}
	}
}

void ChessBoard::Init_Board() {

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			board[i][j] = '\0';
		}
	}
}

void ChessBoard::Init_White_Piece() {
	board[6][0] = new Pawn(PAWN, WHITE, 6, 0); 
	board[6][1] = new Pawn(PAWN, WHITE, 6, 1); 
	board[6][2] = new Pawn(PAWN, WHITE, 6, 2); 
	board[6][3] = new Pawn(PAWN, WHITE, 6, 3); 
	board[6][4] = new Pawn(PAWN, WHITE, 6, 4); 
	board[6][5] = new Pawn(PAWN, WHITE, 6, 5); 
	board[6][6] = new Pawn(PAWN, WHITE, 6, 6); 
	board[6][7] = new Pawn(PAWN, WHITE, 6, 7); 
	board[7][0] = new Rook(ROOK, WHITE, 7, 0); 
	board[7][1] = new Knight(KNIGHT, WHITE, 7, 1); 
	board[7][2] = new Bishop(BISHOP, WHITE, 7, 2); 
	board[7][3] = new Queen(QUEEN, WHITE, 7, 3); 
	board[7][4] = new King(KING, WHITE, 7, 4); 
	board[7][5] = new Bishop(BISHOP, WHITE, 7, 5); 
	board[7][6] = new Knight(KNIGHT, WHITE, 7, 6); 
	board[7][7] = new Rook(ROOK, WHITE, 7, 7); 
	w_King = board[7][4];
}

void ChessBoard::Init_Black_Piece() {

	board[0][0] = new Rook(ROOK, BLACK, 0, 0); 
	board[0][1] = new Knight(KNIGHT, BLACK, 0, 1); 
	board[0][2] = new Bishop(BISHOP, BLACK, 0, 2); 
	board[0][3] = new Queen(QUEEN, BLACK, 0, 3); 
	board[0][4] = new King(KING, BLACK, 0, 4); 
	board[0][5] = new Bishop(BISHOP, BLACK, 0, 5); 
	board[0][6] = new Knight(KNIGHT, BLACK, 0, 6); 
	board[0][7] = new Rook(ROOK, BLACK, 0, 7); 
	board[1][0] = new Pawn(PAWN, BLACK, 1, 0); 
	board[1][1] = new Pawn(PAWN, BLACK, 1, 1); 
	board[1][2] = new Pawn(PAWN, BLACK, 1, 2); 
	board[1][3] = new Pawn(PAWN, BLACK, 1, 3); 
	board[1][4] = new Pawn(PAWN, BLACK, 1, 4); 
	board[1][5] = new Pawn(PAWN, BLACK, 1, 5); 
	board[1][6] = new Pawn(PAWN, BLACK, 1, 6); 
	board[1][7] = new Pawn(PAWN, BLACK, 1, 7); 
	b_King = board[0][4];
}

Piece* ChessBoard::GetPiece(int row, int col) const {

	return board[row][col];
}

void ChessBoard::MovePiece(int r_selected, int c_selected, int row, int col) {

	board[row][col] = board[r_selected][c_selected];
 	board[r_selected][c_selected] = '\0';
	(board[row][col])->Update_location(row, col);
}

void ChessBoard::Reset_Piece (const int s_row, const int s_col, const int end_r, const int end_col) {

	board[s_row][s_col] = board[end_r][end_col];
	board[end_r][end_col] = '\0';
	(board[s_row][s_col])->Update_location(s_row, s_col);
}

void ChessBoard::PutBack_Piece(Piece* piece, const int row, const int col) {
	
	assert(piece != NULL);
	board[row][col] = piece;
}

Piece * ChessBoard::GetKing(PieceColor color) {
	
	if (color == WHITE)
		return Get_W_King();
	else
		return Get_B_King();
}



Piece * ChessBoard::Get_W_King() {
	return w_King;
}

Piece * ChessBoard::Get_B_King() {
	return b_King;
}

bool ChessBoard::Test(std::ostream & os) {
	bool success = true;
	ChessBoard cb;

	Piece* p = cb.GetPiece(1,0);
	TEST(p->GetType() == PAWN);
	if (success) {
		TEST(p->GetColor() == BLACK);
		if (success)
			std::cout<<"\tPiece testing success"<<std::endl;
	}
	else
		std::cout<<"\tPiece testing -- fail"<<std::endl;
	
	p = cb.GetPiece(2,1);
	TEST(p == NULL);
	if (success)
		std::cout<<"\tChess board testing success"<<std::endl;
	else
		std::cout<<"\tChess board testing -- failed!"<<std::endl;

//************************************

	cb.MovePiece(1, 0, 2, 1);  //move pawn 
	Piece* p1 = cb.GetPiece(1,0);
	Piece* q1 = cb.GetPiece(2,1);
	TEST(p1 == NULL);
	if (success) {
		TEST(q1 != NULL);
		if (success) {
			TEST(q1->GetType() == PAWN);
			if (success)
				std::cout<<"\tMove Piece testing success"<<std::endl;
			else
				std::cout<<"\tMove Piece testing failed!!!"<<std::endl;
		}
		else
			std::cout<<"\tMove Piece testing failed!!!"<<std::endl;
	}
	else 
		std::cout<<"\tMove Piece testing failed!!!"<<std::endl;

	//****************************
	cb.MovePiece(0,1,2,1);   // knight -> Pawn
	Piece* q2 = cb.GetPiece(2,1);
	TEST(q2->GetType() == KNIGHT);
	if(success) {
		Knight* k = dynamic_cast<Knight*>(q2);
		TEST(k!=NULL);
		if (success)
			std::cout<<"\tAttack Piece testing success."<<std::endl;
		else
			std::cout<<"\tAttack Piece testing failed!!!"<<std::endl;
		}
	else
		std::cout<<"\tAttack Piece testing failed!!!"<<std::endl;
		
//*************************************
/*	cb.MovePiece(6, 0, 2, 1); // w_Pawn -> b_Knight
	TEST((cb.off_pieces).size() == 2);
	if (success) {
		Piece* p3 = (cb.off_pieces).top();
		TEST(p3->GetType() == KNIGHT);
		if (success) {
			TEST(p3->GetColor() == BLACK);
			if (success)
				std::cout<<"\toff pieces stack testing success."<<std::endl;
			else
				std::cout<<"\toff pieces stack testing failed!!!"<<std::endl;
		}
		else
			std::cout<<"\toff pieces stack testing failed!!!"<<std::endl;
	}	
	else
		std::cout<<"\toff pieces stack testing failed!!!"<<std::endl;
		
*/

//************************************	
/*	cb.ClearPieceStack();
	TEST(cb.Piece_Stack_Empty());
	if(success)
		std::cout<<"\tClear Piece Stack success."<<std::endl;
		
	else
		std::cout<<"\tClear Piece Stack failed!!!"<<std::endl;
*/

	cb.ClearBoard();
	q1 = cb.GetPiece(2,1);
	TEST(q1 == NULL);
	if (success)
		std::cout<<"\tClear board testing success."<<std::endl;
	else
		std::cout<<"\tClear board testing failed!!!!"<<std::endl;
		

	return success;
}


