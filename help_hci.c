#include <windows.h>
#include <setupapi.h>
#include <stdio.h>
#include <initguid.h>
#include <stddef.h>

#pragma comment(lib, "setupapi.lib")

//DEFINE_GUID(GUID_BTHPORT_DEVICE_INTERFACE,
//0x850302a, 0xb344, 0x4fda, 0x9b, 0xe9, 0x90, 0x57, 0x6b, 0x8d, 0x46, 0xf0);

#include "bthdef.h"
#include "winioctl.h"
//#include "bthioctl.h"
//#include <winioctl.h>


#define FILE_DEVICE_BLUETOOTH 0x00000041
#define BTH_IOCTL_BASE      0

#define BTH_CTL(id)         CTL_CODE(FILE_DEVICE_BLUETOOTH,  \
                                     (id), \
                                     METHOD_BUFFERED,  \
                                     FILE_ANY_ACCESS)

//
// Input:   BTH_VENDOR_SPECIFIC_COMMAND
// Output:  PVOID
//
#define IOCTL_BTH_HCI_VENDOR_COMMAND                BTH_CTL(BTH_IOCTL_BASE+0x14)

#define IOCTL_BTH_GET_LOCAL_INFO            BTH_CTL(BTH_IOCTL_BASE+0x00)


//gcc ble_test2.c -lsetupapi -lole32 -Wl,--entry,ServiceMain@8 -o ble_test2.exe

//Device path: \\?\usb#vid_0bda&pid_b00c#00e04c000001#{0850302a-b344-4fda-9be9-90576b8d46f0}
//Ordinateur\HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\USB\VID_0BDA&PID_B00C\00e04c000001\Device Parameters
//Donne VsMsftOpCode REG_DWORD 0x0000fcf0

#pragma pack(push,1)

//
// Vendor specific HCI command header
//
typedef struct _BTH_COMMAND_HEADER {
    //
    // Opcode for the command
    //
    USHORT OpCode;
    //
    // Payload of the command excluding the header.
    // TotalParameterLength = TotalCommandLength - sizeof(BTH_COMMAND_HEADER)
    //
    UCHAR TotalParameterLength;

} BTH_COMMAND_HEADER, * PBTH_COMMAND_HEADER;

//
// Vendor Specific Command structure
//
typedef struct _BTH_VENDOR_SPECIFIC_COMMAND {
    //
    // Manufacturer ID
    //
    ULONG ManufacturerId;
    //
    // LMP version. Command is send to radio only if the radios
    // LMP version is greater than this value.
    //
    UCHAR LmpVersion;

    //
    // Should all the patterns match or just one. If MatchAnySinglePattern == TRUE
    // then if a single pattern matches the command, we decide that we have a match.
    //
    BOOLEAN MatchAnySinglePattern;

    //
    // HCI Command Header
    //
    BTH_COMMAND_HEADER  HciHeader;

    //
    // Data for the above command including patterns
    //
    UCHAR   Data[1];
} BTH_VENDOR_SPECIFIC_COMMAND, * PBTH_VENDOR_SPECIFIC_COMMAND;


//structure de reseption via IOCTL_BTH_GET_LOCAL_INFO
/*typedef struct _BTH_DEVICE_INFO {
  ULONG    flags;
  BTH_ADDR address;
  BTH_COD  classOfDevice;
  CHAR     name[BTH_MAX_NAME_SIZE];
} BTH_DEVICE_INFO, *PBTH_DEVICE_INFO;*/

typedef struct _BTH_RADIO_INFO {
  ULONGLONG lmpSupportedFeatures;
  USHORT    mfg;
  USHORT    lmpSubversion;
  UCHAR     lmpVersion;
} BTH_RADIO_INFO, *PBTH_RADIO_INFO;

typedef struct _BTH_LOCAL_RADIO_INFO {
  BTH_DEVICE_INFO localInfo;
  ULONG           flags;
  USHORT          hciRevision;
  UCHAR           hciVersion;
  BTH_RADIO_INFO  radioInfo;
} BTH_LOCAL_RADIO_INFO, *PBTH_LOCAL_RADIO_INFO;

#pragma pack(pop)



