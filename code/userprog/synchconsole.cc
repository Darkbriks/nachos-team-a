#include "copyright.h"
#include "system.h"
#include "synchconsole.h"
#include "synch.h"

static Semaphore *readAvail;
static Semaphore *writeDone;

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

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
    console->PutChar(ch);
    writeDone->P(); 
}

char SynchConsole::SynchGetChar()
{
    readAvail->P();
    return console->GetChar();
}

int SynchConsole::SynchPutString(const char s[], unsigned int n)
{
    for (unsigned i = 0; i < n ; i++) {
        if (s[i] == '\0') { return i; }
        SynchPutChar(s[i]);
    }
    return n;
}

void SynchConsole::SynchGetString(char *s, int n)
{
    // ...
}
