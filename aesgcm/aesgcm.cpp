#include "aes_gcm.h"


AESGCM:: ~AESGCM(){
    Cleanup();
}

// Freebie: initialize AES class
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

    status = BCryptDecrypt(hKey, data, dataLen, NULL, nonce, nonceLen, plaintext, pcbResult, &pcbResult, 0);
		if(!NT_SUCCESS(status)){
			printf("[!] Error decrypting data: %x", status);
			Cleanup();
			return;
		}
}

void AESGCM::Encrypt(BYTE* nonce, size_t nonceLen, BYTE* data, size_t dataLen){
   //get length for output buffer
   NTSTATUS status;

   ULONG pcbResult;
   status = BCryptEncrypt(hKey, data, dataLen, NULL, nonce, nonceLen, NULL, 0, &pcbResult, 0);
   if (!NT_SUCCESS(status)){
    printf("[!] Error getting buffer size!");
    Cleanup();
    return;
   }
    
   //allocate buffer for ciphertext
   ciphertext = (BYTE*)malloc(pcbResult);
   if(!ciphertext){
    printf("[!] Error with malloc!");
    Cleanup();
    return;
   }

   //encrypt data
   ULONG bytesDone;
   status = BCryptEncrypt(hKey, data, dataLen, NULL, nonce, nonceLen, ciphertext, pcbResult, &bytesDone, 0);
   if(!NT_SUCCESS(status)){
    printf( "[!] Error encrypting: %x", status);
    Cleanup();
    return;
   }
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