/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/04/07
*/

#include "authCrypt.h"

#include <cryptopp/hex.h>
#include <cryptopp/files.h>

#define KEY_LENGTH  1024

CryptoPP::RandomPool    CryptRSA::sRandomPool;

CryptRSA::CryptRSA()
    : m_strKeyPub("")
    , m_strKeyPri("")
    , m_seed("zhongzi")
{
    init();
}

CryptRSA::~CryptRSA()
{

}

// generate key
void CryptRSA::init()
{
    CryptoPP::RandomPool randPool;
    /*
        Update RNG state with additional unpredictable values.

        Parameters
        input	the entropy to add to the generator
        length	the size of the input buffer
        Exceptions
        NotImplemented	
        A generator may or may not accept additional entropy. Call CanIncorporateEntropy() to test for the ability to use additional entropy.

        If a derived class does not override IncorporateEntropy(), then the base class throws NotImplemented.

        Reimplemented from RandomNumberGenerator.

        Definition at line 26 of file randpool.cpp.
    */
    randPool.IncorporateEntropy((byte*)m_seed.c_str(), m_seed.length());

    // generate private key
    m_keyPri = CryptoPP::RSAES_OAEP_SHA_Decryptor(randPool, KEY_LENGTH);

    // generate public key by using private key
    m_keyPub = CryptoPP::RSAES_OAEP_SHA_Encryptor(m_keyPri);

    return;
}

void CryptRSA::EncryptSend(std::string& plainText, std::string& cipherText)
{
    if(plainText.empty())
    {
        LOG(WARNING)<<"input plainText is empty";
        return;
    }

    CryptoPP::RandomPool randPool;
    randPool.IncorporateEntropy((byte*)m_seed.c_str(), m_seed.length());

    int msgLengthMax = m_keyPub.FixedMaxPlaintextLength();
    Evil_ASSERT(0 != msgLengthMax);

#ifdef DEBUG_INFO_CRYPT
    LOG(ERROR)<<"plainTextSize : "<<plainText.size();
    LOG(ERROR)<<"m_keyPub.FixedMaxPlaintextLength() : "<<msgLengthMax;
#endif

    for(int i = plainText.size(), j = 0; i > 0; i -= msgLengthMax, j += msgLengthMax)
    {
        std::string plainTextPart = plainText.substr(j, msgLengthMax);
        std::string cipherTextPart;
        // is memery leak ???
        CryptoPP::StringSource(plainTextPart, true, new CryptoPP::PK_EncryptorFilter(randPool, m_keyPub, new CryptoPP::HexEncoder(new CryptoPP::StringSink(cipherTextPart))));
        cipherText += cipherTextPart;
    }
#ifdef DEBUG_INFO_CRYPT
    LOG(ERROR)<<"cipherTextSize : "<<cipherText.size();
#endif
}

void CryptRSA::DecryptRecv(std::string& plainText, std::string& cipherText)
{
    if(cipherText.empty())
    {
        LOG(WARNING)<<"input cipherText is empty";
        return;
    }

    //indicate the ciphertext in hexcode ????
    int cipherTextLength = m_keyPri.FixedCiphertextLength() * 2;
    Evil_ASSERT(0 != cipherTextLength);

#ifdef DEBUG_INFO_CRYPT
    LOG(ERROR)<<"m_keyPri.FixedCiphertextLength() : "<<cipherTextLength;
    LOG(ERROR)<<"cipherTextSize : "<<cipherText.size();
#endif

    for(int i = cipherText.size(), j = 0; i > 0; i -= cipherTextLength, j += cipherTextLength)
    {
        std::string cipherTextPart = cipherText.substr(j, cipherTextLength);
        std::string plainTextPart;
        // is memery leak ???
        CryptoPP::StringSource(cipherTextPart, true, new CryptoPP::HexDecoder(new CryptoPP::PK_DecryptorFilter(getRandomPool(), m_keyPri, new CryptoPP::StringSink(plainTextPart))));
        plainText += plainTextPart;
    }
#ifdef DEBUG_INFO_CRYPT
    LOG(ERROR)<<"plainTextSize : "<<plainText.size();
#endif
}

void CryptRSA::EncryptSend(char* , size_t)
{
    
}

void CryptRSA::DecryptRecv(char* , size_t)
{
    
}

CryptoPP::RandomPool& CryptRSA::getRandomPool()
{
    return sRandomPool;
}





