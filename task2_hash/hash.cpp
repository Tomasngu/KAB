#ifndef __PROGTEST__
#include <assert.h>
#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

#include <openssl/evp.h>
#include <openssl/rand.h>

#endif /* __PROGTEST__ */
using namespace std;

int checker(int bits, const unsigned char * hash) {
    int zeros = 0;
    for(int i = 0; i < 64 ; i++){
        unsigned char x = hash[i];
        for(int j = 1 << 7; j >= 1; j >>= 1){
            if(!(x & j)){
                zeros++;
                if(zeros >= bits) return 1;
            }
            else{
                return zeros >= bits;
            }
        }
    }
    return 0;
}
char * getHex(const char * toHex, size_t len){
    char * hex = (char *) malloc(2*len + 10);
    for(size_t i = 0, j = 0; i < len; i++,  j+= 2){
        sprintf(hex + j, "%02x", toHex[i] & 0xff);
    }
    return hex;
}

string rand_string(size_t length){
    string letters = "1234567890qwertyuiopasdfghjklzxcvbnm";
    string ret;
    for(size_t i = 0; i < length; i++){
        ret += letters[rand() % letters.size()];
    }
    return ret;
}

int findHashEx (int bits, char ** retMessage, char ** retHash, const char * hashFunction) {
    if(bits < 0 || bits > 8*64) return 0;

    /*COURSES*/
    EVP_MD_CTX * ctx;  // struktura kontextu
    const EVP_MD * type; // typ pouzite hashovaci funkce
    unsigned char hash[EVP_MAX_MD_SIZE]; // char pole pro hash - 64 bytu (max pro sha 512)
    unsigned int length;  // vysledna delka hashe
    /* Inicializace OpenSSL hash funkci */
    OpenSSL_add_all_digests();
    /* Zjisteni, jaka hashovaci funkce ma byt pouzita */
    type = EVP_get_digestbyname(hashFunction);
    if (!type) {
        return 0;
    }
    ctx = EVP_MD_CTX_new();

    while(true){
        string text = rand_string(16);
        if (!EVP_DigestInit_ex(ctx, type, NULL)) // context setup for our hash type
            return 0;
        if (!EVP_DigestUpdate(ctx, text.c_str(), text.size())) // feed the message in
            return 0;
        if (!EVP_DigestFinal_ex(ctx, hash, &length)) // get the hash
            return 0;
        if(length * 8 < (size_t) bits) return 0;

        if (checker(bits, hash)){
            *retMessage = getHex(text.c_str(), 16);
            *retHash = getHex(reinterpret_cast<char *>(hash), (size_t) length);
            break;
        }
    }
    EVP_MD_CTX_free(ctx); 
    return 1;
}
int findHash (int bits, char ** retMessage, char ** retHash) {
    return findHashEx(bits, retMessage, retHash, "sha512");
}

#ifndef __PROGTEST__
int checkHash(int bits, const char * hexString) {
    int zeros = 0;
    for(size_t i = 0; i < strlen(hexString); i++){
        char x = hexString[i];
        if (x >= '8'){
            return zeros >= bits;
        }
        if (x >= '4'){
            return zeros + 1 >= bits;
        }
        if (x >= '2'){
            return zeros + 2 >= bits;
        }
        if (x == '1'){
            return zeros + 3 >= bits;
        }
        zeros += 4;

    }
    return 0;
}

int main (void) {
    char * message, * hash;
    assert(findHashEx(0, &message, &hash, "md5") == 1);
    assert(message && hash && checkHash(0, hash));
    assert(findHash(0, &message, &hash) == 1);
    assert(message && hash && checkHash(0, hash));
    free(message);
    free(hash);
    assert(findHash(1, &message, &hash) == 1);
    assert(message && hash && checkHash(1, hash));
    free(message);
    free(hash);
    assert(findHash(2, &message, &hash) == 1);
    assert(message && hash && checkHash(2, hash));
    free(message);
    free(hash);
    assert(findHash(10, &message, &hash) == 1);
    assert(message && hash && checkHash(10, hash));
    free(message);
    free(hash);
    assert(findHash(-1, &message, &hash) == 0);
    return EXIT_SUCCESS;
}
#endif /* __PROGTEST__ */

