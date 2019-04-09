#include "Chat.h"
#include "DES.h"
#include <iostream>

using namespace std;

int main() {

	DES encryptor;
	char str[4] = "123";
	char cipher[4];
	char plain[4];
	char key[8] = "1234567";
	key[7] = '8';
	int len = 8;
	encryptor.Encry(str, 4, cipher, len, key, 8);

	char buffer[1024];
	buffer[4] = 0;
	for (int j = 0; j < 4; j++)
		sprintf_s(&buffer[2 * j], 16, "%02X", cipher[j]);
	cout << buffer << endl;

	encryptor.Decry(cipher, 8, plain, len, key, 8);
	cout << plain << endl;


	string port;
	cout << "Input the listening port:" << endl;
	cin >> port;

	Chat c(port.c_str());
	c.start();
	return 0;
}