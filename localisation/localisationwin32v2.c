/*
 * =============================================
 * Fichier : localisationwin32v2.c
 * Description : permet de récupérer la localisation
 * de l'appareil via l'api WinRT depuis Win32.
 *
 * Auteur : Argentropcher
 * Date de création : 20/08/2026
 * Date de dernière modification : 21/08/2026
 *
 * ---------------------------------------------
 * Droit d'auteur (c) 2026 Argentropcher
 * Tous droits réservés.
 *
 * Ce logiciel est fourni "tel quel", sans garantie
 * d'aucune sorte, expresse ou implicite, y compris,
 * sans limitation, les garanties de qualité marchande,
 * d'adéquation à un usage particulier et de non-violation
 * des droits de tiers.
 *
 * En aucun cas, les auteurs ou détenteurs des droits
 * d'auteur ne pourront être tenus responsables de
 * tout dommage, réclamation ou autre responsabilité,
 * que ce soit dans le cadre d'un contrat, d'un délit ou
 * autre, résultant de l'utilisation, de l'impossibilité
 * d'utiliser ou des résultats obtenus avec ce logiciel.
 *
 * La version localisationwin32v2.dll est sous le même code
 * et est strictement appliqué aux mêmes conditions.
 *
 * ---------------------------------------------
 * Ce code utilise WinRT via un exécutable Win32,
 * mais n'est pas affilié à, approuvé par ou associé
 * à Microsoft Corporation (c) / Win32 / WinRT.
 * =============================================
 */

 /* ===========================================================================
    compilation : gcc -shared localisationwin32v2.c -o localisationwin32v2.dll -lgdi32
    =========================================================================== */

#define WIN32_LEAN_AND_MEAN

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00
#endif

#include <windows.h>
#include <winstring.h>
#include <stdint.h>

#include <roapi.h>
#include <hstring.h>
#include <stdio.h>

/* ============================================================
    Information du Thread pour main
   ============================================================ */
typedef struct {
    HRESULT hr;
    DOUBLE latitude;
    DOUBLE longitude;
    DOUBLE accuracy;
    char exception_value[2048];
} GeoResult;

static GeoResult global_result = {0};

/* ============================================================
   Fonctions chargées dynamiquement depuis combase.dll
   ============================================================ */

typedef HRESULT (WINAPI *fnRoInitialize)(RO_INIT_TYPE);
typedef void    (WINAPI *fnRoUninitialize)(void);

typedef HRESULT (WINAPI *fnCoInitializeEx)(LPVOID pvReserved, DWORD dwCoInit);
typedef void    (WINAPI *fnCoUninitialize)(void);

typedef HRESULT (WINAPI *fnWindowsCreateString)(LPCWSTR, UINT32, HSTRING *);
typedef HRESULT (WINAPI *fnWindowsDeleteString)(HSTRING);

typedef HRESULT (WINAPI *fnRoGetActivationFactory)(HSTRING, REFIID, void **);

/* ============================================================
   GUID / IID
   ============================================================ */

/*
    Windows.Devices.Geolocation.IGeolocatorStatics
    9a8e7571-2df5-4591-9f87-eb5fd894e9b7
*/
static const IID IID_IGeolocatorStatics = {
    0x9a8e7571,
    0x2df5,
    0x4591,
    {0x9f, 0x87, 0xeb, 0x5f, 0xd8, 0x94, 0xe9, 0xb7}
};

/*
    Windows.Devices.Geolocation.IGeolocator
    a9c3bf62-4524-4989-8aa9-de019d2e551f
*/
static const IID IID_IGeolocator = {
    0xa9c3bf62,
    0x4524,
    0x4989,
    {0x8a, 0xa9, 0xde, 0x01, 0x9d, 0x2e, 0x55, 0x1f}
};

/*
    IActivationFactory
    00000035-0000-0000-C000-000000000046
*/
static const IID MY_IID_IActivationFactory = {
    0x00000035,
    0x0000,
    0x0000,
    {0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}
};

/*
    IAsyncInfo
    00000036-0000-0000-C000-000000000046
*/
static const IID IID_IAsyncInfo = {
    0x00000036,
    0x0000,
    0x0000,
    {0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x46}
};

/* ============================================================
   Types WinRT nécessaires
   ============================================================ */

typedef enum GeolocationAccessStatus {
    GeolocationAccessStatus_Unspecified = 0,
    GeolocationAccessStatus_Allowed     = 1,
    GeolocationAccessStatus_Denied      = 2
} GeolocationAccessStatus;


typedef enum AsyncStatus {
    AsyncStatus_Started  = 0,
    AsyncStatus_Completed = 1,
    AsyncStatus_Canceled  = 2,
    AsyncStatus_Error     = 3
} AsyncStatus;

