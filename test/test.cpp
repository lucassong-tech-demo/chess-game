#include <iostream>
#include <string>
#include <cstring>
#include "ChessBoard.h"
#include "GameFacade.h"

using namespace std;

int main() {

	bool success = true;


	cout<<"Chess board testing ...."<<endl;
	if (!ChessBoard::Test(cout)) success = false;

	if (success) {
		cout << "Tests Succeeded!" << endl;
	}
	else {
		cout << "Tests Failed!" << endl;
	}

	cout<<"Game Facade testing ...."<<endl;
	if (!GameFacade::Test(cout)) success = false;

	if (success) {
		cout << "Tests Succeeded!" << endl;
	}
	else {
		cout << "Tests Failed!" << endl;
	}


	return 0;
}


