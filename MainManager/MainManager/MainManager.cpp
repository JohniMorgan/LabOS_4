#include <iostream>
#include <windows.h>
#include <string>
#include <stdio.h>

using namespace std;

const int page_number = 12; // = 0 + 3 + 0 + 5 + 0 + 4

void InitReaderSemaphores(int num); //Инициализация семафоров-чтецов
void InitWriterSemaphores(int num); //Инициализация семафоров-писателей
void InitReaders(int n); //Инициализация самих читателей
void InitWriters(int n); //Инициализация самих писателей

void InitProgramm();
void InitSemaphores();
void InitShareFile(); //Инициализация общего проецируемого файла

int NeededMemory(); //Рассчёт размера ПАМЯТИ для нужного количества страниц


HANDLE *reader_semaphore = new HANDLE[page_number]; //По семафору на каждую страницу для читателей
HANDLE *writer_semaphore = new HANDLE[page_number]; //По семафору на каждую страницу для писателей
HANDLE *proceses = new HANDLE[page_number * 2]; //12 читателей + 12 писателей

int main()
{
    SetLastError(0);

    InitShareFile();
    InitSemaphores();
    InitProgramm();
    cout << "start\n";
    WaitForMultipleObjects(page_number * 2, proceses, TRUE, INFINITY);
    cout << "end";
    return 0;
}


int NeededMemory() {
    SYSTEM_INFO info;
    GetSystemInfo(&info); // Для запуска на любой системе, будем опираться на их размеры страниц
    int cnt = page_number * info.dwPageSize;

    return cnt;
}

void InitShareFile() {
    HANDLE file_descriptor, virt_memory_descriptor;
    LPVOID address;

    //Путь до отображаемого файла разделяемой памяти
    LPCSTR file_path = new char[100] {"D:\\newdir\\OSLab4\\data.txt"};
    //Псевдоним для страниц в памяти
    LPCSTR area_name = new char[20] {"DataArea"};
    // Открытие файла для последующего отображения
    int size = NeededMemory();
    file_descriptor = CreateFileA(file_path, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,
            NULL);
    // Создание дескриптора объекта отображения
    virt_memory_descriptor = CreateFileMappingA(file_descriptor, NULL, PAGE_READWRITE, 0, size, area_name);
    // Отображение файла в виртуальную память
    address = MapViewOfFile(virt_memory_descriptor, FILE_MAP_ALL_ACCESS, 0, 0, (SIZE_T)size);
    // Блокировка страниц в Оперативной памяти
    VirtualLock(address, (SIZE_T)size);
}

void InitReaderSemaphores(int num) {
    string name; // Именование семафора
    name = to_string(num) + " reader semaphore";
    //Создание семафора для читателя n-й страницы в состоянии "заблокировано" 
    reader_semaphore[num] = CreateSemaphoreA(NULL, 0, 1, name.c_str());
}

void InitWriterSemaphores(int num) {
    string name; //Именование семафора
    name = to_string(num) + " writer semaphore";
    //Создание семафора для писателя n-й страницы в состоянии "свободно"
    writer_semaphore[num] = CreateSemaphoreA(NULL, 1, 1, name.c_str());
}

void InitReaders(int n) {
    LPSTARTUPINFO start_info = new STARTUPINFO[page_number];
    LPPROCESS_INFORMATION proc_info = new PROCESS_INFORMATION[page_number];
    LPWSTR process_path = new wchar_t[] {L"C:\\Users\\Evgeny\\source\\repos\\LabWork_4\\ReaderApplication\\Debug\\ReaderApplication.exe"};
    int aboba = GetLastError();
        ZeroMemory(&start_info[n], sizeof(start_info[n]));

    CreateProcess(process_path, NULL, NULL, NULL, TRUE,
        0, NULL, NULL, &start_info[n], &proc_info[n]);
    aboba = GetLastError();
    proceses[n] = proc_info[n].hProcess;
}

void InitWriters(int n) {
    LPSTARTUPINFO start_info = new STARTUPINFO[page_number];
    LPPROCESS_INFORMATION proc_info = new PROCESS_INFORMATION[page_number];
    LPWSTR process_path = new wchar_t[] {L"C:\\Users\\Evgeny\\source\\repos\\LabWork_4\\WriterApplication\\Debug\\WriterApplication.exe"};

    ZeroMemory(&start_info[n], sizeof(start_info[n]));

    CreateProcess(process_path, NULL, NULL, NULL, TRUE,
        0, NULL, NULL, &start_info[n], &proc_info[n]);

    proceses[n + page_number] = proc_info[n].hProcess;
}

void InitProgramm() {
    for (int i = 0; i < page_number; i++) {
        InitReaders(i);
        InitWriters(i);
    }
}

void InitSemaphores() {
    for (int i = 0; i < page_number; i++) {
        InitReaderSemaphores(i);
        InitWriterSemaphores(i);
    }
}