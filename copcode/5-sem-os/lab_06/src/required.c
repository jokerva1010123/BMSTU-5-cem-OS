#include <stdio.h>
#include <windows.h>

#define FALSE 0
#define TRUE 1

const DWORD sleep_time_for_writer = 50;
const DWORD sleep_time_for_reader = 30;

#define READERS_COUNT 5
#define WRITERS_COUNT 3
#define ITERATIONS 5

HANDLE mutex;
HANDLE can_read;
HANDLE can_write;

HANDLE writers[WRITERS_COUNT];
HANDLE readers[READERS_COUNT];

LONG waiting_writers = 0;
LONG waiting_readers = 0;
LONG active_readers = 0;

int active_writer = FALSE;

int value = 0;

void start_read()
{
    InterlockedIncrement(&waiting_readers);

    if (active_writer || (waiting_writers && WaitForSingleObject(can_write, 0) == WAIT_OBJECT_0))
        WaitForSingleObject(can_read, INFINITE);

    WaitForSingleObject(mutex, INFINITE);

    InterlockedDecrement(&waiting_readers);
    InterlockedIncrement(&active_readers);
    SetEvent(can_read);

    ReleaseMutex(mutex);
}

void stop_read()
{
    InterlockedDecrement(&active_readers);

    if (active_readers == 0)
    {
        SetEvent(can_write);
    }
}

void start_write()
{
    InterlockedIncrement(&waiting_writers);
    if (active_writer || active_readers > 0)
        WaitForSingleObject(can_write, INFINITE);

    active_writer = TRUE;
    InterlockedDecrement(&waiting_writers);
    ResetEvent(can_write);
}

void stop_write()
{
    active_writer = FALSE;

    if (waiting_readers)
        SetEvent(can_read);
    else
        SetEvent(can_write);
}

DWORD WINAPI reader(LPVOID lpParams)
{
while (value < WRITERS_COUNT * ITERATIONS)
{
start_read();
printf("Reader #%ld read value %d\n", GetCurrentThreadId(), value);
stop_read();
Sleep(sleep_time_for_reader);
}

return 0;
}

DWORD WINAPI writer(LPVOID lpParams)
{
for (int i = 0; i < ITERATIONS; i++)
{
start_write();
printf("       Writer #%ld wrote value %ld\n", GetCurrentThreadId(), ++value);
stop_write();
Sleep(sleep_time_for_writer);
}

return 0;
}

int main()
{
    int code = EXIT_SUCCESS;

    mutex = CreateMutex(NULL, FALSE, NULL);
    if (mutex == NULL)
    {
        perror("Can't create mutex");
        return EXIT_FAILURE;
    }
    can_read = CreateEvent(NULL, FALSE, TRUE, NULL);
    if (can_read == NULL)
    {
        perror("Can't create event can read");
        return EXIT_FAILURE;
    }
    can_write = CreateEvent(NULL, TRUE, TRUE, NULL);
    if (can_write == NULL)
    {
        perror("Can't create event can write");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < WRITERS_COUNT; i++)
    {
        writers[i] = CreateThread(NULL, 0,  writer, NULL, 0, NULL);
        if (writers[i] == NULL)
        {
            perror("Can't create writer");
            return EXIT_FAILURE;
        }
    }
    for (int i = 0; i < READERS_COUNT; i++)
    {
        readers[i] = CreateThread(NULL, 0, reader, NULL, 0, NULL);
        if (readers[i] == NULL)
        {
            perror("Can't create reader");
            return EXIT_FAILURE;
        }
    }

    WaitForMultipleObjects(WRITERS_COUNT, writers, TRUE, INFINITE);
    WaitForMultipleObjects(READERS_COUNT, readers, TRUE, INFINITE);

    CloseHandle(mutex);
    CloseHandle(can_read);
    CloseHandle(can_write);

    return code;
}
