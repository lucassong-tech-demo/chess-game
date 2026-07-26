#ifndef GAME_FACADE_H
#define GAME_FACADE_H

#include <iostream>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "BoardPosition.h"
#include "ChessBoard.h"
#include "PieceHistory.h"

class GameFacade
{
public:
	GameFacade();
	~GameFacade() = default;

	GameFacade(const GameFacade &) = delete;
	GameFacade & operator=(const GameFacade &) = delete;
	GameFacade(GameFacade &&) noexcept = default;
	GameFacade & operator=(GameFacade &&) noexcept = default;

	void NewGame();
	Piece * GetPiece(int row, int col, PieceColor color);
	std::set<BoardPosition> & GetValidMoves();
	bool isCellTaken(int row, int col) const;
	bool isValidMove(int row, int col) const;

	void MovePiece(int source_row, int source_col, int destination_row, int destination_col);
	const PieceHistory * Undo();
	void ClearHistory() noexcept;
	void ClearUndo() noexcept;
	void Clear_Board() noexcept;

	bool Check(int row, int col);
	bool Mate(int row, int col);
	void Quit();

	void SaveGame(const std::string & file_name);
	void SaveAs(const std::string & file_name);
	bool LoadGame(const std::string & file_name);
	void UpdateMoveHistory(std::vector<PieceHistory> & moves);
	void ReadMoveHistory();
	void Clean_History();
	void Reset_Chess_Board();

	std::size_t HistorySize() const noexcept;
	const ChessBoard & Board() const;

	static bool Test(std::ostream & os);

private:
	std::unique_ptr<ChessBoard> board_;
	Piece * current_piece_ = nullptr; // observer; ChessBoard owns the piece
	std::set<BoardPosition> valid_moves_;
	std::vector<PieceHistory> move_history_;
	std::optional<PieceHistory> undo_once_;
	std::string file_name_;

	void LookForMoves();
	void FilterMoves(const std::set<BoardPosition> & moves);
	std::set<BoardPosition> BoardCheck(
		int source_row,
		int source_col,
		int destination_row,
		int destination_col);
	void RequireBoard() const;
};

#endif
