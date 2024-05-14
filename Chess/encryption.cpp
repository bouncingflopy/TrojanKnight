#pragma warning(disable : 4996)

#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <openssl/rand.h>
#include <string>
#include <iostream>
#include <vector>

#include "encryption.h"

struct RSADeleter {
    void operator() (RSA* rsa) const {
        RSA_free(rsa);
    }
};

pair<shared_ptr<RSA>, shared_ptr<RSA>> Encryption::generateRSAKeyPair() {
    RSA* rsa = RSA_new();
    BIGNUM* bn = BN_new();
    BN_set_word(bn, RSA_F4);
    RSA_generate_key_ex(rsa, RSA_KEY_SIZE, bn, NULL);

    shared_ptr<RSA> publicKey(RSAPublicKey_dup(rsa), RSADeleter{});
    shared_ptr<RSA> privateKey(RSAPrivateKey_dup(rsa), RSADeleter{});

    RSA_free(rsa);
    BN_free(bn);

    return make_pair(publicKey, privateKey);
}

string Encryption::exportRSAPublicKey(shared_ptr<RSA> key) {
    BIO* bio = BIO_new(BIO_s_mem());
    PEM_write_bio_RSA_PUBKEY(bio, key.get());
    char* pemKey;
    size_t pemKeyLen = BIO_get_mem_data(bio, &pemKey);
    string publicKey(pemKey, pemKeyLen);
    BIO_free(bio);
    return publicKey;
}

shared_ptr<RSA> Encryption::importRSAPublicKey(string& pem) {
    RSA* rsa = nullptr;
    BIO* bio = BIO_new_mem_buf(pem.c_str(), -1);
    if (bio) {
        rsa = PEM_read_bio_RSA_PUBKEY(bio, &rsa, NULL, NULL);
        BIO_free(bio);
    }
    return shared_ptr<RSA>(rsa, RSADeleter{});
}

string Encryption::encryptRSA(string& data, shared_ptr<RSA> key) {
    int max_length = RSA_size(key.get());
    string ciphertext(max_length, '\0');

    int encrypted_length = RSA_public_encrypt(data.size(), reinterpret_cast<const unsigned char*>(data.c_str()),
        reinterpret_cast<unsigned char*>(&ciphertext[0]), key.get(), RSA_PKCS1_PADDING);
    if (encrypted_length == -1) {
        // Error handling
        cerr << "Encryption failed" << endl;
        ERR_print_errors_fp(stderr);
        return "";
    }

    return ciphertext.substr(0, encrypted_length);
}

string Encryption::decryptRSA(string& data, shared_ptr<RSA> key) {
    int max_length = RSA_size(key.get());
    string plaintext(max_length, '\0');

    int decrypted_length = RSA_private_decrypt(data.size(), reinterpret_cast<const unsigned char*>(data.c_str()),
        reinterpret_cast<unsigned char*>(&plaintext[0]), key.get(), RSA_PKCS1_PADDING);
    if (decrypted_length == -1) {
        // Error handling
        cerr << "Decryption failed" << endl;
        ERR_print_errors_fp(stderr);
        return "";
    }

    return plaintext.substr(0, decrypted_length);
}

string Encryption::generateAESKey() {
    const int aes_key_size = AES_KEY_SIZE / 8;
    unsigned char key[aes_key_size];
    if (RAND_bytes(key, aes_key_size) != 1) {
        // Error handling
        cerr << "Error generating AES key" << endl;
        ERR_print_errors_fp(stderr);
        return "";
    }
    return string(reinterpret_cast<char*>(key), aes_key_size);
}

