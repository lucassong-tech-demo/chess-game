#include "Xml.h"

#include <cctype>
#include <charconv>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace {

struct Node
{
	std::string name;
	std::map<std::string, std::string> attributes;
	std::vector<Node> children;
};

class Parser
{
public:
	explicit Parser(std::string_view input) : input_(input) {}

	Node ParseDocument()
	{
		SkipWhitespace();
		if (StartsWith("<?xml")) {
			const auto end = input_.find("?>", position_);
			if (end == std::string_view::npos) {
				Fail("unterminated XML declaration");
			}
			position_ = end + 2;
		}
		SkipMisc();
		Node root = ParseNode();
		SkipMisc();
		if (position_ != input_.size()) {
			Fail("unexpected content after root element");
		}
		return root;
	}

private:
	std::string_view input_;
	std::size_t position_ = 0;

	[[noreturn]] void Fail(const std::string & message) const
	{
		throw std::invalid_argument(
			"invalid XML near byte " + std::to_string(position_) + ": " + message);
	}

	bool StartsWith(std::string_view text) const
	{
		return input_.substr(position_, text.size()) == text;
	}

	void SkipWhitespace()
	{
		while (position_ < input_.size()
			&& std::isspace(static_cast<unsigned char>(input_[position_]))) {
			++position_;
		}
	}

	void SkipMisc()
	{
		for (;;) {
			SkipWhitespace();
			if (!StartsWith("<!--")) {
				return;
			}
			const auto end = input_.find("-->", position_ + 4);
			if (end == std::string_view::npos) {
				Fail("unterminated comment");
			}
			position_ = end + 3;
		}
	}

	std::string ParseName()
	{
		const std::size_t start = position_;
		while (position_ < input_.size()) {
			const unsigned char ch = static_cast<unsigned char>(input_[position_]);
			if (!(std::isalnum(ch) || ch == '_' || ch == '-' || ch == ':')) {
				break;
			}
			++position_;
		}
		if (start == position_) {
			Fail("expected an element or attribute name");
		}
		return std::string(input_.substr(start, position_ - start));
	}

	std::string ParseAttributeValue()
	{
		if (position_ >= input_.size()
			|| (input_[position_] != '"' && input_[position_] != '\'')) {
			Fail("expected quoted attribute value");
		}
		const char quote = input_[position_++];
		std::string value;
		while (position_ < input_.size() && input_[position_] != quote) {
			if (input_[position_] != '&') {
				value.push_back(input_[position_++]);
				continue;
			}
			const auto end = input_.find(';', position_ + 1);
			if (end == std::string_view::npos) {
				Fail("unterminated entity");
			}
			const std::string_view entity =
				input_.substr(position_ + 1, end - position_ - 1);
			if (entity == "amp") value.push_back('&');
			else if (entity == "lt") value.push_back('<');
			else if (entity == "gt") value.push_back('>');
			else if (entity == "quot") value.push_back('"');
			else if (entity == "apos") value.push_back('\'');
			else Fail("unsupported entity");
			position_ = end + 1;
		}
		if (position_ >= input_.size()) {
			Fail("unterminated attribute value");
		}
		++position_;
		return value;
	}

	Node ParseNode()
	{
		if (position_ >= input_.size() || input_[position_] != '<'
			|| StartsWith("</")) {
			Fail("expected opening element");
		}
		++position_;
		Node node;
		node.name = ParseName();
		for (;;) {
			SkipWhitespace();
			if (StartsWith("/>")) {
				position_ += 2;
				return node;
			}
			if (position_ < input_.size() && input_[position_] == '>') {
				++position_;
				break;
			}
			const std::string key = ParseName();
			SkipWhitespace();
			if (position_ >= input_.size() || input_[position_] != '=') {
				Fail("expected '=' after attribute name");
			}
			++position_;
			SkipWhitespace();
			if (!node.attributes.emplace(key, ParseAttributeValue()).second) {
				Fail("duplicate attribute");
			}
		}

		for (;;) {
			SkipMisc();
			if (StartsWith("</")) {
				position_ += 2;
				const std::string closing = ParseName();
				SkipWhitespace();
				if (position_ >= input_.size() || input_[position_] != '>') {
					Fail("expected closing '>'");
				}
				++position_;
				if (closing != node.name) {
					Fail("mismatched closing element");
				}
				return node;
			}
			if (position_ >= input_.size()) {
				Fail("unterminated element");
			}
			if (input_[position_] == '<') {
				node.children.push_back(ParseNode());
				continue;
			}
			const std::size_t text_start = position_;
			while (position_ < input_.size() && input_[position_] != '<') {
				if (!std::isspace(static_cast<unsigned char>(input_[position_]))) {
					Fail("text content is not supported");
				}
				++position_;
			}
			if (text_start == position_ && position_ >= input_.size()) {
				Fail("unterminated element");
			}
		}
	}
};

