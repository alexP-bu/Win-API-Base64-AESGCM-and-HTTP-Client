#include "aes_gcm.h"


AESGCM:: ~AESGCM(){
    Cleanup();
}

AESGCM::AESGCM( BYTE key[AES_256_KEY_SIZE]){
    hAlg = 0;
    hKey = NULL;

    // create a handle to an AES-GCM provider
    nStatus = ::BCryptOpenAlgorithmProvider(
        &hAlg, 
        BCRYPT_AES_ALGORITHM, 
        NULL, 
        0);
    if (! NT_SUCCESS(nStatus))
    {
        printf("**** Error 0x%x returned by BCryptOpenAlgorithmProvider\n", nStatus);
        Cleanup();
        return;
    }
    if (!hAlg){
        printf("Invalid handle!\n");
    }
    nStatus = ::BCryptSetProperty(
        hAlg, 
        BCRYPT_CHAINING_MODE, 
        (BYTE*)BCRYPT_CHAIN_MODE_GCM, 
        sizeof(BCRYPT_CHAIN_MODE_GCM), 
        0);
    if (!NT_SUCCESS(nStatus)){
         printf("**** Error 0x%x returned by BCryptGetProperty ><\n", nStatus);
         Cleanup();
         return;
    }
  

    nStatus = ::BCryptGenerateSymmetricKey(
        hAlg, 
        &hKey, 
        NULL, 
        0, 
        key, 
        AES_256_KEY_SIZE, 
        0);
    if (!NT_SUCCESS(nStatus)){
        printf("**** Error 0x%x returned by BCryptGenerateSymmetricKey\n", nStatus);
        Cleanup();
        return;
    }
    DWORD cbResult = 0;
     nStatus = ::BCryptGetProperty(
         hAlg, 
         BCRYPT_AUTH_TAG_LENGTH, 
         (BYTE*)&authTagLengths, 
         sizeof(authTagLengths), 
         &cbResult, 
         0);
   if (!NT_SUCCESS(nStatus)){
       printf("**** Error 0x%x returned by BCryptGetProperty when calculating auth tag len\n", nStatus);
   }

   
}


void AESGCM::Decrypt(BYTE* nonce, size_t nonceLen, BYTE* data, size_t dataLen, BYTE* macTag, size_t macTagLen){
    NTSTATUS status;
    //get buffer size
    ULONG pcbResult;
    status = BCryptDecrypt(hKey, data, dataLen, NULL, nonce, nonceLen, NULL, 0, &pcbResult, 0);
    if (!NT_SUCCESS(status)){
    	printf("[!] Error getting buffer size: %x", status);
      Cleanup();
      return;
    }

    //allocate memory for the plaintext
    plaintext = (BYTE*)malloc(pcbResult);
    if(!NT_SUCCESS(status)){
      printf("[!] Error allocating memory: %x", status);
      Cleanup();
      return;
    }

    /*setup the decryption*/
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO auth;
    BCRYPT_INIT_AUTH_MODE_INFO(auth);
    auth.pbNonce = nonce;
    auth.cbNonce = nonceLen;
    auth.pbAuthData = NULL;
    auth.cbAuthData = 0;
    auth.pbTag = macTag;
    auth.cbTag = macTagLen;
    auth.pbMacContext = NULL;
    auth.cbMacContext = 0;
    auth.cbAAD = 0;
    auth.cbData = 0;
    auth.dwFlags = 0;
    ULONG numBytes = 0;
    status = BCryptDecrypt(hKey, data, dataLen, &auth, nonce, nonceLen, plaintext, pcbResult, &numBytes, 0);
		if(!NT_SUCCESS(status)){
			printf("[!] Error decrypting data: %x", status);
			Cleanup();
			return;
		}
    //SUCCESS!
    return;
}

/*https://stackoverflow.com/questions/30720414/how-to-chain-bcryptencrypt-and-bcryptdecrypt-calls-using-aes-in-gcm-mode*/
void AESGCM::Encrypt(BYTE* nonce, size_t nonceLen, BYTE* data, size_t dataLen){
   //malloc the tag
   tag = (BYTE*)malloc(authTagLengths.dwMaxLength);
   //setup auth 
   BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO auth;
   BCRYPT_INIT_AUTH_MODE_INFO(auth);
   auth.pbNonce = nonce;
   auth.cbNonce = nonceLen;
   auth.pbAuthData = NULL;
   auth.cbAuthData = 0;
   auth.pbTag = tag;
   auth.cbTag = authTagLengths.dwMaxLength;
   auth.pbMacContext = NULL;
   auth.cbMacContext = 0;
   auth.cbAAD = 0;
   auth.dwFlags = 0;

   //get length for output buffer
   NTSTATUS status;
   ULONG pcbResult;
   status = BCryptEncrypt(hKey, data, dataLen, &auth, nonce, nonceLen, NULL, 0, &pcbResult, 0);
   if (!NT_SUCCESS(status)){
    printf("[!] Error getting buffer size!");
    Cleanup();
    return;
   }

   //allocate buffer for ciphertext
   ptBufferSize = pcbResult;
   ciphertext = (BYTE*)malloc(pcbResult);
   if(!ciphertext){
    printf("[!] Error allocating memory!");
    Cleanup();
    return;
   }

   ULONG numBytes = 0;
   //encrypt data
   status = BCryptEncrypt(hKey, data, dataLen, &auth, nonce, nonceLen, ciphertext, pcbResult, &numBytes, 0);
   if(!NT_SUCCESS(status)){
    printf( "[!] Error encrypting: %x", status);
    Cleanup();
    return;
   }
   //success!
   return;
}

void AESGCM::Cleanup(){
    if(hAlg){
        ::BCryptCloseAlgorithmProvider(hAlg,0);
        hAlg = NULL;
    }
    if(hKey){
        ::BCryptDestroyKey(hKey);
        hKey = NULL;
    }
    if(tag){
          ::HeapFree(GetProcessHeap(), 0, tag);
          tag = NULL;
    }
    if(ciphertext){
        ::HeapFree(GetProcessHeap(), 0, tag);
        ciphertext = NULL;
    }
    if(plaintext){
        ::HeapFree(GetProcessHeap(), 0, plaintext);
        plaintext = NULL;
    }
}
