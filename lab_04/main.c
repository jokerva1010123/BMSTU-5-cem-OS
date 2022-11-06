#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>    // S_IRUSR...
#include <errno.h>
#include <string.h> // strerr
#include <sys/file.h> // flock
#include <unistd.h>
#include <time.h>
#include <stdlib.h> // exit
#include <pthread.h>

void daemonize(const char *cmd)
{
    int i, fd0, fd1, fd2;
    pid_t pid;
    struct rlimit rl;
    struct sigaction sa;

    //1. сбросить маску режима создания файла в значение 0 означает, 
    //что следует (можно) выставить все биты прав
    umask(0);

    // получить максимально возможный номер дескриптора файла (для того чтобы закрыть все дескрипторы до этого
    // номера и не удерживать унаследованные дескрипторы в открытом состоянии)
    // RLIMIT_NOFILE - Определяет значение, на 1 больше максимального количества описателей файлов, возможных для открытыя этим процессом. Попытки исполнить 
    //(open(), pipe(), dup(), и т.п.) для превышения этого лимита приведут к ошибке.
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
    {
        printf("Ошибка вызова getrlimit\n");
        exit(1);
    }
    
    //2. Стать лидером нового сеанса, чтобы утратить управляющий терминал.
    // В результате потомок теряет группу
    // Процесс не должен являться лидером группы - это условие вызова setsid()
    if ((pid = fork()) < 0)
    {
        printf("Ошибка вызова fork\n");
        exit(1);
    } else if (pid != 0) // Завершение родительского
        exit(0);
    // Создание нового сеанса, в котором вызывающий процесс является лидером группы процессов.
    // ID сеанса = ID процесса = ID группы
    if (setsid() == -1)
    {
        printf("Ошибка вызова setsid\n");
        exit(1);
    }

    //обеспечить невозможность обретения управляющего терминала путём игнорирования сигнала SIGHUP
    // sa_handler задает тип действий процесса, связанный с сигналом signum SIG_IGN для игнорирования сигнала 
    // или быть указателем на функцию обработки сигнала.
    sa.sa_handler = SIG_IGN; // SIG_DFL - выполнение стандартных действий, SIG_IGN - игнорирование сигнала
    sigemptyset(&sa.sa_mask); // инициализирует пустой набор сигналов, на который указывает аргумент set. 
    // (sa_mask задает маску сигналов, которые должны блокироваться при обработке сигнала)
    sa.sa_flags = 0;// Если выбран 0, обработчик прерываний запускается при разрешенных посторонних прерываниях
    // и возвращает значение через сигнальные функции обработчика.

    // SIGHUP - номер сигнала, обработчик которого устанавливается адрес структуры sa - новый обработчик сигнала
    // NULL => старый обработчик сигнала никуда не запишется
    // Функция sigaction позволяет проверить действие, связанное с определенным сигналом, изменить его или выполнить обе эти операции. 
    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        printf("Невозможно игнорировать сигнал SIGHUP\n");
        exit(1);
    }

    // Назначить корневую директорию текущей директорией, чтобы можно было отмонтировать файловую систему, на которой демон
    if (chdir("/") < 0)
    {
        printf("Ошибка вызова chdir\n");
        exit(1);
    }

    // Закрытие всех файловых дескрипторов
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024; // в Linux по умолчанию установлен лимит на 1024 одновременно открытых файлов
    for (i = 0; i < rl.rlim_max; ++i)
        close(i); // Освободить дескриптор файла, после чего он будет доступен при вызовах open()

    // присоединить файловые дескрипторы 0,1,2 
    // /dev/null - "пустое устройство", специальный файл
    // RDWR - на чтение RD=read и запись WR=write
    // O_RDWR - Файл открывается для чтения и для записи.
    fd0 = open("/dev/null", O_RDWR); // "dev/null" специальный файл в системах класса UNIX, представляющий собой так называемое «пустое устройство»
    //dup(int oldfd) - создаёт копию файлового дескриптора oldfd, использует самый маленький свободный номер дескриптора.
    //Файл дескриптор 0 называется STDIN и ассоциируется с вводом данных у приложения
    fd1 = dup(0); // stdout, dup копирует файловый дескриптор
    fd2 = dup(0); // stdf

    // Инициализация файла журнала
    // cmd - строка, которая добавляется к каждому сообщению, обычно - название программы
    // LOG_CONS - флаг, управляющий работой openlog() & syslog()
    // LOG_CONS 0x02 /* log on the console if errors in sending */
    // LOG_DAEMON - тип программы, записывающей сообщения, другие системные демоны
    openlog(cmd, LOG_CONS, LOG_DAEMON);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2)
    {
        printf("Некорректные файловые дескрипторы: %d %d %d\n", fd0, fd1, fd2);
        exit(1);
    }
    syslog(LOG_NOTICE, "Демон создан");

}

#define LOCKFNAME "/var/run/oslabd"
#define LOCKFMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)