void printHexBuffer(const void *buffer, size_t size) {
    const unsigned char *bytes = (const unsigned char *)buffer;
    for (size_t i = 0; i < size; i++) {
        printf("%02X ", bytes[i]);
        if ((i + 1) % 16 == 0) {
            printf("\n");
        }
    }
    printf("\n");
}

void transformDevicePathToRegistryKey(const char *devicePath, char *registryKey, size_t registryKeySize) {
    const char *prefix = "SYSTEM\\CurrentControlSet\\Enum\\";  //in Ordinateur\\HKEY_LOCAL_MACHINE\\

    const char *suffix = "\\Device Parameters";

    const char *start = strstr(devicePath, "\\\\?\\");
    if (!start) {
        strncpy(registryKey, "Erreur : Format de chemin invalide", registryKeySize);
        return;
    }
    start += 4;

    const char *end = strstr(start, "#{");
    if (!end) {
        strncpy(registryKey, "Erreur : Format de chemin invalide", registryKeySize);
        return;
    }

    char temp[2048] = {0};
    size_t length = end - start;
    strncpy(temp, start, length);
    temp[length] = '\0';

    for (char *p = temp; *p; p++) {
        if (*p == '#') {
            *p = '\\';
        }
    }

    for (char *p = temp; *p; p++) {
        *p = toupper((unsigned char)*p);
    }

    snprintf(registryKey, registryKeySize, "%s%s%s", prefix, temp, suffix);
}