/* ============================================================
   Forward declarations
   ============================================================ */

typedef struct IGeolocatorStatics IGeolocatorStatics;
typedef struct IGeolocator IGeolocator;
typedef struct IGeoposition IGeoposition;
typedef struct IGeocoordinate IGeocoordinate;

typedef struct IAsyncOperationAccess IAsyncOperationAccess;
typedef struct IAsyncOperationAccessVtbl IAsyncOperationAccessVtbl;
typedef struct IAsyncOperationGeo IAsyncOperationGeo;
typedef struct IAsyncOperationGeoVtbl IAsyncOperationGeoVtbl;

typedef struct IAsyncOperationAccessCompletedHandler IAsyncOperationAccessCompletedHandler;
typedef struct IAsyncOperationGeoCompletedHandler IAsyncOperationGeoCompletedHandler;

typedef struct IAsyncInfo IAsyncInfo;
typedef struct IAsyncInfoVtbl IAsyncInfoVtbl;

/* ============================================================
   IGeolocatorStatics
   ============================================================ */

typedef struct IGeolocatorStaticsVtbl {
    /* IUnknown */
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        IGeolocatorStatics *,
        REFIID,
        void **
    );
    ULONG (STDMETHODCALLTYPE *AddRef)(
        IGeolocatorStatics *
    );
    ULONG (STDMETHODCALLTYPE *Release)(
        IGeolocatorStatics *
    );
    /* IInspectable */
    HRESULT (STDMETHODCALLTYPE *GetIids)(
        IGeolocatorStatics *,
        ULONG *,
        IID **
    );
    HRESULT (STDMETHODCALLTYPE *GetRuntimeClassName)(
        IGeolocatorStatics *,
        HSTRING *
    );
    HRESULT (STDMETHODCALLTYPE *GetTrustLevel)(
        IGeolocatorStatics *,
        TrustLevel *
    );
    /* IGeolocatorStatics */
    HRESULT (STDMETHODCALLTYPE *RequestAccessAsync)(
        IGeolocatorStatics *,
        IAsyncOperationAccess **
    );
    HRESULT (STDMETHODCALLTYPE *GetGeopositionHistoryAsync)(
        IGeolocatorStatics *,
        INT64,
        void **
    );
    HRESULT (STDMETHODCALLTYPE *GetGeopositionHistoryWithDurationAsync)(
        IGeolocatorStatics *,
        INT64,
        INT64,
        void **
    );

} IGeolocatorStaticsVtbl;

struct IGeolocatorStatics {
    const IGeolocatorStaticsVtbl *lpVtbl;
};


/* ============================================================
   IGeolocator
   ============================================================ */

typedef struct IGeolocatorVtbl {
    /* IUnknown */
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        IGeolocator *,
        REFIID,
        void **
    );
    ULONG (STDMETHODCALLTYPE *AddRef)(
        IGeolocator *
    );
    ULONG (STDMETHODCALLTYPE *Release)(
        IGeolocator *
    );
    /* IInspectable */
    HRESULT (STDMETHODCALLTYPE *GetIids)(
        IGeolocator *,
        ULONG *,
        IID **
    );
    HRESULT (STDMETHODCALLTYPE *GetRuntimeClassName)(
        IGeolocator *,
        HSTRING *
    );
    HRESULT (STDMETHODCALLTYPE *GetTrustLevel)(
        IGeolocator *,
        TrustLevel *
    );
    /* IGeolocator */
    HRESULT (STDMETHODCALLTYPE *get_DesiredAccuracy)(
        IGeolocator *,
        int *
    );
    HRESULT (STDMETHODCALLTYPE *put_DesiredAccuracy)(
        IGeolocator *,
        int
    );
    HRESULT (STDMETHODCALLTYPE *get_MovementThreshold)(
        IGeolocator *,
        DOUBLE *
    );
    HRESULT (STDMETHODCALLTYPE *put_MovementThreshold)(
        IGeolocator *,
        DOUBLE
    );
    HRESULT (STDMETHODCALLTYPE *get_ReportInterval)(
        IGeolocator *,
        UINT32 *
    );
    HRESULT (STDMETHODCALLTYPE *put_ReportInterval)(
        IGeolocator *,
        UINT32
    );
    HRESULT (STDMETHODCALLTYPE *get_LocationStatus)(
        IGeolocator *,
        int *
    );
    HRESULT (STDMETHODCALLTYPE *GetGeopositionAsync)(
        IGeolocator *,
        IAsyncOperationGeo **
    );
    HRESULT (STDMETHODCALLTYPE *GetGeopositionAsyncWithAgeAndTimeout)(
        IGeolocator *,
        INT64,
        INT64,
        IAsyncOperationGeo **
    );
    HRESULT (STDMETHODCALLTYPE *add_PositionChanged)(
        IGeolocator *,
        void *,
        void *
    );
    HRESULT (STDMETHODCALLTYPE *remove_PositionChanged)(
        IGeolocator *,
        INT64
    );
    HRESULT (STDMETHODCALLTYPE *add_StatusChanged)(
        IGeolocator *,
        void *,
        void *
    );
    HRESULT (STDMETHODCALLTYPE *remove_StatusChanged)(
        IGeolocator *,
        INT64
    );

} IGeolocatorVtbl;

