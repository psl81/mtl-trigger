#ifndef MTLTASK_H
#define MTLTASK_H

#include <QThread>
#include <QReadWriteLock>
#include <QVector>
#include <QAbstractTableModel>
#include <QMutex>
#include <windows.h>
#include <winbase.h>

#define LOG_CHANNELS 145
#define U_WORD int
#define _export __declspec(dllimport)

extern "C"
{
#include "MTL/MTL32dllH.h"
}

typedef int(__stdcall* PGDSRegisterStation)(const char* sPort, const char* sPanel, int iStation, GDSCallback lpGDSTaskCall);
typedef int(__stdcall* PGDSUnregisterStation)(void);

typedef int(__stdcall* PGDSRegisterDAQRequest)(unsigned uCtrlCh, unsigned uDataType, unsigned uStartCycCount,
    unsigned uNoOfCycles, unsigned uEndPts, SYSTEMTIME StartTime, unsigned uEndTicks, GDSLogProc lpCallback);
typedef int(__stdcall* PGDSRegisterVBDAQRequest)(unsigned uCtrlCh, unsigned uDataType, unsigned uStartCycCount,
    unsigned uNoOfCycles, unsigned uEndPts, unsigned uEndTicks, GDSLogProc lpCallback);
typedef int(__stdcall* PGDSRemoveDAQRequest)(int uIDCode);

typedef int(__stdcall* PGDSInitializePacketPointer) (int* lpFrom);
typedef int(__stdcall* PGDSReturnVariableTypeOfLOGCh)(unsigned uCtrlCh, unsigned uCh);
typedef int(__stdcall* PGDSNextLongPoint)();
typedef float(__stdcall* PGDSNextFloatPoint)();

typedef int(__stdcall* PGDSCycleBlockControlSegment)(int uCtrlCh, int iMode, float fMean, float fAmplitude, float fFrequency,
    unsigned uQCycles, unsigned uQuadrant, unsigned uWaveform, BOOL bRelative,
    BOOL bAdaptiveControl, BOOL bConstantRate, BOOL bCycleCount, BOOL bIncBlockCount);

typedef int(__stdcall* PGDSPresetCycleCount)(int uCtrlCh, unsigned uCount);
typedef int(__stdcall* PGDSResetControlWaveform)(int uCtrlCh);
typedef int(__stdcall* PGDSPeakValleySegment)(int uCtrlCh, float fToSetPoint, float fGain, float fFrequency, unsigned uWaveform,
    int bRelative, int bConstantRate);

typedef int(__stdcall* PGDSSwitchControlMode)(int uCtrlCh, int iMode);

class MTL32
{
    static bool connected;
    static HMODULE hMTL32;
    static PGDSRegisterStation           _GDSRegisterStation;
    static PGDSUnregisterStation         _GDSUnregisterStation;
    static PGDSRegisterDAQRequest        _GDSRegisterDAQRequest;
    static PGDSRemoveDAQRequest          _GDSRemoveDAQRequest;
    static PGDSInitializePacketPointer   _GDSInitializePacketPointer;
    static PGDSReturnVariableTypeOfLOGCh _GDSReturnVariableTypeOfLOGCh;
    static PGDSNextLongPoint             _GDSNextLongPoint;
    static PGDSNextFloatPoint            _GDSNextFloatPoint;
    static PGDSRegisterVBDAQRequest      _GDSRegisterVBDAQRequest;
    static PGDSCycleBlockControlSegment  _GDSCycleBlockControlSegment;
    static PGDSPresetCycleCount          _GDSPresetCycleCount;
    static PGDSResetControlWaveform      _GDSResetControlWaveform;
    static PGDSPeakValleySegment         _GDSPeakValleySegment;
    static PGDSSwitchControlMode         _GDSSwitchControlMode;


public:
    enum ControlMode { kStroke = 0, kLoad = 1, kStrain = 2 };
    enum WaveForm { kSine = 0, kRamp = 1, kPulse = 2 };

    MTL32();
    ~MTL32();

    static bool isLoaded();
    static bool isConnected();
    static int connectStation(const QString& station);
    static void disconnectStation();

    static int GDSRegisterStation(const char* sPort, const char* sPanel, int iStation, GDSCallback lpGDSTaskCall)
    {
        int ret = 0;
        //GDSRegisterStation((char*)sPort, (char*)sPanel, iStation, lpGDSTaskCall);
        if (_GDSRegisterStation)
        {
            ret = _GDSRegisterStation(sPort, sPanel, iStation, lpGDSTaskCall);
            if (ret == 0)
                connected = true;
        }
        return ret;
    }

    static int GDSUnregisterStation()
    {
        int ret = 0;
        if (_GDSUnregisterStation)
            ret = _GDSUnregisterStation();
        connected = false;
        return ret;
    }

