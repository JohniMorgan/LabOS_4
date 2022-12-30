#pragma comment(lib, "winmm.lib")
#include <iostream>
#include <windows.h>
#include <string>

using namespace std;

BOOL conection = FALSE;
OVERLAPPED overlapped_struct;
HANDLE ev;
HANDLE pipe;

void WINAPI callback(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {
	cout << "Message successfull received:\n";
}

void cleanStream() {
	cin.clear();
	if (char(cin.peek()) == '\n') cin.ignore();
}

int InputAsNumber() {
	string user;
	int in = -1;
	bool flag = true;
	do {
		cout << ">> ";
		cleanStream();
		getline(cin, user);
		try {
			in = stoi(user);
			flag = true;
		}
		catch (const std::invalid_argument) {
			flag = false;
			cout << "Invalid format of command! Expected natural number" << endl;
		}

	} while (!flag);
	return in;
}

void GetMsg();
void Disconnect();

int main()
{
	ev = CreateEvent(NULL, FALSE, FALSE, NULL);
	pipe = CreateFile(L"\\\\.\\pipe\\superpipe", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);
	overlapped_struct = OVERLAPPED();
	CHAR msg[64];
	int user;
	if (ev != INVALID_HANDLE_VALUE && pipe != INVALID_HANDLE_VALUE) {
		do {
			cout << "1 - Recieve message" << endl;
			cout << "2 - Disconnect named pipe chanell" << endl;
			cout << "0 - Exit" << endl;
			user = InputAsNumber();
			if (user == 1) {
				overlapped_struct.hEvent = ev;
				conection = ReadFileEx(pipe, (LPVOID)msg, 64, &overlapped_struct,
					callback);
				SleepEx(-1, TRUE);
				if (conection) {
					cout << msg << endl;
				}
				else
					cout << "Reading failed" << endl;
			}
			else if (user == 2) {
				conection = CloseHandle(pipe);
				if (conection) cout << "You have been disconnected from a named pipe" << endl;
				else cout << "Failed to disconnect" << endl;
				conection = FALSE;
			}
			else if (user < 0 || user > 2) {
				cout << "Invalid command";
			}
		} while (user != 0);
	}
	return 0;
}