struct IGeolocator {
    const IGeolocatorVtbl *lpVtbl;
};


/* ============================================================
   IAsyncOperationAccess
   IAsyncOperation<GeolocationAccessStatus>
   ============================================================ */

struct IAsyncOperationAccessVtbl {
    /* IUnknown */
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        IAsyncOperationAccess *This,
        REFIID riid,
        void **ppvObject
    );
    ULONG (STDMETHODCALLTYPE *AddRef)(
        IAsyncOperationAccess *This
    );
    ULONG (STDMETHODCALLTYPE *Release)(
        IAsyncOperationAccess *This
    );
    /* IInspectable */
    HRESULT (STDMETHODCALLTYPE *GetIids)(
        IAsyncOperationAccess *This,
        ULONG *iidCount,
        IID **iids
    );
    HRESULT (STDMETHODCALLTYPE *GetRuntimeClassName)(
        IAsyncOperationAccess *This,
        HSTRING *className
    );
    HRESULT (STDMETHODCALLTYPE *GetTrustLevel)(
        IAsyncOperationAccess *This,
        TrustLevel *trustLevel
    );
    /* GeolocationAccessStatus */
    HRESULT (STDMETHODCALLTYPE *put_Completed)(
        IAsyncOperationAccess *This,
        IAsyncOperationAccessCompletedHandler *handler
    );
    HRESULT (STDMETHODCALLTYPE *get_Completed)(
        IAsyncOperationAccess *This,
        IAsyncOperationAccessCompletedHandler **handler
    );
    HRESULT (STDMETHODCALLTYPE *GetResults)(
        IAsyncOperationAccess *This,
        GeolocationAccessStatus *results
    );
};

struct IAsyncOperationAccess {
    const IAsyncOperationAccessVtbl *lpVtbl;
};

/* ============================================================
   IAsyncOperationGeo
   IAsyncOperation<Geoposition>
   ============================================================ */

struct IAsyncOperationGeoVtbl {
    /* IUnknown */
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        IAsyncOperationGeo *This,
        REFIID riid,
        void **ppvObject
    );
    ULONG (STDMETHODCALLTYPE *AddRef)(
        IAsyncOperationGeo *This
    );
    ULONG (STDMETHODCALLTYPE *Release)(
        IAsyncOperationGeo *This
    );
    /* IInspectable */
    HRESULT (STDMETHODCALLTYPE *GetIids)(
        IAsyncOperationGeo *This,
        ULONG *iidCount,
        IID **iids
    );
    HRESULT (STDMETHODCALLTYPE *GetRuntimeClassName)(
        IAsyncOperationGeo *This,
        HSTRING *className
    );
    HRESULT (STDMETHODCALLTYPE *GetTrustLevel)(
        IAsyncOperationGeo *This,
        TrustLevel *trustLevel
    );
    /* Geoposition */
    HRESULT (STDMETHODCALLTYPE *put_Completed)(
        IAsyncOperationGeo *This,
        IAsyncOperationGeoCompletedHandler *handler
    );
    HRESULT (STDMETHODCALLTYPE *get_Completed)(
        IAsyncOperationGeo *This,
        IAsyncOperationGeoCompletedHandler **handler
    );
    HRESULT (STDMETHODCALLTYPE *GetResults)(
        IAsyncOperationGeo *This,
        IGeoposition **results
    );
};

struct IAsyncOperationGeo {
    const IAsyncOperationGeoVtbl *lpVtbl;
};

/* ============================================================
   CompletedHandler<GeolocationAccessStatus>
   ============================================================ */

typedef struct IAsyncOperationAccessCompletedHandlerVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        IAsyncOperationAccessCompletedHandler *,
        REFIID,
        void **
    );
    ULONG (STDMETHODCALLTYPE *AddRef)(
        IAsyncOperationAccessCompletedHandler *
    );
    ULONG (STDMETHODCALLTYPE *Release)(
        IAsyncOperationAccessCompletedHandler *
    );
    HRESULT (STDMETHODCALLTYPE *Invoke)(
        IAsyncOperationAccessCompletedHandler *,
        IAsyncOperationAccess *asyncoInfo,
        AsyncStatus
    );

} IAsyncOperationAccessCompletedHandlerVtbl;

