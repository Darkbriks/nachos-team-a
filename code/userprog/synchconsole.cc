#include "copyright.h"
#include "system.h"
#include "synchconsole.h"
#include "synch.h"



Semaphore* SynchConsole::readAvail = new Semaphore("read avail", 0);
Semaphore* SynchConsole::writeDone = new Semaphore("write done", 0);
Semaphore* SynchConsole::IO_Lock = new Semaphore("IO_Lock", 1);

Semaphore* SynchConsole::writeAvail = new Semaphore("write avail", 1);

void SynchConsole::freeAllStatic(){
    delete SynchConsole::readAvail;
    delete SynchConsole::writeDone;
    delete SynchConsole::IO_Lock;
    delete SynchConsole::writeAvail;

    SynchConsole::readAvail = nullptr;
    SynchConsole::writeDone = nullptr;
    SynchConsole::IO_Lock = nullptr;
    SynchConsole::writeAvail = nullptr;
}


SynchConsole::SynchConsole(char *readFile, char *writeFile)
{
    console = new Console(readFile, writeFile, ReadAvail, WriteDone, 0);
}

SynchConsole::~SynchConsole()
{
    delete console;
}

void SynchConsole::ReadAvail(int arg) { 
    readAvail->V();
    IO_Lock->V();
}

void SynchConsole::WriteDone(int arg) { 
    writeDone->V();
    writeAvail->V();
    IO_Lock->V();
}

void SynchConsole::SynchPutChar(const char ch)
{

    IO_Lock->P();
    writeAvail->P();
    console->PutChar(ch);
    writeDone->P(); 
}

char SynchConsole::SynchGetChar()
{
    IO_Lock->P();
    readAvail->P();
    char result = console->GetChar();
    return result;
}

int SynchConsole::SynchPutString(const char s[], unsigned int n)
{
    for (unsigned i = 0; i < n ; i++) {
        if (s[i] == '\0') { return i; }
        SynchPutChar(s[i]);
    }
    return n;
}

int SynchConsole::SynchGetString(char *s, int n)
{
    int i = 0;
    char ch;
    while (i < n - 1 && ( (ch = SynchGetChar()) != EOF ) ) {
        s[i++] = ch;
        if (ch == '\n'){
            break;
        }
    }
    s[i] = '\0';
    return i-1;
}
