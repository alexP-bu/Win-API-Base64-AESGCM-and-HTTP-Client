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
    std::wstring stemp = std::wstring(fqdn.begin(), fqdn.end());
    LPCWSTR domain = stemp.c_str();
    HINTERNET hConnect;
    hConnect = WinHttpConnect(hInternet, domain, port, 0);
    if(!hConnect){
        printf("[!] Failed to connect");
        return result;
    }
    //create request
    
    free(hInternet);
    free(hConnect);
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