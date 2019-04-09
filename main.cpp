#include "Chat.h"
#include <iostream>

using namespace std;

int main() {

	string port;
	cout << "Input the listening port:" << endl;
	cin >> port;

	Chat c(port.c_str());
	c.start();
	return 0;
}