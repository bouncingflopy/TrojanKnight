#pragma warning(disable : 4996)

#include <iostream>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <string>

#include "encryption.h"

// Function to generate RSA key pair and return public and private keys
std::pair<RSA*, RSA*> Encryption::generateRSAKeyPair(int keyLength) {
    RSA* rsa = RSA_new();
    BIGNUM* bn = BN_new();
    BN_set_word(bn, RSA_F4);
    RSA_generate_key_ex(rsa, keyLength, bn, NULL);

    RSA* publicKey = RSAPublicKey_dup(rsa);
    RSA* privateKey = RSAPrivateKey_dup(rsa);

    RSA_free(rsa);
    BN_free(bn);

    return std::make_pair(publicKey, privateKey);
}

// Function to encrypt data using RSA public key
std::string Encryption::encryptRSA(const std::string& plaintext, RSA* publicKey) {
    int max_length = RSA_size(publicKey);
    std::string ciphertext(max_length, '\0');

    int encrypted_length = RSA_public_encrypt(plaintext.size(), reinterpret_cast<const unsigned char*>(plaintext.c_str()),
        reinterpret_cast<unsigned char*>(&ciphertext[0]), publicKey, RSA_PKCS1_PADDING);
    if (encrypted_length == -1) {
        // Error handling
        std::cerr << "Encryption failed" << std::endl;
        ERR_print_errors_fp(stderr);
        return "";
    }

    return ciphertext.substr(0, encrypted_length);
}

// Function to decrypt data using RSA private key
std::string Encryption::decryptRSA(const std::string& ciphertext, RSA* privateKey) {
    int max_length = RSA_size(privateKey);
    std::string plaintext(max_length, '\0');

    int decrypted_length = RSA_private_decrypt(ciphertext.size(), reinterpret_cast<const unsigned char*>(ciphertext.c_str()),
        reinterpret_cast<unsigned char*>(&plaintext[0]), privateKey, RSA_PKCS1_PADDING);
    if (decrypted_length == -1) {
        // Error handling
        std::cerr << "Decryption failed" << std::endl;
        ERR_print_errors_fp(stderr);
        return "";
    }

    return plaintext.substr(0, decrypted_length);
}

// Function to export RSA public key to PEM format
std::string Encryption::exportRSAPublicKey(RSA* rsaKey) {
    BIO* bio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSA_PUBKEY(bio, rsaKey);
    char* pemKey;
    size_t pemKeyLen = BIO_get_mem_data(bio, &pemKey);
    std::string publicKey(pemKey, pemKeyLen);
    BIO_free(bio);
    return publicKey;
}

// Function to import RSA public key from PEM format
RSA* Encryption::importRSAPublicKey(const std::string& publicKeyPEM) {
    RSA* rsa = nullptr;
    BIO* bio = BIO_new_mem_buf(publicKeyPEM.c_str(), -1);
    if (bio) {
        rsa = PEM_read_bio_RSA_PUBKEY(bio, &rsa, NULL, NULL);
        BIO_free(bio);
    }
    return rsa;
}

void Encryption::main() {
    // Generate RSA key pair
    std::pair<RSA*, RSA*> keys = generateRSAKeyPair(2048);

    RSA* publicKey = keys.first;
    RSA* privateKey = keys.second;

    // Example plaintext
    std::string plaintext = "Hello, world!";

    // Export public key
    std::string publicKeyPEM = exportRSAPublicKey(publicKey);

    std::cout << "Exported public key: " << publicKeyPEM << std::endl;

    // Import public key from PEM format
    RSA* importedPublicKey = importRSAPublicKey(publicKeyPEM);

    // Encrypt plaintext using RSA public key
    std::string encryptedMessage = encryptRSA(plaintext, importedPublicKey);

    // Verify the signature using imported public key
    std::string decryptedText = decryptRSA(encryptedMessage, privateKey);

    // Output results
    std::cout << "Original text: " << plaintext << std::endl;
    std::cout << "Encrypted message: " << encryptedMessage << std::endl;
    std::cout << "Decrypted text: " << decryptedText << std::endl;

    // Clean up resources
    RSA_free(publicKey);
    RSA_free(privateKey);
    RSA_free(importedPublicKey);
}