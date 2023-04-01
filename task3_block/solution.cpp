#ifndef __PROGTEST__
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <climits>
#include <cstdint>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <unistd.h>
#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <cassert>
#include <cstring>

#include <openssl/evp.h>
#include <openssl/rand.h>

using namespace std;

struct crypto_config
{
	const char * m_crypto_function;
	std::unique_ptr<uint8_t[]> m_key;
	std::unique_ptr<uint8_t[]> m_IV;
	size_t m_key_len;
	size_t m_IV_len;
};

#endif /* _PROGTEST_ */

struct HeaderTGA{
        std::uint8_t ID_length;
        std::uint8_t colorMapType;
        std::uint8_t imageType;
        std::uint8_t colorSpecs[5];
        std::uint8_t Coords[4];
        std::uint16_t Width;          
        std::uint16_t Height; 
        std::uint8_t bitsPerPixel;
        std::uint8_t Alpha;
};

string rand_string(size_t length){
    string letters = "1234567890qwertyuiopasdfghjklzxcvbnm";
    string ret;
    for(size_t i = 0; i < length; i++){
        ret += letters[rand() % letters.size()];
    }
    return ret;
}


const int BUFFER_LEN = 1024;

bool copyHeader(std::ifstream & ifs, std::ofstream & ofs, const string & fileName){
    if(!ifs.good()) return false;
	HeaderTGA header;
    ifs.read(reinterpret_cast<char *>(&header), sizeof(header));
    if(!ifs.good()) return false;
	ofs.write(reinterpret_cast<char *>(&header), sizeof(header));
	if(!ofs.good()) return false; 
	ofs.flush();
    return true;
}

bool encrypt_decrypt(const std::string & in_filename, const std::string & out_filename, crypto_config & config, int encrypt, const EVP_CIPHER * cipher){
	ifstream ifs(in_filename, std::ios::in | std::ios::binary);
	ofstream ofs(out_filename, ios::out | ios::binary);

	if(!ifs.good() || !ofs.good()) return false;
    if (!copyHeader(ifs, ofs, in_filename)) return false;
	unsigned char bufferIn[BUFFER_LEN];
	unsigned char bufferOut[2*BUFFER_LEN];

	EVP_CIPHER_CTX * ctx; // context structure
    ctx = EVP_CIPHER_CTX_new();
	if(!ctx){
		EVP_CIPHER_CTX_free(ctx);
		return false;
	}

	if (!EVP_CipherInit_ex(ctx, cipher, NULL, config.m_key.get(), config.m_IV.get(), encrypt)){ // context init - set cipher, key, init vector
        EVP_CIPHER_CTX_free(ctx);
		return false;
	}

	int inLength = BUFFER_LEN;
	int outSize = 0;
	bool done = false;
	while(!done){
		ifs.read(reinterpret_cast<char *>(bufferIn), BUFFER_LEN);
		if(ifs.eof()){
			inLength = ifs.gcount();
			done = true;
		}
		if (!EVP_CipherUpdate(ctx, bufferOut, &outSize, bufferIn, inLength) ){
			EVP_CIPHER_CTX_free(ctx);
			return false;
		}
		ofs.write(reinterpret_cast<char *>(bufferOut), outSize);
		if(!ofs.good()){
			EVP_CIPHER_CTX_free(ctx);
			return false;
		}
	}
	
	if ( !EVP_CipherFinal_ex(ctx, bufferOut, &outSize) ){
		EVP_CIPHER_CTX_free(ctx);
		return false;
	}
	ofs.write(reinterpret_cast<char *>(bufferOut), outSize);
	if(!ofs.good()){
		EVP_CIPHER_CTX_free(ctx);
		return false;
	}
	EVP_CIPHER_CTX_free(ctx);
	return true;
}

bool encrypt_data ( const std::string & in_filename, const std::string & out_filename, crypto_config & config ){
	
	const EVP_CIPHER * cipher;
	OpenSSL_add_all_ciphers();
    /* sifry i hashe by se nahraly pomoci OpenSSL_add_all_algorithms() */
    cipher = EVP_get_cipherbyname(config.m_crypto_function);
	if (!cipher) return false;
	// cout << "I want " << EVP_CIPHER_key_length(cipher) << " key to encrypt" << endl;
	// cout << "I want " << EVP_CIPHER_iv_length(cipher) << " IV to decrypt" << endl;

	int neededKeySize = EVP_CIPHER_key_length(cipher);
	if((int) config.m_key_len < neededKeySize){
		string newKey = rand_string(neededKeySize);
		config.m_key = std::make_unique<uint8_t[]>(neededKeySize);
		strncpy((char *) config.m_key.get(), newKey.c_str(), neededKeySize );
		config.m_key_len = neededKeySize;
		// cout << "REDOING KEY" << endl;
	}
	int neededIVSize = EVP_CIPHER_iv_length(cipher);
	// if(config.m_IV_len && !config.m_IV) return false;
	if((int) config.m_IV_len < neededIVSize){
		string newIV = rand_string(neededIVSize);
		config.m_IV = std::make_unique<uint8_t[]>(neededIVSize);
		strncpy((char *) config.m_IV.get(), newIV.c_str(), neededIVSize );
		config.m_IV_len = neededIVSize;
		// cout << "REDOING IV" << endl;
	}

	return encrypt_decrypt(in_filename, out_filename, config, 1, cipher);
}