//запуск единственного экземпляра демона
// Для обеспечения работы демона в единственном экземпляре. 
// Осуществляется блокировкой файла в области ядра ОС, пока один процесс "держит" блокировку, другой заблокировать файл не сможет
int already_running()
{
    int fd;
    char buf[16];

    syslog(LOG_NOTICE, "Обеспечение работы демона в ед. экземпляре");

    if ((fd = open(LOCKFNAME, O_RDWR | O_CREAT, LOCKFMODE)) < 0)
    {
        syslog(LOG_ERR, "Невозможно открыть файл %s %s", LOCKFNAME, strerror(errno));
        exit(1);
    }
    syslog(LOG_NOTICE, "Файл открыт");

    // LOCK_EX - эксклюзивная блокировка
    // LOCK_NB - неблокируемый запрос
    // каждая копия демона будет пытаться создать файл fd
    // flock - установить или снять advisory(мягкую) блокировку на открытый файл
    // LOCK_EX - Установить эксклюзивную блокировку. Только один процесс может держать эксклюзивную блокировку файла.  
    // Вызов flock() может быть блокирован, если несовместимый тип блокировки уже удерживается другим процессом. Чтобы выполнить неблокирующий запрос, включите LOCK_NB
    if (flock(fd, LOCK_EX | LOCK_NB) != 0)
        // EWOULDBLOCK - Файл блокирован и был выбран флаг LOCK_NB .
        if (errno == EWOULDBLOCK)
        {
            syslog(LOG_ERR, "Демон уже запущен - %s", strerror(errno));
            close(fd);
            return -1;
        }

    ftruncate(fd, 0);
    sprintf(buf, "%d", getpid());
    write(fd, buf, strlen(buf) + 1);
    syslog(LOG_NOTICE, "Закончена запись идентификатора процесса в файл");
    return 0;
}

void daemon_action()
{
    time_t raw_time;
    struct tm *tm;

    time(&raw_time);
    tm = localtime(&raw_time);

    syslog(LOG_INFO, "Работа демона, время - %s", asctime(tm));
}

sigset_t mask;

void *thr_fn(void *arg)
{
    int err, signo;

    for (;;)
    {
        err = sigwait(&mask, &signo);
        if (err != 0)
        {
            syslog(LOG_ERR, "Ошибка вызова sigwait()");
            exit(1);
        }

        switch (signo)
        {
            case SIGHUP:
                syslog(LOG_INFO, "Получен сигнал SIGHUP");
                break;
            case SIGTERM:
                syslog(LOG_INFO, "Завершена работа демона");
                exit(0);
            default:
                syslog(LOG_INFO, "Получен непредвиденный сигнал");
        }
    }
}

int main(int argc, char *argv[])
{
    pthread_t tid;
    char *cmd;
    struct sigaction sa;

    // strrchr(const char * str, int symbol) - Функция ищет последнее вхождение символа symbol в строку string
    if ((cmd = strrchr(argv[0], '/')) == 0)
        cmd = argv[0];
    else
        cmd++;
    // cmd теперь == main - это команда и это имя демона
    
    daemonize(cmd);  //Перейти в режим демона.
    //Убедиться, что ранее не была запущена другая копия демона.
    if (already_running() < 0)
        exit(1);

    //Восстановить действие по умолчанию для сигнала SIGHUP и заблокировать все сигналы.
    // Использyется с сигнальной функцией на месте yказателя на обpаботчик пpеpывания для выбоpа стандаpтного обpаботчика пpеpывания опеpационной системы.
    sa.sa_handler = SIG_DFL;  // Обработчик по умолчанию
    sigemptyset(&sa.sa_mask); //sigemptyset инициализирует набор сигналов, указанный в set, и "очищает" его от всех сигналов.

    sa.sa_flags = 0;

    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        syslog(LOG_ERR, "Ошибка при установке обработчика по умолчанию для SIGHUP");
        exit(1);
    }

    sigfillset(&mask); // Все сигналы в маске
    // Новый поток будет использовать маску mask, в ней блокируются все сигналы, выставленные в маске oldmask - NULL =>
    // все сигналы будут обрабатываться новым потоком
    if (pthread_sigmask(SIG_BLOCK, &mask, NULL) != 0) //pthread_sigmask - проверить и изменить маску заблокированных сигналов
    {
        syslog(LOG_ERR, "Ошибка при блокировке сигналов");
        exit(1);
    }
    //Создать поток для обработки SIGHUP и SIGTERM.
    // tid - id нового потока, NULL - атрибуты по умолчанию для нового потока
    // thr_fn - функция, которую выполняет поток, 0 - аргумент, передаваемый функции
    if (pthread_create(&tid, NULL, thr_fn, 0) != 0)
    {
        syslog(LOG_ERR, "Ошибка при создании потока для обработки сигналов");
        exit(1);
    }

    while (1)
    {
        daemon_action();
        sleep(1);
    }

    return 0;
}