///************************** MyRSA.h ********************************/
//#ifndef __MYRSA_H__
//#define __MYRSA_H__
//
//#include <string>
//
//#include "files.h"
//#include "filters.h"
//#include "hex.h"
//#include "randpool.h"
//#include "rsa.h"
//
//using namespace std;
//using namespace CryptoPP;
//
//class CMyRSA
//{
//public:
//    CMyRSA(void);
//    virtual ~CMyRSA(void);
//
//    //You must set the KeyLength 512, 1024, 2048 ...
//    void GenerateKey(const unsigned int KeyLength, const char *Seed, RSAES_OAEP_SHA_Decryptor &Priv, RSAES_OAEP_SHA_Encryptor &Pub);
//    void GenerateKey(const unsigned int KeyLength, const char *Seed, string &strPriv, string &strPub);
//    
//    //use public key to encrypt
//    void EncryptString(const RSAES_OAEP_SHA_Encryptor &Pub, const char *Seed, const string &Plaintext, string &Ciphertext);
//    void EncryptString(const string &strPub, const char *Seed, const string &Plaintext, string &Ciphertext);
//
//    //use private key to decrypt
//    void DecryptString(const RSAES_OAEP_SHA_Decryptor &Priv, const string &Ciphertext, string &Plaintext);
//    void DecryptString(const string &strPriv, const string &Ciphertext, string &Plaintext);
//
//private:
//    static RandomPool & RNG(void);
//
//private:
//    static RandomPool m_sRandPool;
//};
//
//#endif /* End of __MYRSA_H__ */
//
//
///************************** MyRSA.cpp ********************************/
//#include "MyRSA.h"
//
//CMyRSA::CMyRSA()
//{
//
//}
//
//CMyRSA::~CMyRSA(void)
//{
//
//}
//
//void CMyRSA::GenerateKey(const unsigned int KeyLength, const char *Seed, RSAES_OAEP_SHA_Decryptor &Priv, RSAES_OAEP_SHA_Encryptor &Pub)
//{
//    RandomPool RandPool;
//    RandPool.IncorporateEntropy((byte *)Seed, strlen(Seed));
//    
//    //generate private key
//    Priv = RSAES_OAEP_SHA_Decryptor(RandPool, KeyLength);
//
//    //generate public key using private key
//    Pub = RSAES_OAEP_SHA_Encryptor(Priv);
//}
//
//void CMyRSA::GenerateKey(const unsigned int KeyLength, const char *Seed, string &strPriv, string &strPub)
//{
//    RandomPool RandPool;
//    RandPool.IncorporateEntropy((byte *)Seed, strlen(Seed));
//
//    //generate private key
//    RSAES_OAEP_SHA_Decryptor Priv(RandPool, KeyLength);
//    HexEncoder PrivateEncoder(new StringSink(strPriv));//本博客作者加: 就为了这句代码整整找了1天！
//    Priv.DEREncode(PrivateEncoder);
//    PrivateEncoder.MessageEnd();               
//
//    //generate public key using private key
//    RSAES_OAEP_SHA_Encryptor Pub(Priv);
//    HexEncoder PublicEncoder(new StringSink(strPub));
//    Pub.DEREncode(PublicEncoder);
//    PublicEncoder.MessageEnd();
//}
//
//void CMyRSA::EncryptString(const RSAES_OAEP_SHA_Encryptor &Pub, const char *Seed, const string &Plaintext, string &Ciphertext)
//{
//    RandomPool RandPool;
//    RandPool.IncorporateEntropy((byte *)Seed, strlen(Seed));
//
//    int MaxMsgLength = Pub.FixedMaxPlaintextLength();
//    for (int i = Plaintext.size(), j=0; i > 0; i -= MaxMsgLength, j += MaxMsgLength)
//    {
//        string PartPlaintext = Plaintext.substr(j, MaxMsgLength);
//        string PartCiphertext;
//        StringSource(PartPlaintext, true, new PK_EncryptorFilter(RandPool, Pub, new HexEncoder(new StringSink(PartCiphertext))));
//        Ciphertext += PartCiphertext;                  
//    }
//}
//
//void CMyRSA::EncryptString(const string &strPub, const char *Seed, const string &Plaintext, string &Ciphertext)
//{
//    StringSource PublicKey(strPub, true, new HexDecoder);
//    RSAES_OAEP_SHA_Encryptor Pub(PublicKey);
//
//    RandomPool RandPool;
//    RandPool.IncorporateEntropy((byte *)Seed, strlen(Seed));
//
//    int MaxMsgLength = Pub.FixedMaxPlaintextLength();
//    for (int i = Plaintext.size(), j=0; i > 0; i -= MaxMsgLength, j += MaxMsgLength)
//    {
//        string PartPlaintext = Plaintext.substr(j, MaxMsgLength);
//        string PartCiphertext;
//        StringSource(PartPlaintext, true, new PK_EncryptorFilter(RandPool, Pub, new HexEncoder(new StringSink(PartCiphertext))));
//        Ciphertext += PartCiphertext;                  
//    }
//}
//
//void CMyRSA::DecryptString(const RSAES_OAEP_SHA_Decryptor &Priv, const string &Ciphertext, string &Plaintext)
//{
//    //indicate the ciphertext in hexcode
//    int CiphertextLength = Priv.FixedCiphertextLength() * 2;
//    for (int i = Ciphertext.size(), j=0; i > 0; i -= CiphertextLength, j += CiphertextLength)
//    {
//            string PartCiphertext = Ciphertext.substr(j, CiphertextLength);
//            string PartPlaintext;
//            StringSource(PartCiphertext, true, new HexDecoder(new PK_DecryptorFilter(RNG(), Priv, new StringSink(PartPlaintext))));
//            Plaintext += PartPlaintext;
//    }
//}
//
//void CMyRSA::DecryptString(const string &strPriv, const string &Ciphertext, string &Plaintext)
//{
//    StringSource PrivKey(strPriv, true, new HexDecoder);
//    RSAES_OAEP_SHA_Decryptor Priv(PrivKey);
//    
//    //indicate the ciphertext in hexcode
//    int CiphertextLength = Priv.FixedCiphertextLength() * 2;
//    for (int i = Ciphertext.size(), j=0; i > 0; i -= CiphertextLength, j += CiphertextLength)
//    {
//            string PartCiphertext = Ciphertext.substr(j, CiphertextLength);
//            string PartPlaintext;
//            StringSource(PartCiphertext, true, new HexDecoder(new PK_DecryptorFilter(RNG(), Priv, new StringSink(PartPlaintext))));
//            Plaintext += PartPlaintext;
//    }
//}
//
//RandomPool & CMyRSA::RNG(void)
//{
//    return m_sRandPool;
//}
//
//RandomPool CMyRSA::m_sRandPool;
//
//
///************************** Main.h ********************************/
//#ifndef __MAIN_H__
//#define __MAIN_H__
//
//
//#endif /* End of __MAIN_H__ */
//
//
///************************** Main.cpp ********************************/
//#include <unistd.h>
//
//#include <iostream>
//
//#include "Main.h"
//#include "MyRSA.h"
//
///***** STATIC VARIABLES *****/
//static RSAES_OAEP_SHA_Encryptor g_Pub;
//static RSAES_OAEP_SHA_Decryptor g_Priv;
//static string g_strPub;
//static string g_strPriv;
//
//int main(int argc, char *argv[])
//{
//    try
//    {
//        char Seed[1024], Message[1024], MessageSeed[1024];
//        unsigned int KeyLength;
//        CMyRSA MyRSA;
//
//        cout << "Key length in bits: ";
//        cin >> KeyLength;
//
//        cout << "/nRandom Seed: ";
//        ws(cin);
//        cin.getline(Seed, 1024);
//
//        cout << "/nMessage: ";
//        ws(cin);    
//        cin.getline(Message, 1024);    
//
//        cout << "/nRandom Message Seed: ";
//        ws(cin);
//        cin.getline(MessageSeed, 1024);
//        
//        MyRSA.GenerateKey(KeyLength, Seed, g_Priv, g_Pub);
//        //MyRSA.GenerateKey(KeyLength, Seed, g_strPriv, g_strPub);
//
//        //If generate key in RSAES_OAEP_SHA_Encryptor and RSAES_OAEP_SHA_Decryptor, please note four lines below
//        /*
//        cout << "g_strPub = " << g_strPub << endl;
//        cout << endl;
//        cout << "g_strPriv = " << g_strPriv << endl;
//        cout << endl;
//        */
//        
//        string Plaintext(Message);
//        string Ciphertext;
//        MyRSA.EncryptString(g_Pub, MessageSeed, Plaintext, Ciphertext);
//        //MyRSA.EncryptString(g_strPub, MessageSeed, Plaintext, Ciphertext);
//        cout << "/nCiphertext: " << Ciphertext << endl;
//        cout << endl;
//
//        string Decrypted;
//        MyRSA.DecryptString(g_Priv, Ciphertext, Decrypted);
//        //MyRSA.DecryptString(g_strPriv, Ciphertext, Decrypted);
//        cout << "/nDecrypted: " << Decrypted << endl;
//        return 0;
//    }
//    catch(CryptoPP::Exception const &e)
//    {
//        cout << "/nCryptoPP::Exception caught: " << e.what() << endl;
//        return -1;
//    }
//    catch(std::exception const &e)
//    {
//        cout << "/nstd::exception caught: " << e.what() << endl;
//        return -2;
//    }
//    return -3;
//}
//
///************************** Makefile ********************************/
//# The executable file name.
//PROGRAM := myrsa
//
//# The directories in which source files reside.
//SRCDIRS := . # current directory
//
//# The source file types (headers excluded).
//SRCEXTS := .cpp
//
//# The flags used by the cpp (man cpp for more).
//CPPFLAGS := 
//
//# The compiling flags used only for C.
//# If it is a C++ program, no need to set these flags.
//# If it is a C and C++ merging program, set these flags for the C parts.
//
//CFLAGS := 
//
//CFLAGS +=
//
//# The compiling flags used only for C++.
//# If it is a C program, no need to set these flags.
//# If it is a C and C++ merging program, set these flags for the C++ parts.
//
//CXXFLAGS := -g -O2 -I./include
//
//CXXFLAGS +=
//
//# The library and the link options ( C and C++ common).
//
//LDFLAGS := -L./lib -lcryptopp
//
//LDFLAGS +=
//
//
//## Implict Section: change the following only when necessary.
//
//##=============================================================================
//
//# The C program compiler. Uncomment it to specify yours explicitly.
//
//#CC = gcc
//
//# The C++ program compiler. Uncomment it to specify yours explicitly.
//
//CXX = g++
//
//# Uncomment the 2 lines to compile C programs as C++ ones.
//
//CC = $(CXX)
//
//CFLAGS = $(CXXFLAGS)
//
//# The command used to delete file.
//
//RM = rm -f
//
//
//## Stable Section: usually no need to be changed. But you can add more.
//
//##=============================================================================
//
//SHELL = /bin/sh
//
//SOURCES = $(foreach d,$(SRCDIRS),$(wildcard $(addprefix $(d)/*,$(SRCEXTS))))
//
//OBJS = $(foreach x,$(SRCEXTS), /
//      $(patsubst %$(x),%.o,$(filter %$(x),$(SOURCES))))
//
//DEPS = $(patsubst %.o,%.d,$(OBJS))
//
//
//
//.PHONY : all objs clean cleanall rebuild
//
//all : $(PROGRAM)
//
//
//
//# Rules for creating the dependency files (.d).
//
//#---------------------------------------------------
//
//%.d : %.c
//
//    @$(CC) -MM -MD $(CFLAGS) $<
//
//%.d : %.C
//
//    @$(CC) -MM -MD $(CXXFLAGS) $<
//
//%.d : %.cc
//
//    @$(CC) -MM -MD $(CXXFLAGS) $<
//
//%.d : %.cpp
//
//    @$(CC) -MM -MD $(CXXFLAGS) $<
//
//%.d : %.CPP
//
//    @$(CC) -MM -MD $(CXXFLAGS) $<
//
//%.d : %.c++
//
//    @$(CC) -MM -MD $(CXXFLAGS) $<
//
//%.d : %.cp
//
//    @$(CC) -MM -MD $(CXXFLAGS) $<
//
//%.d : %.cxx
//
//    @$(CC) -MM -MD $(CXXFLAGS) $<
//
//# Rules for producing the objects.
//
//#---------------------------------------------------
//
//objs : $(OBJS)
//
//%.o : %.c
//
//    $(CC) -c $(CPPFLAGS) $(CFLAGS) $<
//
//%.o : %.C
//
//    $(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<
//
//%.o : %.cc
//
//    $(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<
//
//%.o : %.cpp
//
//    $(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<
//
//%.o : %.CPP
//
//    $(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<
//
//%.o : %.c++
//
//    $(CXX -c $(CPPFLAGS) $(CXXFLAGS) $<
//
//%.o : %.cp
//
//    $(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<
//
//%.o : %.cxx
//
//    $(CXX) -c $(CPPFLAGS) $(CXXFLAGS) $<
//
//# Rules for producing the executable.
//
//#----------------------------------------------
//
//$(PROGRAM) : $(OBJS)
//
//ifeq ($(strip $(SRCEXTS)), .c) # C file
//
//    $(CC) -o $(PROGRAM) $(OBJS) $(LDFLAGS)
//
//else                            # C++ file
//
//    $(CXX) -o $(PROGRAM) $(OBJS) $(LDFLAGS)
//
//endif
//
//-include $(DEPS)
//
//rebuild: clean all
//
//clean :
//    @$(RM) *.o *.d
//cleanall: clean
//    @$(RM) $(PROGRAM) $(PROGRAM).exe