#ifndef ENCRYPTION_H
#define ENCRYPTION_H

#include <openssl/rsa.h>

using namespace std;

namespace Encryption {
	pair<RSA*, RSA*> generateRSAKeyPair(int keyLength);
	string encryptRSA(const std::string& plaintext, RSA* publicKey);
	string decryptRSA(const std::string& ciphertext, RSA* privateKey);
	string exportRSAPublicKey(RSA* rsaKey);
	RSA* importRSAPublicKey(const std::string& publicKeyPEM);
	void main();
}

#endif