const Node & RequireChild(const Node & node, const std::string & name)
{
	const Node * found = nullptr;
	for (const Node & child : node.children) {
		if (child.name == name) {
			if (found) {
				throw std::invalid_argument("duplicate <" + name + "> element");
			}
			found = &child;
		}
	}
	if (!found) {
		throw std::invalid_argument("missing <" + name + "> element");
	}
	return *found;
}

const std::string & RequireAttribute(const Node & node, const std::string & name)
{
	const auto it = node.attributes.find(name);
	if (it == node.attributes.end()) {
		throw std::invalid_argument(
			"missing '" + name + "' attribute on <" + node.name + ">");
	}
	return it->second;
}

std::optional<std::string> Attribute(const Node & node, const std::string & name)
{
	const auto it = node.attributes.find(name);
	return it == node.attributes.end()
		? std::nullopt : std::optional<std::string>(it->second);
}

unsigned ParseUnsigned(const std::string & value, const std::string & field)
{
	unsigned result = 0;
	const auto [end, error] =
		std::from_chars(value.data(), value.data() + value.size(), result);
	if (error != std::errc{} || end != value.data() + value.size()) {
		throw std::invalid_argument("invalid unsigned value for " + field);
	}
	return result;
}

int ParseCoordinate(const std::string & value, const std::string & field)
{
	int result = 0;
	const auto [end, error] =
		std::from_chars(value.data(), value.data() + value.size(), result);
	if (error != std::errc{} || end != value.data() + value.size()
		|| result < 0 || result >= 8) {
		throw std::invalid_argument("invalid board coordinate for " + field);
	}
	return result;
}

bool ParseBool(const std::string & value, const std::string & field)
{
	if (value == "true" || value == "1") return true;
	if (value == "false" || value == "0") return false;
	throw std::invalid_argument("invalid boolean value for " + field);
}

PieceType ParseType(const std::string & value)
{
	if (value == "king") return KING;
	if (value == "queen") return QUEEN;
	if (value == "knight") return KNIGHT;
	if (value == "bishop") return BISHOP;
	if (value == "rook") return ROOK;
	if (value == "pawn") return PAWN;
	throw std::invalid_argument("unknown chess piece type: " + value);
}

PieceColor ParseColor(const std::string & value)
{
	if (value == "white") return WHITE;
	if (value == "black") return BLACK;
	throw std::invalid_argument("unknown chess piece color: " + value);
}

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

std::string KindString(MoveKind kind)
{
	switch (kind) {
	case MoveKind::Normal: return "normal";
	case MoveKind::Castling: return "castling";
	case MoveKind::EnPassant: return "en-passant";
	case MoveKind::Promotion: return "promotion";
	}
	throw std::logic_error("unknown move kind");
}

MoveKind ParseKind(const std::string & value)
{
	if (value == "normal") return MoveKind::Normal;
	if (value == "castling") return MoveKind::Castling;
	if (value == "en-passant") return MoveKind::EnPassant;
	if (value == "promotion") return MoveKind::Promotion;
	throw std::invalid_argument("unknown move kind: " + value);
}

std::string Escape(std::string_view value)
{
	std::string escaped;
	for (const char ch : value) {
		switch (ch) {
		case '&': escaped += "&amp;"; break;
		case '<': escaped += "&lt;"; break;
		case '>': escaped += "&gt;"; break;
		case '"': escaped += "&quot;"; break;
		case '\'': escaped += "&apos;"; break;
		default: escaped.push_back(ch); break;
		}
	}
	return escaped;
}

void WritePiece(std::ostream & file, const PieceSnapshot & piece, int indent)
{
	file << std::string(static_cast<std::size_t>(indent), ' ')
		<< "<piece type=\"" << TypeString(piece.type)
		<< "\" color=\"" << ColorString(piece.color)
		<< "\" row=\"" << piece.row
		<< "\" column=\"" << piece.column << "\"/>\n";
}

