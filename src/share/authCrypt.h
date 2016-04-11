/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/04/07
*/

#ifndef _AUTH_CRYPT_H_
#define _AUTH_CRYPT_H_

#include "Common.h"

#include <cryptopp/rsa.h>
#include <cryptopp/randpool.h>

class Crypt
{
public:
    Crypt()
    {

    }

    virtual ~Crypt()
    {

    }

public:
    virtual void EncryptSend(char* data, size_t len) = 0;
    virtual void EncryptSend(std::string& plainText, std::string& cipherText) = 0;

    virtual void DecryptRecv(char* data, size_t len) = 0;
    virtual void DecryptRecv(std::string& plainText, std::string& cipherText) = 0;
};

class CryptRSA
    : public Crypt
{
public:
    CryptRSA();
    ~CryptRSA();

public:
    void EncryptSend(char* data, size_t len) override;
    void EncryptSend(std::string& plainText, std::string& cipherText) override;

    void DecryptRecv(char* data, size_t len) override;
    void DecryptRecv(std::string& plainText, std::string& cipherText) override;
    

    void init();

private:
    CryptoPP::RSAES_OAEP_SHA_Encryptor      m_keyPub;
    CryptoPP::RSAES_OAEP_SHA_Decryptor      m_keyPri;
    std::string         m_strKeyPub;
    std::string         m_strKeyPri;
    std::string         m_seed;

    static  CryptoPP::RandomPool    sRandomPool;
    static  CryptoPP::RandomPool&   getRandomPool();
};

#endif//_AUTH_CRYPT_H_