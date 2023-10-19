#include <iostream>
#include <array>
#include <ctime>
#include <functional>
#include <QDebug>
#include "mainwindow.h"
#include "MTLtask.h"

class MtlEmulator : public QThread
{
    GDSCallback callback = nullptr;
public:
    ~MtlEmulator()
    {
        terminate();
    }

    void registerCallback(GDSCallback callback)
    {
        this->callback = callback;
    }

    void run()
    {
        CTRL_CHANNEL_STATUS CCS;
        memset(&CCS, 0, sizeof(CCS));

        while (true)
        {
            std::vector<const char*> names;

            if (rand() % 2)
                names = { "Time1", "Strain", "Load", "Strain1", "CY-Actuator", "Strain2"/*, "Stoke"*/};
            else
                names = { "Time1", "Strain", "Load", "Strain1", "CY-Actuator"};

            CCS.uLogChannels = names.size();


            for (int i = 0; i < CCS.uLogChannels; i++)
                strcpy_s(CCS.sChID[i], names[i]);

            CCS.fReadout[0] = time(nullptr) % 10000;
            auto sin_time = sin(CCS.fReadout[0] / 4.);// -0.5;

            for (int i = 1; i < CCS.uLogChannels; i++)
            {
                if (strcmp(CCS.sChID[i], "CY-Actuator") == 0)
                    CCS.fReadout[i] += (sin_time < 0 ? 0 : 2);
                else
                    CCS.fReadout[i] = std::rand();
            }

            //CCS.bSGIdle = std::rand() % 2;
            CCS.bSGIdle = MtlDataModel::kBusy;

            if (callback)
                callback(&CCS);

            msleep(250);
        }
    }
};

/// @brief class MTL32
bool MTL32::connected = false;
HMODULE MTL32::hMTL32 = 0;
PGDSRegisterStation           MTL32::_GDSRegisterStation = 0;
PGDSUnregisterStation         MTL32::_GDSUnregisterStation = 0;
PGDSRegisterDAQRequest        MTL32::_GDSRegisterDAQRequest = 0;
PGDSRegisterVBDAQRequest      MTL32::_GDSRegisterVBDAQRequest = 0;
PGDSRemoveDAQRequest          MTL32::_GDSRemoveDAQRequest = 0;
PGDSInitializePacketPointer   MTL32::_GDSInitializePacketPointer = 0;
PGDSReturnVariableTypeOfLOGCh MTL32::_GDSReturnVariableTypeOfLOGCh = 0;
PGDSNextLongPoint             MTL32::_GDSNextLongPoint = 0;
PGDSNextFloatPoint            MTL32::_GDSNextFloatPoint = 0;
PGDSCycleBlockControlSegment  MTL32::_GDSCycleBlockControlSegment = 0;
PGDSPresetCycleCount          MTL32::_GDSPresetCycleCount = 0;
PGDSResetControlWaveform      MTL32::_GDSResetControlWaveform = 0;
PGDSPeakValleySegment         MTL32::_GDSPeakValleySegment = 0;
PGDSSwitchControlMode         MTL32::_GDSSwitchControlMode = 0;