struct IAsyncOperationAccessCompletedHandler {
    const IAsyncOperationAccessCompletedHandlerVtbl *lpVtbl;
};

/* ============================================================
   CompletedHandler<Geoposition>
   ============================================================ */

typedef struct IAsyncOperationGeoCompletedHandlerVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        IAsyncOperationGeoCompletedHandler *,
        REFIID,
        void **
    );
    ULONG (STDMETHODCALLTYPE *AddRef)(
        IAsyncOperationGeoCompletedHandler *
    );
    ULONG (STDMETHODCALLTYPE *Release)(
        IAsyncOperationGeoCompletedHandler *
    );
    HRESULT (STDMETHODCALLTYPE *Invoke)(
        IAsyncOperationGeoCompletedHandler *,
        IAsyncOperationGeo *asyncoInfo,
        AsyncStatus
    );

} IAsyncOperationGeoCompletedHandlerVtbl;

struct IAsyncOperationGeoCompletedHandler {
    const IAsyncOperationGeoCompletedHandlerVtbl *lpVtbl;
};

/* ============================================================
   IAsyncInfo
   ============================================================ */

struct IAsyncInfoVtbl {
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        IAsyncInfo *,
        REFIID,
        void **
    );
    ULONG (STDMETHODCALLTYPE *AddRef)(
        IAsyncInfo *
    );
    ULONG (STDMETHODCALLTYPE *Release)(
        IAsyncInfo *
    );
    HRESULT (STDMETHODCALLTYPE *GetIids)(
        IAsyncInfo *,
        ULONG *,
        IID **
    );
    HRESULT (STDMETHODCALLTYPE *GetRuntimeClassName)(
        IAsyncInfo *,
        HSTRING *
    );
    HRESULT (STDMETHODCALLTYPE *GetTrustLevel)(
        IAsyncInfo *,
        TrustLevel *
    );
    HRESULT (STDMETHODCALLTYPE *get_Id)(
        IAsyncInfo *,
        UINT32 *
    );
    HRESULT (STDMETHODCALLTYPE *get_Status)(
        IAsyncInfo *,
        AsyncStatus *
    );
    HRESULT (STDMETHODCALLTYPE *get_ErrorCode)(
        IAsyncInfo *,
        HRESULT *
    );
    HRESULT (STDMETHODCALLTYPE *Cancel)(
        IAsyncInfo *
    );
    HRESULT (STDMETHODCALLTYPE *Close)(
        IAsyncInfo *
    );
};

struct IAsyncInfo {
    const IAsyncInfoVtbl *lpVtbl;
};

/* ============================================================
   IGeoposition
   ============================================================ */

typedef struct IGeopositionVtbl {
    /* IUnknown */
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        IGeoposition *,
        REFIID,
        void **
    );
    ULONG (STDMETHODCALLTYPE *AddRef)(
        IGeoposition *
    );
    ULONG (STDMETHODCALLTYPE *Release)(
        IGeoposition *
    );
    /* IInspectable */
    HRESULT (STDMETHODCALLTYPE *GetIids)(
        IGeoposition *,
        ULONG *,
        IID **
    );
    HRESULT (STDMETHODCALLTYPE *GetRuntimeClassName)(
        IGeoposition *,
        HSTRING *
    );
    HRESULT (STDMETHODCALLTYPE *GetTrustLevel)(
        IGeoposition *,
        TrustLevel *
    );
    /* IGeoposition */
    HRESULT (STDMETHODCALLTYPE *get_Coordinate)(
        IGeoposition *,
        IGeocoordinate **
    );
    HRESULT (STDMETHODCALLTYPE *get_CivicAddress)(
        IGeoposition *,
        void **
    );

} IGeopositionVtbl;

struct IGeoposition {
    const IGeopositionVtbl *lpVtbl;
};

/* ============================================================
   IGeocoordinate
   ============================================================ */