bool decrypt_data ( const std::string & in_filename, const std::string & out_filename, crypto_config & config )
{	
	const EVP_CIPHER * cipher;
	OpenSSL_add_all_ciphers();
    /* sifry i hashe by se nahraly pomoci OpenSSL_add_all_algorithms() */
    cipher = EVP_get_cipherbyname(config.m_crypto_function);
	if (!cipher) return false;

	// cout << "I want " << EVP_CIPHER_key_length(cipher) << " key to decrypt" << endl;
	// cout << "I want " << EVP_CIPHER_iv_length(cipher) << " IV to decrypt" << endl;

	int neededKeySize = EVP_CIPHER_key_length(cipher);
	if(!config.m_key || (int) config.m_key_len < neededKeySize) return false;

	int neededIVSize = EVP_CIPHER_iv_length(cipher);
	if((int) config.m_IV_len < neededIVSize) return false;
	if(config.m_IV_len && !config.m_IV) return false;

	return encrypt_decrypt(in_filename, out_filename, config, 0, cipher);
}


#ifndef __PROGTEST__

bool compare_files ( const char * name1, const char * name2 )
{
	string n1 = name1;
	string n2 = name2;
	string command = "diff " + n1 + " " + n2;
	return system(command.c_str()) == 0;
}

int main ( void )
{
	crypto_config config {nullptr, nullptr, nullptr, 0, 0};

	// ECB mode
	config.m_crypto_function = "AES-128-ECB";
	config.m_key = std::make_unique<uint8_t[]>(16);
 	memset(config.m_key.get(), 0, 16);
	config.m_key_len = 16;
	assert( encrypt_data  ("homer-simpson.TGA", "out_file.TGA", config));
	assert(decrypt_data  ("out_file.TGA", "homer.TGA", config));
	assert(compare_files("homer-simpson.TGA","homer.TGA" ));
	assert( encrypt_data  ("homer-simpson.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "homer-simpson_enc_ecb.TGA") );

	assert( decrypt_data  ("homer-simpson_enc_ecb.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "homer-simpson.TGA") );

	assert( encrypt_data  ("UCM8.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "UCM8_enc_ecb.TGA") );

	assert( decrypt_data  ("UCM8_enc_ecb.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "UCM8.TGA") );

	assert( encrypt_data  ("image_1.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "ref_1_enc_ecb.TGA") );

	assert( encrypt_data  ("image_2.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "ref_2_enc_ecb.TGA") );

	assert( decrypt_data ("image_3_enc_ecb.TGA", "out_file.TGA", config)  &&
		    compare_files("out_file.TGA", "ref_3_dec_ecb.TGA") );

	assert( decrypt_data ("image_4_enc_ecb.TGA", "out_file.TGA", config)  &&
		    compare_files("out_file.TGA", "ref_4_dec_ecb.TGA") );

	// // CBC mode
	config.m_crypto_function = "AES-128-CBC";
	config.m_IV = std::make_unique<uint8_t[]>(16);
	config.m_IV_len = 16;
	memset(config.m_IV.get(), 0, 16);

	assert( encrypt_data  ("UCM8.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "UCM8_enc_cbc.TGA") );

	assert( decrypt_data  ("UCM8_enc_cbc.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "UCM8.TGA") );

	assert( encrypt_data  ("homer-simpson.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "homer-simpson_enc_cbc.TGA") );

	assert( decrypt_data  ("homer-simpson_enc_cbc.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "homer-simpson.TGA") );

	assert( encrypt_data  ("image_1.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "ref_5_enc_cbc.TGA") );

	assert( encrypt_data  ("image_2.TGA", "out_file.TGA", config) &&
			compare_files ("out_file.TGA", "ref_6_enc_cbc.TGA") );

	assert( decrypt_data ("image_7_enc_cbc.TGA", "out_file.TGA", config)  &&
		    compare_files("out_file.TGA", "ref_7_dec_cbc.TGA") );

	assert( decrypt_data ("image_8_enc_cbc.TGA", "out_file.TGA", config)  &&
		    compare_files("out_file.TGA", "ref_8_dec_cbc.TGA") );
	return 0;
}

#endif /* _PROGTEST_ */