    static int GDSRegisterDAQRequest(unsigned uCtrlCh, unsigned uDataType, unsigned uStartCycCount,
        unsigned uNoOfCycles, unsigned uEndPts, SYSTEMTIME StartTime, unsigned uEndTicks, GDSLogProc lpCallback)
    {
        if (_GDSRegisterDAQRequest)
            return _GDSRegisterDAQRequest(uCtrlCh, uDataType, uStartCycCount,
                uNoOfCycles, uEndPts, StartTime, uEndTicks, lpCallback);
        else
            return 0;
    }

    static int GDSRegisterVBDAQRequest(unsigned uCtrlCh, unsigned uDataType, unsigned uStartCycCount,
        unsigned uNoOfCycles, unsigned uEndPts, unsigned uEndTicks, GDSLogProc lpCallback)
    {
        if (_GDSRegisterVBDAQRequest)
            return _GDSRegisterVBDAQRequest(uCtrlCh, uDataType, uStartCycCount,
                uNoOfCycles, uEndPts, uEndTicks, lpCallback);
        else
            return 0;
    }

    static int GDSRemoveDAQRequest(int uIDCode)
    {
        return _GDSRemoveDAQRequest ? _GDSRemoveDAQRequest(uIDCode) : 0;
    }

    static int GDSInitializePacketPointer(int* lpFrom)
    {
        return _GDSInitializePacketPointer ?
            _GDSInitializePacketPointer(lpFrom) : 0;
    }

    static int GDSReturnVariableTypeOfLOGCh(unsigned uCtrlCh, unsigned uCh)
    {
        return _GDSReturnVariableTypeOfLOGCh ?
            _GDSReturnVariableTypeOfLOGCh(uCtrlCh, uCh) : 0;
    }

    static int GDSNextLongPoint()
    {
        return _GDSNextLongPoint ? _GDSNextLongPoint() : 0;
    }

    static float GDSNextFloatPoint()
    {
        return _GDSNextFloatPoint ? _GDSNextFloatPoint() : 0;
    }

    static int GDSCycleBlockControlSegment(int uCtrlCh, int iMode, float fMean, float fAmplitude, float fFrequency,
        unsigned uQCycles, unsigned uQuadrant, unsigned uWaveform, BOOL bRelative,
        BOOL bAdaptiveControl, BOOL bConstantRate, BOOL bCycleCount, BOOL bIncBlockCount)
    {
        if (_GDSCycleBlockControlSegment)
            return _GDSCycleBlockControlSegment(uCtrlCh, iMode, fMean, fAmplitude, fFrequency,
                uQCycles, uQuadrant, uWaveform, bRelative,
                bAdaptiveControl, bConstantRate, bCycleCount, bIncBlockCount);
        else
            return 0;
    }

    static int GDSPresetCycleCount(int uCtrlCh, unsigned uCount)
    {
        if (_GDSPresetCycleCount)
            return _GDSPresetCycleCount(uCtrlCh, uCount);
        else
            return 0;
    }

    static int GDSResetControlWaveform(int uCtrlCh)
    {
        if (_GDSResetControlWaveform)
            return _GDSResetControlWaveform(uCtrlCh);
        else
            return 0;
    }

    static int GDSPeakValleySegment(int uCtrlCh, float fToSetPoint, float fGain, float fFrequency, unsigned uWaveform,
        int bRelative, int bConstantRate)
    {
        if (_GDSPeakValleySegment)
            return _GDSPeakValleySegment(uCtrlCh, fToSetPoint, fGain, fFrequency, uWaveform, bRelative, bConstantRate);
        else
            return 0;
    }

    static int GDSSwitchControlMode(int uCtrlCh, int iMode)
    {
        if (_GDSSwitchControlMode)
            return _GDSSwitchControlMode(uCtrlCh, iMode);
        else
            return 0;
    }
};

/// @brief class MtlDataModel
/// MTL32 and TS data manager
class MtlDataModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum MachineState { kBusy = 0, kIdle = 1 };

    MtlDataModel(QObject* parent = nullptr);
    ~MtlDataModel();
    virtual int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    virtual QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    virtual bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;

    QVector<QString> getVariables();
    bool getValue(const QString & variable, QString& value);

    static int __stdcall Callback(CTRL_CHANNEL_STATUS* ccs);

    static QAtomicInt           power_on;
    static QAtomicInt           power_status_ignore;
    static QAtomicInt           machine_state;
    static QAtomicInt           mtltask_state;
private:
    static  MTL32               _MTL32;
    static  QReadWriteLock      data_lock;
    static  QVector<QVariant>   mtl_values;
    static  QVector<QString>    mtl_names;
    static  QMutex              instance_lock;
    static  MtlDataModel*       instance;
    static  QAtomicInt          on_connect;
    mutable QAtomicInt          data_is_editing = false;
    mutable QAtomicInt          editing_column = -1;

signals:
    void dataReceived(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles = { Qt::DisplayRole });
    void headerChanged(Qt::Orientation orientation, int first, int last);
    void machineStateChanged(bool state);
};

class MtlController
{
public:
    MtlController();
    MtlDataModel* model();
    void connectStation(const QString& station);
    void disconnectStation();
    static bool checkPower();
private:
    MtlDataModel mtl_data;
};

#endif // MTLTASK_H
