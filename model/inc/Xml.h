#ifndef XML_H
#define XML_H

#include <string>

#include "GameState.h"

class Xml
{
public:
	explicit Xml(std::string filename);

	void WriteIntoFile(const SavedGameState & state) const;
	SavedGameState ReadFromFile() const;

private:
	std::string file_name_;
};

#endif
