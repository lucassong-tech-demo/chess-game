/*** Test method for the URL class ***/

bool URL::Test(ostream & os) {

	bool success = true;

	URL url("http://www.byu.edu/this/is/a/path/index.html;lang=engl?team=Dallas#Basketball");

	TEST(url.GetScheme() != "http");

	URL resolved = url.ResolveRelative("../../../fred.gif");
	TEST(resolved == URL("http://www.byu.edu/this/fred.gif"));

	resolved = url.ResolveFragment("Football");
	TEST(resolved == URL("http://www.byu.edu/this/is/a/path/index.html;lang=engl?team=Dallas#Football"));

	return success;
}


/*** Test method for the FileUtils class ***/

bool FileUtils::Test(ostream & os) {

	bool success = true;

	FileUtils::CopyFile("cs240utils.tar", "copy-of-cs240utils.tar");
	TEST(CompareFiles("cs240utils.tar", "copy-of-cs240utils.tar"));

	return success;
}


/*** helper method for FileUtils::Test ***/

bool FileUtils::CompareFiles(const string & filename1, 
		const string & filename2) {

	ifstream file1(filename1.c_str());
	ifstream file2(filename2.c_str());

	if (!file1 || !file2) {
		return false;
	}

	while (true) {
		int c1 = file1.get();
		int c2 = file2.get();
		if (c1 != c2) {
			return false;
		}
		if (c1 == -1) {
			return true;
		}
	}
}


/*** Driver program that calls the Test method for each class ***/

int main() {

	bool success = true;

	if (!URL::Test(cout)) success = false;
	if (!FileUtils::Test(cout)) success = false;

	if (success) {
		cout << "Tests Succeeded!" << endl;
	}
	else {
		cout << "Tests Failed!" << endl;
	}

	return 0;
}