PieceSnapshot ReadPiece(const Node & node)
{
	if (node.name != "piece") {
		throw std::invalid_argument("expected <piece> element");
	}
	return {
		ParseType(RequireAttribute(node, "type")),
		ParseColor(RequireAttribute(node, "color")),
		ParseCoordinate(RequireAttribute(node, "row"), "piece row"),
		ParseCoordinate(RequireAttribute(node, "column"), "piece column"),
	};
}

CastlingRights ReadCastling(const Node & node)
{
	return {
		ParseBool(RequireAttribute(node, "whiteKingSide"), "whiteKingSide"),
		ParseBool(RequireAttribute(node, "whiteQueenSide"), "whiteQueenSide"),
		ParseBool(RequireAttribute(node, "blackKingSide"), "blackKingSide"),
		ParseBool(RequireAttribute(node, "blackQueenSide"), "blackQueenSide"),
	};
}

void WriteCastling(
	std::ostream & file,
	const std::string & element,
	const CastlingRights & rights,
	int indent)
{
	file << std::string(static_cast<std::size_t>(indent), ' ')
		<< '<' << element
		<< " whiteKingSide=\"" << (rights.white_king_side ? "true" : "false")
		<< "\" whiteQueenSide=\"" << (rights.white_queen_side ? "true" : "false")
		<< "\" blackKingSide=\"" << (rights.black_king_side ? "true" : "false")
		<< "\" blackQueenSide=\"" << (rights.black_queen_side ? "true" : "false")
		<< "\"/>\n";
}

std::string NormalizeLegacyPrefixes(std::string text)
{
	bool line_start = true;
	for (std::size_t index = 0; index + 1 < text.size(); ++index) {
		if (line_start) {
			std::size_t cursor = index;
			while (cursor < text.size()
				&& (text[cursor] == ' ' || text[cursor] == '\t')) {
				++cursor;
			}
			if (cursor + 1 < text.size()
				&& text[cursor] == '-' && text[cursor + 1] == '<') {
				text.erase(cursor, 1);
			}
			line_start = false;
		}
		if (text[index] == '\n') {
			line_start = true;
		}
	}
	return text;
}

SavedGameState ReadLegacy(const Node & root)
{
	SavedGameState state;
	state.version = 1;
	state.castling = {false, false, false, false};
	for (const Node & child : RequireChild(root, "board").children) {
		state.pieces.push_back(ReadPiece(child));
	}

	const Node & history = RequireChild(root, "history");
	PieceColor expected_turn = WHITE;
	unsigned fullmove = 1;
	for (const Node & move : history.children) {
		if (move.name != "move" || (move.children.size() != 2
			&& move.children.size() != 3)) {
			throw std::invalid_argument("invalid legacy move history");
		}
		const PieceSnapshot start = ReadPiece(move.children[0]);
		const PieceSnapshot end = ReadPiece(move.children[1]);
		if (start.type != end.type || start.color != end.color) {
			throw std::invalid_argument("legacy move endpoints disagree");
		}
		std::optional<PieceSnapshot> captured;
		if (move.children.size() == 3) {
			captured = ReadPiece(move.children[2]);
		}
		state.history.emplace_back(
			start,
			start.row,
			start.column,
			end.row,
			end.column,
			captured,
			MoveKind::Normal,
			std::nullopt,
			std::nullopt,
			std::nullopt,
			CastlingRights{false, false, false, false},
			std::nullopt,
			0,
			fullmove,
			expected_turn);
		if (expected_turn == BLACK) {
			++fullmove;
		}
		expected_turn = expected_turn == WHITE ? BLACK : WHITE;
	}
	state.turn = expected_turn;
	state.fullmove_number = fullmove;
	return state;
}

} // namespace

Xml::Xml(std::string filename) : file_name_(std::move(filename)) {}