MTL32::MTL32()
{
    hMTL32 = LoadLibrary("MTL32Calls.dll");
    if (hMTL32 == 0) return;
    _GDSRegisterStation = (PGDSRegisterStation)GetProcAddress(hMTL32, "GDSRegisterStation");
    _GDSUnregisterStation = (PGDSUnregisterStation)GetProcAddress(hMTL32, "GDSUnregisterStation");
    _GDSRegisterDAQRequest = (PGDSRegisterDAQRequest)GetProcAddress(hMTL32, "GDSRegisterDAQRequest");
    _GDSRegisterVBDAQRequest = (PGDSRegisterVBDAQRequest)GetProcAddress(hMTL32, "GDSRegisterVBDAQRequest");
    _GDSRemoveDAQRequest = (PGDSRemoveDAQRequest)GetProcAddress(hMTL32, "GDSRemoveDAQRequest");

    _GDSInitializePacketPointer = (PGDSInitializePacketPointer)GetProcAddress(hMTL32, "GDSInitializePacketPointer");
    _GDSReturnVariableTypeOfLOGCh = (PGDSReturnVariableTypeOfLOGCh)GetProcAddress(hMTL32, "GDSReturnVariableTypeOfLOGCh");
    _GDSNextLongPoint = (PGDSNextLongPoint)GetProcAddress(hMTL32, "GDSNextLongPoint");
    _GDSNextFloatPoint = (PGDSNextFloatPoint)GetProcAddress(hMTL32, "GDSNextFloatPoint");

    _GDSCycleBlockControlSegment = (PGDSCycleBlockControlSegment)GetProcAddress(hMTL32, "GDSCycleBlockControlSegment");
    _GDSPresetCycleCount = (PGDSPresetCycleCount)GetProcAddress(hMTL32, "GDSPresetCycleCount");
    _GDSResetControlWaveform = (PGDSResetControlWaveform)GetProcAddress(hMTL32, "GDSResetControlWaveform");
    _GDSPeakValleySegment = (PGDSPeakValleySegment)GetProcAddress(hMTL32, "GDSPeakValleySegment");
    _GDSSwitchControlMode = (PGDSSwitchControlMode)GetProcAddress(hMTL32, "GDSSwitchControlMode");
}

MTL32::~MTL32()
{
    disconnectStation();
    FreeLibrary(hMTL32);
}

bool MTL32::isLoaded()
{
    return hMTL32 ? true : false;
}

bool MTL32::isConnected()
{
    return connected;
}

int MTL32::connectStation(const QString& station)
{
    return GDSRegisterStation(
        station.toStdString().c_str(),
        "Demo",
        0,
        MtlDataModel::Callback);
}

void MTL32::disconnectStation()
{
    if (!isConnected()) return;
    GDSUnregisterStation();
}

// end class MTL32

/// @brief class MtlDataModel

MTL32               MtlDataModel::_MTL32;
QReadWriteLock      MtlDataModel::data_lock;
QVector<QVariant>   MtlDataModel::mtl_values;
QVector<QString>    MtlDataModel::mtl_names;
QMutex              MtlDataModel::instance_lock;
MtlDataModel*       MtlDataModel::instance;
QAtomicInt          MtlDataModel::on_connect          = true;
QAtomicInt          MtlDataModel::power_on            = false;
QAtomicInt          MtlDataModel::power_status_ignore = false;
QAtomicInt          MtlDataModel::machine_state       = MtlDataModel::kIdle;
QAtomicInt          MtlDataModel::mtltask_state       = MtlDataModel::kBusy;

MtlDataModel::MtlDataModel(QObject* parent) :
    QAbstractTableModel(parent)
{
    QMutexLocker locker(&instance_lock);
    instance = this;
    connect(this, &MtlDataModel::dataReceived, this, &MtlDataModel::dataChanged, Qt::QueuedConnection);
    connect(this, &MtlDataModel::headerChanged, this, &MtlDataModel::headerDataChanged, Qt::QueuedConnection);
}

MtlDataModel::~MtlDataModel()
{
    QMutexLocker locker(&instance_lock);
    instance = nullptr;
}

int MtlDataModel::rowCount(const QModelIndex& parent) const
{
    return 1;
}

int MtlDataModel::columnCount(const QModelIndex& parent) const
{
    QReadLocker lock(&data_lock);
    return mtl_names.size();
}

QVariant MtlDataModel::data(const QModelIndex& index, int role) const
{
    QReadLocker lock(&data_lock);
    if (role == Qt::EditRole && 0 <= index.column() && index.column() < mtl_values.size())
    {
        data_is_editing = true;
        editing_column = index.column();
        return mtl_values[index.column()];
    }
    if ((role == Qt::DisplayRole) && 0 <= index.column() && index.column() < mtl_values.size())
        return mtl_values[index.column()];
    else
        return QVariant();
}

bool MtlDataModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (role == Qt::EditRole && 0 <= index.column() && index.column() < mtl_values.size())
    {
        if (mtl_names[index.column()].contains("CY-Actuator"))
            MTL32::GDSPresetCycleCount(0, value.toInt());
        data_is_editing = false;
        editing_column = -1;
    }

    return false;
}

