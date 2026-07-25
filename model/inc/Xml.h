#ifndef XML_H
#define XML_H

#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "ChessBoard.h"
#include "PieceHistory.h"
#include "Piece.h"

/**
 *  Xml class is a xml file generater as well as xml reader.
 *  It interprets all movement history and current state of chess board
 *  into a xml file. It also can read the xml file and reset the board
 *  and the history stack.
 */
class Xml
{
private:
	std::string file_name;

public:
	Xml(const std::string & filename): file_name(filename){}
	~Xml(){}

	/**
	 * Save current state of chess board and move history.
	 * open the file, and pass the file descriptor to writehistory function and 
	 * writeBoard() function.
	 * @param history a copy of history stack.
	 * @param board a pointer which points to the current chess board.
	 * @throw if the file cannot be open successfully, throw exception.
	 */
	void WriteIntoFile(ChessBoard * board, std::vector<PieceHistory* > & history);

	/**
	 * Read the history stack. write all data into the .xml file.
	 * @param file the opened file
	 * @param history a reference of stack history
	 */
	void WriteHistory(std::ofstream & file, vector<PieceHistory *> & history);

	/**
	 * Read the chess board, get each piece and learn the type, color, and location. Write them into file
	 * @param file the opened .xml file.
	 * @param board a pointer points to the chess board, using it to get each piece.
	 */
	void WriteBoard(std::ofstream & file, ChessBoard * board);
	
	/**
	 * read the xml file and rebuild the board and history stack.
	 * @param history a reference of empty history stack
	 * @param board a pointer for rebuild the chess board
	 * @throw if the file cannot be open successfully, throw exception.
	 */
	//void ReadFromFile(stack< PieceLocation, vector<PieceLocation> > & history, ChessBoard * board);

	// rebuild the chess board
	//void ReadBoardValue(std::ifstream & file, ChessBoard * board);

	// reload the history stack
	//void ReadHistoryValue(std::ifstream & file, vector<PieceLocation> > & history);

		


};

#endif