typedef struct IGeocoordinateVtbl {
    /* IUnknown */
    HRESULT (STDMETHODCALLTYPE *QueryInterface)(
        IGeocoordinate *,
        REFIID,
        void **
    );
    ULONG (STDMETHODCALLTYPE *AddRef)(
        IGeocoordinate *
    );
    ULONG (STDMETHODCALLTYPE *Release)(
        IGeocoordinate *
    );
    /* IInspectable */
    HRESULT (STDMETHODCALLTYPE *GetIids)(
        IGeocoordinate *,
        ULONG *,
        IID **
    );
    HRESULT (STDMETHODCALLTYPE *GetRuntimeClassName)(
        IGeocoordinate *,
        HSTRING *
    );
    HRESULT (STDMETHODCALLTYPE *GetTrustLevel)(
        IGeocoordinate *,
        TrustLevel *
    );
    /* IGeocoordinate */
    HRESULT (STDMETHODCALLTYPE *get_Latitude)(
        IGeocoordinate *,
        DOUBLE *
    );
    HRESULT (STDMETHODCALLTYPE *get_Longitude)(
        IGeocoordinate *,
        DOUBLE *
    );
    HRESULT (STDMETHODCALLTYPE *get_Altitude)(
        IGeocoordinate *,
        void **
    );
    HRESULT (STDMETHODCALLTYPE *get_Accuracy)(
        IGeocoordinate *,
        DOUBLE *
    );
    HRESULT (STDMETHODCALLTYPE *get_AltitudeAccuracy)(
        IGeocoordinate *,
        void **
    );
    HRESULT (STDMETHODCALLTYPE *get_Heading)(
        IGeocoordinate *,
        void **
    );
    HRESULT (STDMETHODCALLTYPE *get_Speed)(
        IGeocoordinate *,
        void **
    );
    HRESULT (STDMETHODCALLTYPE *get_Timestamp)(
        IGeocoordinate *,
        void *
    );

} IGeocoordinateVtbl;

struct IGeocoordinate {
    const IGeocoordinateVtbl *lpVtbl;
};

/* ============================================================
*   ///////////////////////////////////////////////////////////
*   ///////////////////////////CODE////////////////////////////
*   ///////////////////////////////////////////////////////////
   ============================================================ */

/* ============================================================
   Attendre une opération WinRT
   ============================================================ */