int my_main()
{
    SetConsoleOutputCP(1252);
    SetConsoleCP(1252);


    HDEVINFO devInfo;
    SP_DEVICE_INTERFACE_DATA devInterface;
    PSP_DEVICE_INTERFACE_DETAIL_DATA detail;
    DWORD size;

    devInfo = SetupDiGetClassDevs(
        &GUID_BTHPORT_DEVICE_INTERFACE,
        NULL,
        NULL,
        DIGCF_PRESENT | DIGCF_DEVICEINTERFACE
    );

    devInterface.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

    SetupDiEnumDeviceInterfaces(
        devInfo,
        NULL,
        &GUID_BTHPORT_DEVICE_INTERFACE,
        0,
        &devInterface
    );

    SetupDiGetDeviceInterfaceDetail(
        devInfo,
        &devInterface,
        NULL,
        0,
        &size,
        NULL
    );

    detail = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(size);
    detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

    SetupDiGetDeviceInterfaceDetail(
        devInfo,
        &devInterface,
        detail,
        size,
        NULL,
        NULL
    );

    printf("Device path: %s\n", detail->DevicePath);

    HANDLE h = CreateFile(
        detail->DevicePath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
    );

    if (h == INVALID_HANDLE_VALUE) {
        printf("Erreur ouverture\n");
        return 1;
    }
    else
        printf("Handle for HCI OK\n");

    char registryKey[2048];
    transformDevicePathToRegistryKey(detail->DevicePath, registryKey, sizeof(registryKey));
    if (strlen(registryKey)<=1){
        printf("error clé de registre à extraire");
        return 1;
    }

    printf("Clé de registre : %s\n", registryKey);

    free(detail);

    HKEY hKey;
    LONG regResult = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        registryKey,
        0,
        KEY_READ,
        &hKey
    );

    if (regResult != ERROR_SUCCESS) {
        printf("Erreur : Impossible d'ouvrir la clé de registre (code %d)\n", regResult);
        return 1;
    }

    // Lire VsMsftOpCode
    DWORD vsMsftOpCode = 0;
    DWORD dataSize = sizeof(DWORD);
    regResult = RegQueryValueEx(
        hKey,
        "VsMsftOpCode",
        NULL,
        NULL,
        (LPBYTE)&vsMsftOpCode,
        &dataSize
    );

    if (regResult != ERROR_SUCCESS) {
        printf("Erreur : Impossible de lire VsMsftOpCode (code %d)\n", regResult);
    } else {
        printf("VsMsftOpCode : 0x%04X\n", vsMsftOpCode);
    }

    RegCloseKey(hKey);

    if (regResult != ERROR_SUCCESS) return 1;

    //printf("IOCTL_BTH_HCI_VENDOR_COMMAND=%lu\n",IOCTL_BTH_HCI_VENDOR_COMMAND);

    //activé les droit de driver (admin obligatoire)
    HANDLE procToken;
    LUID luidLoadDriver, luidDebug;
    TOKEN_PRIVILEGES tp;

    if (!OpenProcessToken(
        GetCurrentProcess(),
        TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
        &procToken
    )) {
        printf("Erreur OpenProcessToken : %d\n", GetLastError());
        return 1;
    }

    if (!LookupPrivilegeValue(
        NULL,
        SE_LOAD_DRIVER_NAME,
        &luidLoadDriver
    )) {
        printf("Erreur LookupPrivilegeValue : %d\n", GetLastError());
        CloseHandle(procToken);
        return 1;
    }

    if (!LookupPrivilegeValue(
        NULL,
        SE_DEBUG_NAME,
        &luidDebug
    )) {
        printf("Erreur LookupPrivilegeValue : %d\n", GetLastError());
        CloseHandle(procToken);
        return 1;
    }

    tp.PrivilegeCount = 2;
    tp.Privileges[0].Luid = luidLoadDriver;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    tp.Privileges[1].Luid = luidDebug;
    tp.Privileges[1].Attributes = SE_PRIVILEGE_ENABLED;

    if (!AdjustTokenPrivileges(
        procToken,
        TRUE,
        &tp,
        sizeof(TOKEN_PRIVILEGES),
        NULL,
        NULL
    )) {
        printf("Erreur AdjustTokenPrivileges : %d\n", GetLastError());
        CloseHandle(procToken);
        return 1;
    }

    if (GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
        printf("Erreur : Les privilèges n'ont pas pu être activé.\n");
        CloseHandle(procToken);
        return 1;
    }

    printf("Privilège SE_LOAD_DRIVER_NAME et SE_DEBUG_NAME activé avec succès !\n");

    CloseHandle(procToken);

    BTH_LOCAL_RADIO_INFO radio_info;
    DWORD bytesReturned = 0;

    BOOL result = DeviceIoControl(
        h,
        IOCTL_BTH_GET_LOCAL_INFO,
        NULL,
        0,
        &radio_info,
        sizeof(radio_info),
        &bytesReturned,
        NULL
    );

    if (!result) {
        printf("Erreur DeviceIoControl : %d\n", GetLastError());
        CloseHandle(h);
        return 1;
    }

    printf("byte returned : 0x%08X (%d) , attendu : %d\n",bytesReturned,bytesReturned,sizeof(radio_info));
    printf("NAME : %s\n",radio_info.localInfo.name);
    printf("Manufacturer ID: 0x%04X (%d)\n",radio_info.radioInfo.mfg, radio_info.radioInfo.mfg);
    printf("LMP Version: 0x%02X\n",radio_info.radioInfo.lmpVersion);
    printf("LMP Subversion: 0x%02X\n", radio_info.radioInfo.lmpSubversion);
    printHexBuffer(&radio_info,bytesReturned);


    //suite commande au périférique (marche pas, pourquoi ????)
    BTH_VENDOR_SPECIFIC_COMMAND vendorCmd = {0};

    vendorCmd.ManufacturerId = (ULONG)radio_info.radioInfo.mfg;
    vendorCmd.LmpVersion = radio_info.radioInfo.lmpVersion;
    vendorCmd.MatchAnySinglePattern = TRUE;

    vendorCmd.HciHeader.OpCode = (USHORT)vsMsftOpCode;
    vendorCmd.HciHeader.TotalParameterLength = 1; //taille HCI

    vendorCmd.Data[0] = 0x00;  //HCI

    printf("send taille=%d :\n",sizeof(vendorCmd));
    printHexBuffer(&vendorCmd,sizeof(vendorCmd));

    bytesReturned = 0;
    UCHAR response[1024] = {0};

    result = DeviceIoControl(
        h,
        IOCTL_BTH_HCI_VENDOR_COMMAND,
        &vendorCmd,
        sizeof(vendorCmd),
        response,
        sizeof(response),
        &bytesReturned,
        NULL
    );

    if (!result) {
        printf("Erreur DeviceIoControl : %d\n", GetLastError());
    } else {
        printf("Commande envoyée avec succès ! Réponse (%d octets) :\n", bytesReturned);
        for (DWORD i = 0; i < bytesReturned; i++) {
            printf("%02X ", response[i]);
        }
        printf("\n");
    }

    CloseHandle(h);
    SetupDiDestroyDeviceInfoList(devInfo);
    return 0;
}
