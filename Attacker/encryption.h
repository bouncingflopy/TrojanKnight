#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <memory>
#include <openssl/rsa.h>

using namespace std;

const int RSA_KEY_SIZE = 2048;
const int AES_KEY_SIZE = 192;

namespace Encryption {
	pair<shared_ptr<RSA>, shared_ptr<RSA>> generateRSAKeyPair();
	string exportRSAPublicKey(shared_ptr<RSA> key);
	shared_ptr<RSA> importRSAPublicKey(string& pem);
	string encryptRSA(string& data, shared_ptr<RSA> key);
	string decryptRSA(string& data, shared_ptr<RSA> key);

	string generateAESKey();
	string encryptAES(string& data, string key);
	string decryptAES(string& data, string key);

	string encrypt(string& data, shared_ptr<RSA> key);
	string decrypt(string& data, shared_ptr<RSA> key);
	pair<string, string> encryptVerbose(string& data, shared_ptr<RSA> key);
	pair<string, string> decryptVerbose(string& data, shared_ptr<RSA> key);
}

#endif