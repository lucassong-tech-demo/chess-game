#include <iostream>
#include "URLInputStream.h"
#include "CS240Exception.h"


void print_contents (InputStream &stream);

/**
 * printer takes a list of urls from he command line and prints their contents to std::cout
 * 
 * usage: printer [files]
 * 
 * ex. 
 *     printer http://google.com file:/home/somefile.txt
 * */
int main (int argc, char* argv [])
{
	
		
	

		for(int i = 1;i<argc;++i)
		{
			std::string path=argv[i];
			//Print source URL
			std::cout << "Printing: " << path << std::endl;
			std::cout << "=======================================" << std::endl;
			
			try
			{
				URLInputStream stream (path);
				print_contents (stream);

				//Handle redirections (Find out where we redirected to)
				path = stream.GetLocation();
				std::cout << "=======================================" << std::endl;
				std::cout << "Actual Location: " << path << std::endl;
			}
			catch (std::exception &e)
			{
				std::cout << "Exception Occurred:" << e.what() << std::endl;
			}
			catch (CS240Exception &e) 
			{
				std::cout << "Exception Occurred:" << e.GetMessage() << std::endl;
			}
			catch (...)
			{
				std::cout << "Unknown Exception Occurred" << std::endl;
			}
		}
				
	
	return 0;
}

//!Note that this function can accept any kind of InputStream, not just URLInputStream
void print_contents (InputStream &stream)
{
	
	while (!stream.IsDone())
	{
		std::cout << stream.Read();
	}
}

