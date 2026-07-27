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
#include "GameState.h"

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
	std::set<BoardPosition> LegalMoves(int row, int col) const;
	bool isCellTaken(int row, int col) const;
	bool isValidMove(int row, int col) const;

	void MovePiece(
		int source_row,
		int source_col,
		int destination_row,
		int destination_col,
		std::optional<PieceType> promotion = std::nullopt);
	const PieceHistory * Undo();
	void ClearHistory() noexcept;
	void ClearUndo() noexcept;
	void Clear_Board() noexcept;

	bool IsInCheck(PieceColor color) const;
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
	PieceColor Turn() const noexcept;
	GameStatus Status() const noexcept;
	bool IsGameOver() const noexcept;
	const CastlingRights & Castling() const noexcept;
	const std::optional<BoardPosition> & EnPassantTarget() const noexcept;
	unsigned HalfmoveClock() const noexcept;
	unsigned FullmoveNumber() const noexcept;
	const std::string & CurrentFile() const noexcept;
	SavedGameState ExportState() const;
	void ImportState(SavedGameState state);

	// A validated setup hook for core tests and non-GUI front ends.
	void SetPosition(
		std::vector<PieceSnapshot> pieces,
		PieceColor turn,
		CastlingRights castling = {},
		std::optional<BoardPosition> en_passant = std::nullopt,
		unsigned halfmove_clock = 0,
		unsigned fullmove_number = 1);

	static bool Test(std::ostream & os);

private:
	std::unique_ptr<ChessBoard> board_;
	Piece * current_piece_ = nullptr;
	std::set<BoardPosition> valid_moves_;
	std::vector<PieceHistory> move_history_;
	std::optional<PieceHistory> undo_once_;
	PieceColor turn_ = WHITE;
	GameStatus status_ = GameStatus::Ongoing;
	CastlingRights castling_{};
	std::optional<BoardPosition> en_passant_;
	unsigned halfmove_clock_ = 0;
	unsigned fullmove_number_ = 1;
	std::vector<std::string> position_keys_;
	std::string file_name_;

	void LookForMoves();
	std::set<BoardPosition> LegalMovesFor(
		const ChessBoard & board,
		int row,
		int col,
		PieceColor side,
		const CastlingRights & castling,
		const std::optional<BoardPosition> & en_passant) const;
	std::set<BoardPosition> PseudoMovesFor(
		const ChessBoard & board,
		const Piece & piece,
		const CastlingRights & castling,
		const std::optional<BoardPosition> & en_passant) const;
	bool IsSquareAttacked(
		const ChessBoard & board,
		int row,
		int col,
		PieceColor by_color) const;
	bool IsInCheck(const ChessBoard & board, PieceColor color) const;
	bool HasLegalMove(PieceColor color) const;
	void UpdateStatus();
	void UpdateCastlingRights(
		const PieceSnapshot & moving,
		int source_row,
		int source_col,
		const std::optional<PieceSnapshot> & captured);
	std::string PositionKey() const;
	bool IsInsufficientMaterial() const;
	void RequireBoard() const;
	static bool IsTerminal(GameStatus status) noexcept;
};

#endif