void Xml::WriteIntoFile(const SavedGameState & state) const
{
	const std::filesystem::path target(file_name_);
	const std::filesystem::path temporary =
		target.string() + ".tmp-"
		+ std::to_string(
			std::chrono::steady_clock::now().time_since_epoch().count());
	std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
	if (!file.is_open()) {
		throw std::runtime_error("unable to open save file: " + file_name_);
	}
	file.exceptions(std::ios::badbit | std::ios::failbit);
	try {
		file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
			<< "<chessgame version=\"2\">\n"
			<< "  <state turn=\"" << ColorString(state.turn)
			<< "\" halfmove=\"" << state.halfmove_clock
			<< "\" fullmove=\"" << state.fullmove_number << "\">\n";
		WriteCastling(file, "castling", state.castling, 4);
		if (state.en_passant) {
			file << "    <enPassant row=\"" << state.en_passant->GetRow()
				<< "\" column=\"" << state.en_passant->GetColumn() << "\"/>\n";
		}
		file << "  </state>\n  <board>\n";
		for (const PieceSnapshot & piece : state.pieces) {
			WritePiece(file, piece, 4);
		}
		file << "  </board>\n  <history>\n";
		for (const PieceHistory & move : state.history) {
			file << "    <move kind=\"" << KindString(move.Kind())
				<< "\" startRow=\"" << move.Get_S_Row()
				<< "\" startColumn=\"" << move.Get_S_Column()
				<< "\" endRow=\"" << move.Get_E_Row()
				<< "\" endColumn=\"" << move.Get_E_Column()
				<< "\" turnBefore=\"" << ColorString(move.TurnBefore())
				<< "\" halfmoveBefore=\"" << move.HalfmoveBefore()
				<< "\" fullmoveBefore=\"" << move.FullmoveBefore() << '"';
			if (move.Promotion()) {
				file << " promotion=\"" << TypeString(*move.Promotion()) << '"';
			}
			file << ">\n";
			WritePiece(file, move.GetMovingSnapshot(), 6);
			if (move.GetAttackSnapshot()) {
				file << "      <captured>\n";
				WritePiece(file, *move.GetAttackSnapshot(), 8);
				file << "      </captured>\n";
			}
			WriteCastling(file, "castlingBefore", move.CastlingBefore(), 6);
			if (move.EnPassantBefore()) {
				file << "      <enPassantBefore row=\""
					<< move.EnPassantBefore()->GetRow() << "\" column=\""
					<< move.EnPassantBefore()->GetColumn() << "\"/>\n";
			}
			if (move.RookStart()) {
				file << "      <rook startRow=\"" << move.RookStart()->GetRow()
					<< "\" startColumn=\"" << move.RookStart()->GetColumn()
					<< "\" endRow=\"" << move.RookEnd()->GetRow()
					<< "\" endColumn=\"" << move.RookEnd()->GetColumn()
					<< "\"/>\n";
			}
			file << "    </move>\n";
		}
		file << "  </history>\n  <repetition>\n";
		for (const std::string & key : state.position_keys) {
			file << "    <position key=\"" << Escape(key) << "\"/>\n";
		}
		file << "  </repetition>\n</chessgame>\n";
		file.flush();
		file.close();
		std::error_code rename_error;
		std::filesystem::rename(temporary, target, rename_error);
		if (rename_error) {
			std::error_code cleanup_error;
			std::filesystem::remove(temporary, cleanup_error);
			throw std::runtime_error(
				"unable to replace save file: " + file_name_
				+ ": " + rename_error.message());
		}
	} catch (const std::ios_base::failure &) {
		std::error_code cleanup_error;
		std::filesystem::remove(temporary, cleanup_error);
		throw std::runtime_error("failed while writing save file: " + file_name_);
	} catch (...) {
		std::error_code cleanup_error;
		std::filesystem::remove(temporary, cleanup_error);
		throw;
	}
}

