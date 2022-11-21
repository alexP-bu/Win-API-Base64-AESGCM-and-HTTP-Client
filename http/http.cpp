#include <windows.h>
#include <string>
#include <iostream>
#include <winhttp.h>


std::string makeHttpRequest(std::string fqdn, int port, std::string uri, bool useTLS){
    std::string result;
    //open the client
    HINTERNET hInternet;
    hInternet = WinHttpOpen(NULL, WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if(!hInternet){
        printf("[!] Failed to open session");
        return result;
    }
    //connect to the server
    std::wstring wfqdn = std::wstring(fqdn.begin(), fqdn.end());
    LPCWSTR domain = wfqdn.c_str();
    HINTERNET hConnect;
    hConnect = WinHttpConnect(hInternet, domain, port, 0);
    if(!hConnect){
        printf("[!] Failed to connect");
        return result;
    }
    //create request
    HINTERNET hOpenReq;
    std::wstring wuri = std::wstring(uri.begin(), uri.end());
    LPCWSTR uriP = wuri.c_str();
    hOpenReq = WinHttpOpenRequest(
        hConnect, 
        L"GET", 
        uriP, 
        NULL, 
        WINHTTP_NO_REFERER, 
        WINHTTP_DEFAULT_ACCEPT_TYPES, 
        (useTLS ? WINHTTP_FLAG_SECURE : 0));
    if(!hOpenReq){
        printf("[!] Failed to open request");
        return result;
    }

    //send request
    if(!WinHttpSendRequest(hOpenReq, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)){
        printf("[!] Failed to send request");
        return result;
    }
    //recieve response
    if(!WinHttpReceiveResponse(hOpenReq, NULL)){
        printf("[!] Failed to recieve response");
        return result;
    }
    //query data available
    LPDWORD data;
    char* lpBuffer[4096];
    while(WinHttpQueryDataAvailable(hOpenReq, data)){
        DWORD dwNumberOfBytesRead;
        if(!WinHttpReadData(hOpenReq, lpBuffer, *data, &dwNumberOfBytesRead)){
            printf("[!] Failed to read data");
        }
        result.append(std::string myString(data, size), dwNumberOfBytesRead);
    }
    //read data


    printf("TEST");
    free(hInternet);
    free(hConnect);
    free(hOpenReq);
    return result;
}

int main(int argc,  char* argv[]){
    if(argc !=5){
        std::cout << "Incorrect number of arguments: you need 4 positional arguments" << std::endl;
        return 0;
    }

    std::string fqdn = std::string(argv[1]);
    int port = std::stoi( argv[2] );

    std::string uri = std::string(argv[3]);
    int  useTLS =std::stoi(argv[4]);
    bool tls;
    
    if (useTLS == 1){
        tls = true;
    } else if (useTLS == 0){
        tls = false;

    } else{
        std::cout << "bad value for useTls" << std::endl;
        return 0;
    }
     std::cout << makeHttpRequest(fqdn,  port, uri, tls) << std::endl;
    return 0;
    
}