static HRESULT wait_access_debug(IAsyncOperationAccess *operation, GeolocationAccessStatus *result) {
    IAsyncInfo *asyncInfo = NULL;
    AsyncStatus status;
    HRESULT hr;
    HRESULT error = S_OK;
    MSG msg;

    /* ========================================================
       IAsyncOperationAccess -> IAsyncInfo
       ======================================================== */

    hr = operation->lpVtbl->QueryInterface(
        operation,
        &IID_IAsyncInfo,
        (void **)&asyncInfo
    );

    if (FAILED(hr))
        return hr;

    for (;;) {
        /* ====================================================
           Traiter toute la file de messages
           ==================================================== */

        while (PeekMessageW(
            &msg,
            NULL,
            0,
            0,
            PM_REMOVE
        )) {
            if (msg.message == WM_QUIT) {

                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);

                goto wait_cleanup;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        /* ====================================================
           Vérifier l'état via IAsyncInfo
           ==================================================== */

        hr = asyncInfo->lpVtbl->get_Status(asyncInfo, &status);

        if (FAILED(hr)) {

            goto wait_cleanup;
        }

        /* ====================================================
           Completed
           ==================================================== */

        if (status == AsyncStatus_Completed) {
            break;
        }


        /* ====================================================
           Error
           ==================================================== */

        if (status == AsyncStatus_Error) {
            hr = asyncInfo->lpVtbl->get_ErrorCode(asyncInfo, &error);

            if (FAILED(hr))
                goto wait_cleanup;

            hr = error;

            goto wait_cleanup;
        }

        /* ====================================================
           Canceled
           ==================================================== */

        if (status == AsyncStatus_Canceled) {

            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);

            goto wait_cleanup;
        }

        MsgWaitForMultipleObjectsEx(0, NULL, 20, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
    }


    /* ========================================================
       Récupérer le résultat
       ======================================================== */
    hr = operation->lpVtbl->GetResults(operation, result);

wait_cleanup:

    asyncInfo->lpVtbl->Release(asyncInfo);

    return hr;
}

/* ============================================================
   Attendre GetGeopositionAsync
   ============================================================ */

static HRESULT wait_geoposition(IAsyncOperationGeo *operation, IGeoposition **result){
    IAsyncInfo *asyncInfo = NULL;
    AsyncStatus status;
    HRESULT error = S_OK;
    HRESULT hr;
    MSG msg;

    /* ========================================================
       IAsyncOperationGeo -> IAsyncInfo
       ======================================================== */

    hr = operation->lpVtbl->QueryInterface(
        operation,
        &IID_IAsyncInfo,
        (void **)&asyncInfo
    );

    if (FAILED(hr))
        return hr;

    for (;;) {

        /* ====================================================
           Traiter les messages
           ==================================================== */

        while (PeekMessageW(
            &msg,
            NULL,
            0,
            0,
            PM_REMOVE
        )) {
            if (msg.message == WM_QUIT) {
                hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);

                goto geo_cleanup;
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }

        /* ====================================================
           Statut via IAsyncInfo
           ==================================================== */

        hr = asyncInfo->lpVtbl->get_Status(asyncInfo, &status);

        if (FAILED(hr))
            goto geo_cleanup;

        if (status == AsyncStatus_Completed)
            break;

        if (status == AsyncStatus_Error) {
            hr = asyncInfo->lpVtbl->get_ErrorCode(asyncInfo, &error);

            if (FAILED(hr))
                goto geo_cleanup;

            hr = error;

            goto geo_cleanup;
        }

        if (status == AsyncStatus_Canceled) {

            hr = HRESULT_FROM_WIN32(ERROR_CANCELLED);

            goto geo_cleanup;
        }

        MsgWaitForMultipleObjectsEx(0, NULL, 20, QS_ALLINPUT, MWMO_INPUTAVAILABLE);
    }

    /* ========================================================
       GetResults appartient à IAsyncOperationGeo
       ======================================================== */

    hr = operation->lpVtbl->GetResults(operation, result);


geo_cleanup:

    asyncInfo->lpVtbl->Release(asyncInfo);

    return hr;
}

static LRESULT CALLBACK GeoWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rc;
            GetClientRect(hwnd, &rc);

            FillRect(hdc, &rc, (HBRUSH)(COLOR_WINDOW + 1));

            SetBkMode(hdc, TRANSPARENT);

            DrawTextW(
                hdc,
                L"Localisation Windows",
                -1,
                &rc,
                DT_CENTER |
                DT_VCENTER |
                DT_SINGLELINE
            );
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

DWORD WINAPI UiThreadProc(LPVOID lpParam) {
    GeoResult *out = (GeoResult *)lpParam;

    // Fenêtre
    WNDCLASSW wc = {0};
    //wc.lpfnWndProc = DefWindowProcW;
    wc.lpfnWndProc = GeoWndProc;
    wc.hInstance = GetModuleHandleW(NULL);
    wc.lpszClassName = L"HiddenWinRTWindow";
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    if (!RegisterClassW(&wc)) {
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            snprintf(out->exception_value, sizeof(out->exception_value), "RegisterClassW failed : %lu\n", GetLastError());
            return 1;
        }
    }

    HWND hwnd = CreateWindowExW(
        0, wc.lpszClassName, L"localisation",
        WS_OVERLAPPEDWINDOW,
        0,0,0,0,
        NULL, NULL, wc.hInstance, NULL
    );

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    SetWindowPos(
        hwnd,
        HWND_TOPMOST,
        0,
        0,
        0,
        0,
        SWP_NOMOVE |
        SWP_NOSIZE |
        SWP_SHOWWINDOW
    );
    SetForegroundWindow(hwnd);
    SetActiveWindow(hwnd);
    SetFocus(hwnd);

    // Boucle de messages
    MSG msg;
    PeekMessageW(&msg, NULL, 0, 0, PM_NOREMOVE); // Active la queue

    // code WinRT

    HMODULE dll;

    fnCoInitializeEx CoInitializeEx;
    fnCoUninitialize CoUninitialize;
    fnRoInitialize pRoInitialize;
    fnRoUninitialize pRoUninitialize;
    fnWindowsCreateString pWindowsCreateString;
    fnWindowsDeleteString pWindowsDeleteString;
    fnRoGetActivationFactory pRoGetActivationFactory;

    HRESULT hr;

    HSTRING className = NULL;

    IGeolocatorStatics *statics = NULL;
    IGeolocator *geolocator = NULL;

    IAsyncOperationAccess *accessOperation = NULL;
    IAsyncOperationGeo *geoOperation = NULL;

    IGeoposition *position = NULL;
    IGeocoordinate *coordinate = NULL;

    GeolocationAccessStatus accessStatus;

    DOUBLE latitude;
    DOUBLE longitude;
    DOUBLE accuracy;


    /* ========================================================
       combase.dll
       ======================================================== */

    dll = LoadLibraryW(L"combase.dll");
    if (!dll) {
        snprintf(out->exception_value, sizeof(out->exception_value), "LoadLibrary(combase.dll) : %lu\n", GetLastError());
        return 1;
    }

    CoInitializeEx = (fnCoInitializeEx)GetProcAddress(dll, "CoInitializeEx");
    CoUninitialize = (fnCoUninitialize)GetProcAddress(dll, "CoUninitialize");

    pRoInitialize = (fnRoInitialize)GetProcAddress(dll, "RoInitialize");
    pRoUninitialize = (fnRoUninitialize)GetProcAddress(dll, "RoUninitialize");

    pWindowsCreateString = (fnWindowsCreateString)GetProcAddress(dll, "WindowsCreateString");
    pWindowsDeleteString = (fnWindowsDeleteString)GetProcAddress(dll, "WindowsDeleteString");

    pRoGetActivationFactory = (fnRoGetActivationFactory)GetProcAddress(dll, "RoGetActivationFactory");

    if (!CoInitializeEx ||
        !CoUninitialize ||
        !pRoInitialize ||
        !pRoUninitialize ||
        !pWindowsCreateString ||
        !pWindowsDeleteString ||
        !pRoGetActivationFactory)
    {
        snprintf(out->exception_value, sizeof(out->exception_value), "Fonction manquante dans combase.dll\n");
        FreeLibrary(dll);
        return 1;
    }

    /* ========================================================
       Initialiser WinRT
       ======================================================== */

    /*
    //version avec CoInitalize désactivé car pRoInitialize suffit
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE); //COINIT_APARTMENTTHREADED |
    printf("CoInitializeEx = 0x%08lX\n", (unsigned long)hr);
    if (FAILED(hr)) {
        FreeLibrary(dll);
        return 1;
    }*/

    // STA choisi pour l'api
    hr = pRoInitialize(RO_INIT_SINGLETHREADED); //RO_INIT_MULTITHREADED

    if (FAILED(hr)) {
        snprintf(out->exception_value, sizeof(out->exception_value), "RoInitialize = 0x%08lX\n", (unsigned long)hr);
        FreeLibrary(dll);
        return 1;
    }

    /* ========================================================
       Nom de la classe WinRT
       ======================================================== */

    hr = pWindowsCreateString(L"Windows.Devices.Geolocation.Geolocator", 38, &className);

    if (FAILED(hr)) {
        snprintf(out->exception_value, sizeof(out->exception_value), "WindowsCreateString = 0x%08lX\n", (unsigned long)hr);
        goto cleanup;
    }

    /* ========================================================
       Récupérer IGeolocatorStatics
       ======================================================== */

    hr = pRoGetActivationFactory(
        className,
        &IID_IGeolocatorStatics,
        (void **)&statics
    );

    if (FAILED(hr)) {
        snprintf(out->exception_value, sizeof(out->exception_value), "RoGetActivationFactory = 0x%08lX\n", (unsigned long)hr);
        goto cleanup;
    }

    /* ========================================================
       DEMANDE D'AUTORISATION
       ======================================================== */

    hr = statics->lpVtbl->RequestAccessAsync(
        statics,
        &accessOperation
    );

    if (FAILED(hr)) {
        snprintf(out->exception_value, sizeof(out->exception_value), "RequestAccessAsync = 0x%08lX\n", (unsigned long)hr);
        goto cleanup;
    }

    // On attend réellement la fin de l'opération.
    hr = wait_access_debug(accessOperation, &accessStatus);

    if (FAILED(hr)) {
        snprintf(out->exception_value, sizeof(out->exception_value), "RequestAccessAsync termine = 0x%08lX\nGeolocationAccessStatus = %d\n", (unsigned long)hr, (int)accessStatus);
        goto cleanup;
    }

    if (accessStatus != GeolocationAccessStatus_Allowed) {
        snprintf(out->exception_value, sizeof(out->exception_value), "Acces à la localisation REFUSE.\nActive la localisation dans les paramètres Windows si la machine l'accepte, bouton emplacement.\n");
        goto cleanup;
    }
    //printf("Acces à la localisation ACCORDE.\n");

    /* ========================================================
       Créer une instance de Geolocator
       ======================================================== */

    {
        IActivationFactory *factory = NULL;
        IInspectable *inspectable = NULL;

        // On récupère IActivationFactory de la même classe.
        hr = pRoGetActivationFactory(
            className,
            &MY_IID_IActivationFactory,
            (void **)&factory
        );

        if (FAILED(hr)) {
            snprintf(out->exception_value, sizeof(out->exception_value), "ActivationFactory = 0x%08lX\n", (unsigned long)hr);
            goto cleanup;
        }

        hr = factory->lpVtbl->ActivateInstance(
            factory,
            &inspectable
        );

        if (FAILED(hr)) {
            snprintf(out->exception_value, sizeof(out->exception_value), "ActivateInstance = 0x%08lX\n", (unsigned long)hr);
            factory->lpVtbl->Release(factory);
            goto cleanup;
        }

        // Transformer IInspectable en IGeolocator.
        hr = inspectable->lpVtbl->QueryInterface(
            inspectable,
            &IID_IGeolocator,
            (void **)&geolocator
        );

        inspectable->lpVtbl->Release(inspectable);

        factory->lpVtbl->Release(factory);

        if (FAILED(hr)) {
            snprintf(out->exception_value, sizeof(out->exception_value), "QueryInterface(IGeolocator) = 0x%08lX\n", (unsigned long)hr);
            goto cleanup;
        }
    }

    /* ========================================================
       Demander la position
       ======================================================== */

    hr = geolocator->lpVtbl->GetGeopositionAsync(
        geolocator,
        &geoOperation
    );

    if (FAILED(hr)) {
        snprintf(out->exception_value, sizeof(out->exception_value), "GetGeopositionAsync = 0x%08lX\n", (unsigned long)hr);
        goto cleanup;
    }

    // Ici on attend le GPS/WiFi/localisation Windows.
    hr = wait_geoposition(geoOperation, &position);

    if (FAILED(hr)) {
        snprintf(out->exception_value, sizeof(out->exception_value), "GetGeopositionAsync termine = 0x%08lX\n", (unsigned long)hr);
        goto cleanup;
    }

    /* ========================================================
       Geoposition -> Geocoordinate
       ======================================================== */

    hr = position->lpVtbl->get_Coordinate(
        position,
        &coordinate
    );

    if (FAILED(hr)) {
        snprintf(out->exception_value, sizeof(out->exception_value), "get_Coordinate = 0x%08lX\n", (unsigned long)hr);
        goto cleanup;
    }

    /* ========================================================
       Latitude
       ======================================================== */

    hr = coordinate->lpVtbl->get_Latitude(coordinate, &latitude);

    if (FAILED(hr)) {
        snprintf(out->exception_value, sizeof(out->exception_value), "get_Latitude = 0x%08lX\n", (unsigned long)hr);
        goto cleanup;
    }


    /* ========================================================
       Longitude
       ======================================================== */

    hr = coordinate->lpVtbl->get_Longitude(coordinate, &longitude);

    if (FAILED(hr)) {
        snprintf(out->exception_value, sizeof(out->exception_value), "get_Longitude = 0x%08lX\n", (unsigned long)hr);
        goto cleanup;
    }

    /* ========================================================
       Précision
       ======================================================== */

    hr = coordinate->lpVtbl->get_Accuracy(coordinate, &accuracy);

    if (FAILED(hr))
        accuracy = -1.0;

    //printf("\n=================================\n");
    //printf("Latitude  : %.10f\n", latitude);
    //printf("Longitude : %.10f\n", longitude);
    //printf("Precision : %.2f m\n", accuracy);
    //printf("=================================\n");

    cleanup:

    if (coordinate) coordinate->lpVtbl->Release(coordinate);
    if (position) position->lpVtbl->Release(position);

    if (geoOperation) geoOperation->lpVtbl->Release(geoOperation);

    if (geolocator) geolocator->lpVtbl->Release(geolocator);

    if (accessOperation) accessOperation->lpVtbl->Release(accessOperation);

    if (statics) statics->lpVtbl->Release(statics);

    if (className) pWindowsDeleteString(className);

    pRoUninitialize();

    //CoUninitialize();

    FreeLibrary(dll);

    // sortie final :
    out->hr = hr;
    out->latitude  = latitude;
    out->longitude = longitude;
    out->accuracy  = accuracy;

    // Fin du thread UI
    PostQuitMessage(0);
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }

    return 0;
}

