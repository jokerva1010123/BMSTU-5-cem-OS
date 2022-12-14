#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <time.h>
#include <wait.h>
#include <unistd.h>
#include <sys/stat.h>

#include "buffer.h"
#include "constants.h"
#include "producer.h"
#include "consumer.h"

// Семафоры:
// 0 - семафор потребителя, fullCount, семафор-счетчик заполненности буфера. [fullCount, ...]
// 1 - семафор производителя, emptyCount, семафор-счетчик свободности буфера. [..., emptyCount]
// 2 - бинарный семафор, useQueue. Показывает, находится ли очередь(массив) в обработке производителя или потребителя
// struct sembuf name { sem_id, sem_op, sem_flags}. применяет опрерацию sem_op к семафору sem_id

int main(void)
{
	// shmid, sem_descr объявляем ID для разделяемого сегмента, массива семафоров
	setbuf(stdout, NULL);

	// Значение IPC_PRIVATE указывает, что к разделяемой
	// памяти нельзя получить доступ другим процессам.
	// S_IRUSR	Владелец может читать.
	// S_IWUSR	Владелец может писать.
	// S_IRGRP	Группа может читать.
	// S_IROTH	Остальные могут читать.
	int perms = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;

	// shmget - создает новый разделяемый сегмент в локальном пр-ве процеса(IPC_PRIVATE), 
	// длинной, с правами доступа
    int shmid = shmget(IPC_PRIVATE, sizeof(buffer_t) + sizeof(char),
                IPC_CREAT | perms);
    if (shmid == -1) 
	{
        perror("Failed to create shared memory!");
        return -1;
    }

	// Функция shmat() возвращает указатель на сегмент shmaddr (второй аргумент) равно NULL,
	// то система выбирает подходящий (неиспользуемый) адрес для подключения сегмента.
	// Присоединяем разделяемый сегмент памяти, получив его начальный адрес
    buffer_t *buffer;
    if ((buffer = (buffer_t*)shmat(shmid, 0, 0)) == (buffer_t*)-1) 
	{
        perror("Shmat failed!");
        return -1;
    }

    char *ch = (char *)(buffer + 1);
    *ch = 'a' - 1;
    if (init_buffer(buffer) == -1) 
	{
        perror("Failed to init buffer!");
        return -1;
    }

	// IPC_PRIVATE - ключ, который показывает, что
	// набор семафоров могут использовать только процессы,
	// порожденные процессом, создавшим семафор.
	// Создание семафоров (3 семафора) в локальном пр-ве процесса, с правами доступа 
    int sem_descr = semget(IPC_PRIVATE, 3, IPC_CREAT | perms);
	if (sem_descr == -1)
	{
		perror("Failed to create semaphores!");
		return -1;
	}
	
	// sem[2] = 1
	if (semctl(sem_descr, BIN_SEM, SETVAL, 1) == -1)
	{
		perror("Can't set control bin_sem!");
		return -1;
	}
	// Изначально все ячейки буфера пусты и, таким образом, количество заполненных ячеек равно 0
    if (semctl(sem_descr, BUF_FULL, SETVAL, 0) == -1)
	{
		perror("Can't set control buf_full semaphore!");
		return -1;
	}
	// sem[1] = N
	// Все ячейки буфера изначально пусты
    if (semctl(sem_descr, BUF_EMPTY, SETVAL, N) == -1)
	{
		perror("Can't set control buf_empty semaphor.");
		return -1;
	}

    for (int i = 0; i < PRODUCER_NUM; i++)
		create_producer(buffer, ch, sem_descr, i + 1);

	for (int i = 0; i < CONSUMER_NUM; i++)
		create_consumer(buffer, sem_descr, i + 1);

    for (size_t i = 0; i < PRODUCER_NUM + CONSUMER_NUM; i++)
    {
        int status;
        if (wait(&status) == -1) 
		{
            perror("Something wrong with children waiting!");
            return -1;
        }
		
        if (!WIFEXITED(status))
            printf("One of children terminated abnormally!\n");
    }

    printf("\e[1;32mOK\e[0m\n");

	// IPC_RMID используется для пометки сегмента как удаленного.
	if (shmctl(shmid, IPC_RMID, NULL))
	{
		perror("Failed to mark the segment to be destroyed!");
		return -1;
	}

	if (shmdt((void*)buffer) == -1) // отсоединяем разделяемый сегмент памяти
	{
		perror("Failed to detachess the segment!");
		return -1;
	}

	if (semctl(sem_descr, 0, IPC_RMID, 0) == -1)
	{
		perror("Failed to delete semaphores!");
		return -1;
	}

	return 0;
}
