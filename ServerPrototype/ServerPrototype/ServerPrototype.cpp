#pragma comment(lib, "winmm.lib")
#include <iostream>
#include <windows.h>
#include <string>

using namespace std;

BOOL conection = FALSE;
OVERLAPPED overlapped_struct, overlapped_pipe;
HANDLE ev;
HANDLE pipe;

void Connect();
void Disconnect();
void SendMsg();
void OpenPipe();

int InputAsNumber();
void cleanStream();

int main()
{
	overlapped_struct = OVERLAPPED();
	overlapped_pipe = OVERLAPPED();
	pipe = CreateNamedPipe(L"\\\\.\\pipe\\superpipe", PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
		1, 64, 64, 0, NULL);
	ev = CreateEvent(NULL, FALSE, FALSE, NULL);

	if (pipe == INVALID_HANDLE_VALUE || ev == INVALID_HANDLE_VALUE) cout << "Error of initialization server\n";
	else {
		cout << "Server init successfull\n";
		int user = 123;
		do {
			void((*option[3]))() = { Connect, SendMsg, Disconnect };
			cout << "1 - Open a named pipe channel\n";
			cout << "2 - Send message\n";
			cout << "3 - Detach named pipe channel\n";
			cout << "0 - Exit\n";
			user = InputAsNumber();
 			if (user < 0 || user > 3) cout << "Invalid command\n";
			else option[user - 1]();
		} while (user != 0);
	}
	if (pipe != INVALID_HANDLE_VALUE) CloseHandle(pipe);
	if (ev != INVALID_HANDLE_VALUE) CloseHandle(ev);

	return 0;
}

void Connect() {
	overlapped_pipe.hEvent = ev;

	conection = ConnectNamedPipe(pipe, &overlapped_pipe);
	WaitForSingleObject(ev, INFINITE);

	if (conection) cout << "Connection successfull\n";
	else cout << "Connection failed\n";

	return;
}

void SendMsg() {
	if (!conection) cout << "No connection. Unable send message\n";
	else {
		string msg;

		cout << "Enter your message: \n";
		cleanStream();
		getline(cin, msg);

		overlapped_struct.hEvent = ev;

		conection = WriteFile(pipe, (LPCVOID)((CHAR*)msg.c_str()), 64, NULL, &overlapped_struct);

		if (WaitForSingleObject(ev, 200000) == WAIT_OBJECT_0 && conection) cout << "Message sending successfull\n";
		else cout << "Message sending failed\n";
	}
	return;
}

void Disconnect() {
	conection = DisconnectNamedPipe(pipe);
	if (conection) cout << "Successful disconected\n";
	else cout << "Disconect failed\n";
	conection = FALSE;
	return;
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
			cout << "Invalid format of command! Expected natural number\n" << endl;
		}

	} while (!flag);
	return in;
}