SavedGameState Xml::ReadFromFile() const
{
	std::ifstream file(file_name_, std::ios::binary);
	if (!file.is_open()) {
		throw std::runtime_error("unable to open save file: " + file_name_);
	}
	std::ostringstream buffer;
	buffer << file.rdbuf();
	if (file.bad()) {
		throw std::runtime_error("failed while reading save file: " + file_name_);
	}
	std::string text = NormalizeLegacyPrefixes(buffer.str());
	if (text.empty()) {
		throw std::invalid_argument("save file is empty");
	}
	const Node root = Parser(text).ParseDocument();
	if (root.name != "chessgame") {
		throw std::invalid_argument("save root must be <chessgame>");
	}
	const auto version_attribute = Attribute(root, "version");
	if (!version_attribute) {
		return ReadLegacy(root);
	}
	const unsigned version = ParseUnsigned(*version_attribute, "version");
	if (version != 2) {
		throw std::invalid_argument(
			"unsupported chess save version: " + std::to_string(version));
	}

	SavedGameState state;
	state.version = 2;
	const Node & state_node = RequireChild(root, "state");
	state.turn = ParseColor(RequireAttribute(state_node, "turn"));
	state.halfmove_clock =
		ParseUnsigned(RequireAttribute(state_node, "halfmove"), "halfmove");
	state.fullmove_number =
		ParseUnsigned(RequireAttribute(state_node, "fullmove"), "fullmove");
	state.castling = ReadCastling(RequireChild(state_node, "castling"));
	for (const Node & child : state_node.children) {
		if (child.name == "enPassant") {
			if (state.en_passant) {
				throw std::invalid_argument("duplicate <enPassant>");
			}
			state.en_passant.emplace(
				ParseCoordinate(RequireAttribute(child, "row"), "en passant row"),
				ParseCoordinate(
					RequireAttribute(child, "column"), "en passant column"));
		} else if (child.name != "castling") {
			throw std::invalid_argument("unknown state element: " + child.name);
		}
	}

	for (const Node & child : RequireChild(root, "board").children) {
		state.pieces.push_back(ReadPiece(child));
	}
	for (const Node & move : RequireChild(root, "history").children) {
		if (move.name != "move") {
			throw std::invalid_argument("unknown history element: " + move.name);
		}
		std::optional<PieceSnapshot> moving;
		std::optional<PieceSnapshot> captured;
		std::optional<BoardPosition> en_passant_before;
		std::optional<BoardPosition> rook_start;
		std::optional<BoardPosition> rook_end;
		std::optional<CastlingRights> castling_before;
		for (const Node & child : move.children) {
			if (child.name == "piece") {
				if (moving) throw std::invalid_argument("duplicate moving piece");
				moving = ReadPiece(child);
			} else if (child.name == "captured") {
				if (captured || child.children.size() != 1) {
					throw std::invalid_argument("invalid <captured> element");
				}
				captured = ReadPiece(child.children.front());
			} else if (child.name == "castlingBefore") {
				if (castling_before) {
					throw std::invalid_argument("duplicate <castlingBefore>");
				}
				castling_before = ReadCastling(child);
			} else if (child.name == "enPassantBefore") {
				if (en_passant_before) {
					throw std::invalid_argument("duplicate <enPassantBefore>");
				}
				en_passant_before.emplace(
					ParseCoordinate(RequireAttribute(child, "row"), "history ep row"),
					ParseCoordinate(
						RequireAttribute(child, "column"), "history ep column"));
			} else if (child.name == "rook") {
				if (rook_start) throw std::invalid_argument("duplicate <rook>");
				rook_start.emplace(
					ParseCoordinate(RequireAttribute(child, "startRow"), "rook start row"),
					ParseCoordinate(
						RequireAttribute(child, "startColumn"), "rook start column"));
				rook_end.emplace(
					ParseCoordinate(RequireAttribute(child, "endRow"), "rook end row"),
					ParseCoordinate(
						RequireAttribute(child, "endColumn"), "rook end column"));
			} else {
				throw std::invalid_argument("unknown move element: " + child.name);
			}
		}
		if (!moving || !castling_before) {
			throw std::invalid_argument("move lacks required state");
		}
		const auto promotion_attribute = Attribute(move, "promotion");
		std::optional<PieceType> promotion;
		if (promotion_attribute) promotion = ParseType(*promotion_attribute);
		state.history.emplace_back(
			*moving,
			ParseCoordinate(RequireAttribute(move, "startRow"), "start row"),
			ParseCoordinate(RequireAttribute(move, "startColumn"), "start column"),
			ParseCoordinate(RequireAttribute(move, "endRow"), "end row"),
			ParseCoordinate(RequireAttribute(move, "endColumn"), "end column"),
			captured,
			ParseKind(RequireAttribute(move, "kind")),
			promotion,
			rook_start,
			rook_end,
			*castling_before,
			en_passant_before,
			ParseUnsigned(
				RequireAttribute(move, "halfmoveBefore"), "halfmove before"),
			ParseUnsigned(
				RequireAttribute(move, "fullmoveBefore"), "fullmove before"),
			ParseColor(RequireAttribute(move, "turnBefore")));
	}
	const Node & repetition = RequireChild(root, "repetition");
	for (const Node & position : repetition.children) {
		if (position.name != "position") {
			throw std::invalid_argument(
				"unknown repetition element: " + position.name);
		}
		state.position_keys.push_back(RequireAttribute(position, "key"));
	}
	return state;
}
