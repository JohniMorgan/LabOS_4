#include <iostream>
#include <windows.h>
#include <string>
#include <stdio.h>
#include <time.h>


using namespace std;

const int page_number = 12;

HANDLE* reader_semaphore = new HANDLE[page_number];
HANDLE* writer_semaphore = new HANDLE[page_number];


int NeededMemory();
void getSemaphoresPair(int n);

void ExecuteReaderProcess();


int main()
{
    srand(time(NULL));
    ExecuteReaderProcess();

    return 0;
}

void getSemaphoresPair(int n) {
    reader_semaphore[n] = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE,
        (to_string(n) + " reader semaphore").c_str()); //Получение семафора по его имени для читателей
    writer_semaphore[n] = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE,
        (to_string(n) + " writer semaphore").c_str()); //Получение семафора по его имени для писателей
}

int NeededMemory() {
    SYSTEM_INFO info;
    GetSystemInfo(&info); // Для запуска на любой системе, будем опираться на их размеры страниц
    int cnt = page_number * info.dwPageSize;

    return cnt;
}

void ExecuteReaderProcess() {
    int memory_size = NeededMemory();

    LPWSTR area_alias = new wchar_t[] {L"DataArea"};

    HANDLE mapped_descriptor;
    LPVOID address;

    mapped_descriptor = OpenFileMapping(GENERIC_WRITE, FALSE, area_alias); //Открываем уже спроецированный менеджером файл
    address = MapViewOfFile(mapped_descriptor, FILE_MAP_WRITE, 0, 0, memory_size); // Получаем отображение этого файла

    // Открываем файлы для журнала логов и для обработки в таблицах
    FILE* log, * cvs;
    fopen_s(&log, ("D:\\newdir\\OSLab4\\reader_log\\reader_log_" + to_string(GetCurrentProcessId()) + ".txt").c_str(), "w");
    fopen_s(&cvs, ("D:\\newdir\\OSLab4\\reader_cvs\\reader_log_" + to_string(GetCurrentProcessId()) + ".txt").c_str(), "w");
    // Получаем все семафоры для всех страниц
    for (int i = 0; i < page_number; i++) {
        getSemaphoresPair(i);
    }

    int* data;

    SYSTEM_INFO info;
    GetSystemInfo(&info);
    DWORD start = GetTickCount();

    while (GetTickCount() - start < 20000) {
        //----------------------------
        fprintf(log, "Proccess: %d\t State: WAITING \t Time: %d \n", GetCurrentProcessId(), GetTickCount());
        fprintf(cvs, "%d;%d\n", GetTickCount(), 0);
        DWORD PageSemaphore = WaitForMultipleObjects(page_number, reader_semaphore, FALSE, INFINITE);
        //В этом блоке процесс находится в состоянии "Ожидание", о чём делается соответствующая запись
        //----------------------------
        //Когда семафор освобождается, поток продолжает своё выполнение
        //----------------------------
        //Происходит вход в состояние "Чтение", о чём делается соответствующая запись
        fprintf(log, "Proccess: %d\t State: READ \t Time: %d \nData:", GetCurrentProcessId(), GetTickCount());
        fprintf(cvs, "%d;%d\n", GetTickCount(), 1);

        data = (int*)((long long)address + (PageSemaphore * info.dwPageSize));
        fprintf(log, " %d\n", *data);
        Sleep(500 + rand() % 1000); //Симулируем задержку чтения между 0.5 и 1.5 секунды
        //----------------------------
        //Блок чтения завершился, необходимо освободить 
        //захваченный семафор писателя соответствующей страницы
        //----------------------------
        FILE* page_log;
        fopen_s(&page_log, ("D:\\newdir\\OSLab4\\pages_statement\\page_" + to_string(PageSemaphore) + ".txt").c_str(), "a");
        fprintf(page_log, "%d;%d\n", GetTickCount(), 2); //Состояние "Прочитаная страница"
        ReleaseSemaphore(writer_semaphore[PageSemaphore], 1, NULL); //Освобождение семафора писателя страницы
        fprintf(page_log, "%d;%d\n", GetTickCount(), 0); //Состояние страницы "Освобождена"
        //----------------------------
        fclose(page_log);
        fprintf(log, "Proccess: %d\t State: RELEASED \t Time: %d \n", GetCurrentProcessId(), GetTickCount());
        fprintf(cvs, "%d;%d\n", GetTickCount(), 2);
    }

    fclose(log);
    fclose(cvs);
}