string Encryption::encryptAES(string& data, string key) {
    AES_KEY aes_key;
    if (AES_set_encrypt_key(reinterpret_cast<const unsigned char*>(key.c_str()), key.size() * 8, &aes_key) != 0) {
        cerr << "Error setting AES encryption key" << endl;
        ERR_print_errors_fp(stderr);
        return "";
    }

    // Create a copy of the input data
    string paddedData = data;

    // Calculate padding size
    int paddingSize = AES_BLOCK_SIZE - (data.size() % AES_BLOCK_SIZE);
    if (paddingSize == 0)
        paddingSize = AES_BLOCK_SIZE;

    // Append padding
    paddedData.append(paddingSize, static_cast<char>(paddingSize));

    unsigned char counter[AES_BLOCK_SIZE];
    memset(counter, 0, AES_BLOCK_SIZE);

    vector<unsigned char> ciphertext;

    for (size_t i = 0; i < paddedData.size(); i += AES_BLOCK_SIZE) {
        unsigned char encrypted_block[AES_BLOCK_SIZE];
        AES_encrypt(counter, encrypted_block, &aes_key);

        // XOR the encrypted counter block with the plaintext block
        for (size_t j = 0; j < AES_BLOCK_SIZE && i + j < paddedData.size(); ++j) {
            encrypted_block[j] ^= paddedData[i + j];
        }

        // Append the encrypted block to the ciphertext
        ciphertext.insert(ciphertext.end(), encrypted_block, encrypted_block + AES_BLOCK_SIZE);

        // Increment the counter
        for (int j = AES_BLOCK_SIZE - 1; j >= 0; --j) {
            if (++counter[j]) {
                break;
            }
        }
    }

    string encryptedData;
    encryptedData.assign(reinterpret_cast<char*>(ciphertext.data()), ciphertext.size());
    return encryptedData;
}

string Encryption::decryptAES(string& data, string key) {
    AES_KEY aes_key;
    if (AES_set_encrypt_key(reinterpret_cast<const unsigned char*>(key.c_str()), key.size() * 8, &aes_key) != 0) {
        cerr << "Error setting AES decryption key" << endl;
        ERR_print_errors_fp(stderr);
        return "";
    }

    string decryptedData;

    unsigned char counter[AES_BLOCK_SIZE];
    memset(counter, 0, AES_BLOCK_SIZE);

    vector<unsigned char> plaintext;

    for (size_t i = 0; i < data.size(); i += AES_BLOCK_SIZE) {
        unsigned char encrypted_block[AES_BLOCK_SIZE];
        AES_encrypt(counter, encrypted_block, &aes_key);

        // XOR the encrypted counter block with the ciphertext block
        for (size_t j = 0; j < AES_BLOCK_SIZE && i + j < data.size(); ++j) {
            encrypted_block[j] ^= data[i + j];
        }

        // Append the decrypted block to the plaintext
        plaintext.insert(plaintext.end(), encrypted_block, encrypted_block + AES_BLOCK_SIZE);

        // Increment the counter
        for (int j = AES_BLOCK_SIZE - 1; j >= 0; --j) {
            if (++counter[j]) {
                break;
            }
        }
    }

    // Remove padding
    size_t paddingSize = plaintext.back();
    decryptedData.assign(reinterpret_cast<char*>(plaintext.data()), plaintext.size() - paddingSize);

    return decryptedData;
}

string Encryption::encrypt(string& data, shared_ptr<RSA> key) {
    string aes = generateAESKey();
    string encrypted_aes = encryptRSA(aes, key);
    string encrypted_data = encryptAES(data, aes);

    return encrypted_aes + encrypted_data;
}

string Encryption::decrypt(string& data, shared_ptr<RSA> key) {
    string encrypted_aes = data.substr(0, RSA_KEY_SIZE / 8);
    string aes = decryptRSA(encrypted_aes, key);

    string encrypted_data = data.substr(RSA_KEY_SIZE / 8);
    string decrypted_data = decryptAES(encrypted_data, aes);

    return decrypted_data;
}

pair<string, string> Encryption::encryptVerbose(string& data, shared_ptr<RSA> key) {
    string aes = generateAESKey();
    string encrypted_aes = encryptRSA(aes, key);
    string encrypted_data = encryptAES(data, aes);

    return make_pair(encrypted_aes + encrypted_data, aes);
}

pair<string, string> Encryption::decryptVerbose(string& data, shared_ptr<RSA> key) {
    string encrypted_aes = data.substr(0, RSA_KEY_SIZE / 8);
    string aes = decryptRSA(encrypted_aes, key);

    string encrypted_data = data.substr(RSA_KEY_SIZE / 8);
    string decrypted_data = decryptAES(encrypted_data, aes);

    return make_pair(decrypted_data, aes);
}