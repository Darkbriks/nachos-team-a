#include "copyright.h"
#include "system.h"
#include "synchconsole.h"
#include "synch.h"

static Semaphore *readAvail;
static Semaphore *writeDone;

Semaphore *SynchConsole::IO_Lock = new Semaphore("a", 1);

void SynchConsole::freeAllStatic(){
    delete IO_Lock;
}

static void ReadAvail(int arg) { 
    readAvail->V();
    SynchConsole::IO_Lock->V();
}

static void WriteDone(int arg) { 
    writeDone->V();
    SynchConsole::IO_Lock->V();
}


SynchConsole::SynchConsole(char *readFile, char *writeFile)
{
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    console = new Console(readFile, writeFile, ReadAvail, WriteDone, 0);
}

SynchConsole::~SynchConsole()
{
    delete console;
    delete writeDone;
    delete readAvail;
}

void SynchConsole::SynchPutChar(const char ch)
{

    IO_Lock->P();
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