QVariant MtlDataModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    QReadLocker lock(&data_lock);
    if (role == Qt::DisplayRole && 0 <= section && section < mtl_names.size())
        return mtl_names[section];
    else
        return QVariant();
}

Qt::ItemFlags MtlDataModel::flags(const QModelIndex& index) const
{
    if (0 <= index.column() && index.column() < mtl_values.size())
    {
        if (mtl_names[index.column()].contains("CY-Actuator"))
            return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    }
    return QAbstractTableModel::flags(index);
}

QVector<QString> MtlDataModel::getVariables()
{
    QReadLocker lock(&data_lock);
    return mtl_names;
}

bool MtlDataModel::getValue(const QString& variable, QString & value)
{
    QReadLocker lock(&data_lock);
    int idx = mtl_names.indexOf(variable);
    if (idx < 0)
    {
        return false;
    }
    else
    {
        value = mtl_values[idx].toString();
        return true;
    }
}

int MtlDataModel::Callback(CTRL_CHANNEL_STATUS* ccs)
{
    if (ccs == nullptr) return 0;

    static int log_channels = ccs->uLogChannels;
    bool is_header_changed = false;
    bool is_machine_state_changed = false;

    data_lock.lockForWrite();
    // check if channels count has changed
    if (log_channels != ccs->uLogChannels || mtl_values.isEmpty() || mtl_names.isEmpty())
    {
        is_header_changed = true;
        log_channels = ccs->uLogChannels;
        mtl_values.resize(log_channels);
        mtl_names.resize(log_channels);
    }

    for (int i = 0; i < ccs->uLogChannels; i++)
    {
        mtl_names[i] = QString(ccs->sChID[i]);
        mtl_values[i] = QVariant(ccs->fReadout[i]);
    }

    is_machine_state_changed = machine_state != ccs->bSGIdle;

    power_on = ccs->bPowerOn;
    machine_state = ccs->bSGIdle;

    data_lock.unlock();

    QMutexLocker instance_locker(&instance_lock);
    if (instance != nullptr)
    {
        if (!instance->data_is_editing || instance->editing_column == -1)
        {
            emit instance->dataReceived(instance->index(0, 0), instance->index(0, ccs->uLogChannels - 1));
        }
        else
        {
            if (instance->editing_column > 0)
                emit instance->dataReceived(instance->index(0, 0), instance->index(0, instance->editing_column-1));
            if (instance->editing_column < ccs->uLogChannels - 1)
                emit instance->dataReceived(instance->index(0, instance->editing_column + 1), instance->index(0, ccs->uLogChannels - 1));
        }
        if (is_header_changed || on_connect)
            emit instance->headerChanged(Qt::Orientation::Horizontal, 0, ccs->uLogChannels - 1);
        if (is_machine_state_changed || on_connect)
            emit instance->machineStateChanged(machine_state);
        if (on_connect)
            on_connect = false;
    }

    return 0;
}

/// @brief MtlController
MtlController::MtlController(bool emulation) 
    : mtl_data_()
    , is_emulation_(emulation)
{
}

MtlDataModel* MtlController::model()
{
    return &mtl_data_;
}

void MtlController::connectStation(const QString& station)
{
    if (is_emulation_) {
        // --- emulate mtl
        static MtlEmulator mtl_emulator;
        mtl_emulator.registerCallback(MtlDataModel::Callback);
        mtl_emulator.start();
        // ---
    }
    else {
        MTL32::GDSRegisterStation(station.toStdString().c_str(),
            "Demo",
            0,
            MtlDataModel::Callback);
    }
}

void MtlController::disconnectStation()
{
    if (!MTL32::isConnected()) return;
    MTL32::GDSUnregisterStation();
}

bool MtlController::checkPower()
{
    if (MtlDataModel::power_status_ignore)
        return true;
    return MtlDataModel::power_on;
}

bool MtlController::isEmulation() const
{
    return is_emulation_;
}
