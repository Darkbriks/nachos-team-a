#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "addrspace.h"
#include "copyright.h"
#include "utility.h"
#include "console.h"

class SynchConsole {
    public:
        static class Semaphore* IO_Lock;
        SynchConsole(char *readFile, char *writeFile);
        // initialize the hardware console device
        ~SynchConsole(); // clean up console emulation
        void SynchPutChar(const char ch); // Unix putchar(3S)
        char SynchGetChar(); // Unix getchar(3S)
        /**
         * @brief Write a String on this Synchronized console
         *        Don't count \0 in n. It's implicit
         *
         * @code int res = SynchPutString("aze\0coucou", 10); assert(res == 3);
         * @param s  The string to print on this console
         * @param n  Number of bytes to write. We will write min(n, first \0 in s).
         * @return The number of bytes effectively write on the console. We don't count \0
         */
        int SynchPutString(const char s[], unsigned int n); //Unix put
                                                            
        /**
         * @brief  Read a String from this synchronized console
         *
         * @param s The buffer to put the string previously allocated by caller
         * @param n The number of bytes we want to read
         * @return The real number of bytes read
         */
        int SynchGetString(char *s, int n); // Unix fgets(3S)
                                            
        /**
         * @brief This function is called only by Cleanup at the end of the program
         */
        static void freeAllStatic();

    private:
        Console *console;
};

#endif // SYNCHCONSOLE_H