/* ============================================================
   DLL API
   ============================================================ */

//process qui récupère la localisation
__declspec(dllexport)
int geo_process(void) {
    HANDLE hThread;
    DWORD threadId;

    // Réinitialiser le résultat précédent.
    memset(&global_result, 0, sizeof(global_result));

    global_result.hr = S_OK;

    hThread = CreateThread(
        NULL,
        0,
        UiThreadProc,
        &global_result,
        0,
        &threadId
    );

    if (!hThread) {
        global_result.hr = HRESULT_FROM_WIN32(GetLastError());
        snprintf(global_result.exception_value, sizeof(global_result.exception_value), "CreateThread failed : 0x%08lX", (unsigned long)global_result.hr);
        return 0;
    }

    WaitForSingleObject(hThread,INFINITE);

    CloseHandle(hThread);

    //Vérifier le résultat.
    if (FAILED(global_result.hr))
        return 0;

    return 1;
}

__declspec(dllexport)
HRESULT geo_get_hr(void) {
    return global_result.hr;
}

__declspec(dllexport)
const char *geo_get_error(void) {
    return global_result.exception_value;
}

__declspec(dllexport)
double geo_get_latitude(void) {
    return (double)global_result.latitude;
}

__declspec(dllexport)
double geo_get_longitude(void) {
    return (double)global_result.longitude;
}

__declspec(dllexport)
double geo_get_accuracy(void) {
    return (double)global_result.accuracy;
}

__declspec(dllexport)
int geo_view(void) {
    //Vérifier le résultat.
    if (FAILED(global_result.hr)) {
        printf("%s", global_result.exception_value);
        fflush(stderr);
        return 0;
    }
    printf("\n=================================\n");
    printf("Latitude  : %.10f\n", global_result.latitude);
    printf("Longitude : %.10f\n", global_result.longitude);
    printf("Precision : %.2f m\n", global_result.accuracy);
    printf("=================================\n");
    fflush(stdout);
    return 1;
}
