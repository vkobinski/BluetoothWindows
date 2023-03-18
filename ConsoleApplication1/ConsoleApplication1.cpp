#include <Windows.h>
#include <bluetoothapis.h>
#include <bthdef.h>
#include <bthsdpdef.h>
#include <ws2bth.h>
#include <winsock.h>
#include <string.h>
#include <stdio.h>

int main() {

    WSADATA wsaData;

    int err = WSAStartup(MAKEWORD(2, 2), &wsaData);

    if (err != 0) {
        printf("WSAStartup failed with error: %d", err);
        return 1;
    }
    
    BLUETOOTH_DEVICE_SEARCH_PARAMS searchParams = { sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS) };
    searchParams.dwSize = sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS);
    searchParams.fReturnConnected = TRUE;

    BLUETOOTH_DEVICE_INFO deviceInfo = { sizeof(BLUETOOTH_DEVICE_INFO) };

    deviceInfo.dwSize = sizeof(BLUETOOTH_DEVICE_INFO);

    HANDLE hDevice = BluetoothFindFirstDevice(&searchParams, &deviceInfo);

    if (hDevice == NULL) {

        printf("BluetoothFindFirstDevice failed with error: %d\n", GetLastError());
    }

    if (BluetoothGetDeviceInfo(hDevice, &deviceInfo) == ERROR_SUCCESS) {
        printf("Device name: %ls\n", deviceInfo.szName);

		SOCKADDR_BTH btAddr = { 0 };
		btAddr.addressFamily = AF_BTH;
		btAddr.btAddr = deviceInfo.Address.ullLong;
        btAddr.serviceClassId = RFCOMM_PROTOCOL_UUID;
        btAddr.port = 1222;
        SOCKET hSocket = socket(AF_BTH, SOCK_STREAM, BTHPROTO_RFCOMM);

        if (connect(hSocket, (SOCKADDR*)&btAddr, sizeof(btAddr)) == SOCKET_ERROR) {
            printf("connect failed with error: %d\n", WSAGetLastError());
            BluetoothFindDeviceClose(hDevice);
            WSACleanup();
            return 1;
        }

        char buffer[12];

        while (1) {

            DWORD bytesWritten;
            const char* command = "AT+CSRBAT=1\r\n";

            send(hSocket, command, strlen(command), NULL);
            
			int bytesReceived = recv(hSocket, buffer, sizeof(buffer), NULL);

			if (bytesReceived > 0) {
				printf("Received %d bytes of data: ", bytesReceived);
				for (int i = 0; i < bytesReceived; i++) {
					printf("%02X", buffer[i]);
				}
				printf("\n");
            }
            else if (bytesReceived == 0) {
                printf("Connection closed by remote device\n");
                break;
            }
			else {
				printf("recv failed with error %d\n", GetLastError());
			}

        }

        closesocket(hSocket);
    }

    BluetoothFindDeviceClose(hDevice);
    WSACleanup();

    return 0;
}
