//
//  main.cpp
//  HasanThread
//
//  Created by Hasan Qureshi on 11/19/19.
//  Copyright © 2019 Hasan Qureshi. All rights reserved.
//

#include <iostream>
#include <iterator>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
#include <list>
#include <stack>
#include <queue>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <fstream>

using namespace std;
sem_t semaphore;//uninitialized global semaphore

void *compress(void *args);//handle encrypt
void *decompress(void *args);//final decrypt
string outputDec(list<string> decompression);
class strEncrypt{
public:
    strEncrypt(string file){
        fileString = file;
    }
    string mainChars;
    string fileString;
    string textLine;
    string decomp;
    int counter1;
    pthread_t threads;

    void decomp1();
    bool operator>(const strEncrypt &linE) const;
};

bool strEncrypt::operator>(const strEncrypt &linE) const{

    bool endof = mainChars=="<EOL>";
    bool checkendOF = linE.mainChars== "<EOL>";
    bool ASCIIcomp = (int)mainChars[0] < (int)linE.mainChars[0];
    bool freq = counter1 > linE.counter1;
    bool isEqual = counter1== linE.counter1;

    return  freq || (isEqual && ((endof && !checkendOF) || ASCIIcomp));
}

void strEncrypt::decomp1(){

    string tstring = fileString;
    counter1 = 0;

    int charFound = fileString.find("\0");
    if(charFound>0){
        tstring = fileString.erase(fileString.length()-1,1);
    }
    int endOF = fileString.find("<EOL>");
    if(endOF >= 0){

        tstring.erase(0,6);
        mainChars = '\n';
    }
    else{
        tstring.erase(0,2);
        mainChars=fileString[0];
    }

    textLine = tstring;
    for (int c =0; c<textLine.length();c++){
        if(textLine[c]=='1'){
            counter1++;
        }
    }
}

string outputDec(list<string> decompression)
{
    string temp = "";
    list<string>::iterator counter;
    for (counter = decompression.begin(); counter != decompression.end(); ++counter)
    {
        temp += *counter;
    }
    return temp;
}

void *compress(void *args){
    //compress
    //ENTER CRITICAL SECTION
    sem_wait(&semaphore);
    strEncrypt *localString=(strEncrypt *)args;
    localString -> decomp1();
    sem_post(&semaphore);
    return NULL;
}

void *decompress(void *args){
    //enter critical section
    sem_wait(&semaphore);
    list<strEncrypt>::iterator index;
    list<strEncrypt> *mainChar = (list<strEncrypt> *)args;
    mainChar -> sort(greater<strEncrypt>());

    mainChar -> reverse();
    for(index=mainChar -> begin(); index!= mainChar -> end(); index++){
        cout<< index -> mainChars <<" Binary code = "<< index -> textLine << endl;
    }
    cout<<"Decompressed file contents: "<<endl;
    list<string> decompressed;
    string decLine="";
    
    for(index=mainChar->begin(); index!=mainChar -> end(); index++)
    {
        queue<char> temporaryString;
        for (int i= 0; i<index -> textLine.length();i++)
        {
            temporaryString.push(index -> textLine[i]);
        }
        int localIterate=0;
        while(!temporaryString.empty()){
            
            char newIndex=temporaryString.front();
            if(newIndex=='1'){
                list<string>::iterator localMainCharIndex = decompressed.begin();
                advance(localMainCharIndex,localIterate);
                decompressed.insert(localMainCharIndex, 1, index -> mainChars);//CHANGE mainChar
            }
            temporaryString.pop();
            localIterate++;
        }
    }
    cout<< outputDec(decompressed)<<endl;
    sem_post(&semaphore);
    return NULL;
}
int main(int argc, char *argv[]){
    pthread_t mainThread;
    //p_tid threadID;
    list<strEncrypt> mainChar;

    sem_init(&semaphore,0,1);
    string textLine;
    //string testFile(argv[1]);
    int threadCheck;
    
        while(getline(cin,textLine)){
            mainChar.push_back(strEncrypt(textLine));
            threadCheck=pthread_create(&mainChar.back().threads,NULL,compress,(void *)&mainChar.back());

            if(threadCheck){
                cerr<<"THREAD NOT CREATED "<<threadCheck<<endl;
            break;
            }
        }

    list<strEncrypt>::iterator index;//iter
    
    for(index=mainChar.begin();index!=mainChar.end();index++){
        pthread_join(index -> threads,NULL);
    }

    threadCheck=pthread_create(&mainThread,NULL,decompress,(void *)&mainChar);
    pthread_join(mainThread,NULL);

    sem_destroy(&semaphore);// end of thread

    return 0;
}
