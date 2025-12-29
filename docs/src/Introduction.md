# NachOS

Documentation du noyau et des bibliothèques utilisateur de NachOS Team-A.

# Table des matières
- [Introduction](./Introduction.md)
- [Table des matières](./SUMMARY.md)
- [Syscalls List](./syscalls/SUMMARY.md)
    - [Syscall Overview](syscalls/Syscalls.md)
    - [Console](./syscalls/console/SUMMARY.md)
        - [Console Overview](syscalls/console/Console.md)
        - [PutChar](./syscalls/console/PutChar.md)
        - [GetChar](./syscalls/console/GetChar.md)
        - [PutString](./syscalls/console/PutString.md)
        - [GetString](./syscalls/console/GetString.md)
        - [PutInt](./syscalls/console/PutInt.md)
        - [GetInt](./syscalls/console/GetInt.md)
    - [Sémaphores](./syscalls/sync/SUMMARY.md)
        - [Sémaphores Overview](syscalls/sync/Sync.md)
        - [SemInit](./syscalls/sync/SemInit.md)
        - [SemP](./syscalls/sync/SemP.md)
        - [SemV](./syscalls/sync/SemV.md)
        - [SemDestroy](./syscalls/sync/SemDestroy.md)
        - [SetMaxSemForProcess](./syscalls/sync/SetMaxSemForProcess.md)
    - [Error Handling](./syscalls/errors.md)
    - [Errno Codes](./syscalls/errno.md)