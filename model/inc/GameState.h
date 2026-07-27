#ifndef GAME_STATE_H
#define GAME_STATE_H

#include <optional>
#include <string>
#include <vector>

#include "BoardPosition.h"
#include "PieceHistory.h"

enum class GameStatus {
	Ongoing,
	Check,
	Checkmate,
	Stalemate,
	DrawThreefold,
	DrawFiftyMove,
	DrawInsufficientMaterial,
};

struct SavedGameState
{
	unsigned version = 2;
	std::vector<PieceSnapshot> pieces;
	PieceColor turn = WHITE;
	CastlingRights castling{};
	std::optional<BoardPosition> en_passant;
	unsigned halfmove_clock = 0;
	unsigned fullmove_number = 1;
	std::vector<PieceHistory> history;
	std::vector<std::string> position_keys;
};

#endif
