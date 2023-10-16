#ifndef TASK_H
#define TASK_H

#include <QThread>
#include <QAbstractItemModel>
#include <QFormLayout>
#include <QJsonObject>
#include <QTableView>
//
#include "MTLtask.h"
#include "keyvaluemodel.h"
#include "task_property.h"

/// @brief class Task
/// base class for task objects
class Task : public QThread
{
    Q_OBJECT
public:
    Task(Task* parent = nullptr);
    ~Task();
    // create gui form
    // создание графической формы
    // функция должна быть реализована для класса задачи
    // форма служит для отображения/изменения свойств задачи
    virtual void createForm(QFormLayout* layout) = 0;
    // initialize / finalize task
    // usually uses for open/close camera/port
    virtual void initialize();
    virtual void finalize();
    // write/read task properties
    virtual void write(QJsonObject& json);
    virtual void read(const QJsonObject& json);
    //
    virtual const QString& taskname() = 0;
    virtual int type();
    virtual QString information() = 0;
    //
    virtual QStringList getLocalVariables();
    QStringList getVariables();
    virtual QString replaceLocalNamesWithValues(const QString& line);
    QString replaceNamesWithValues(const QString& line);
    virtual bool getValue(const QString & variable, QString & value);
    //
    Task* duplicate();
    void appendChild(Task* child);
    void insertChild(int row, Task* child);
    void removeChild(int row);
    void eraseChild(int row);
    void eraseChilds();
    void move(Task* src_parent, Task* dst_parent, int row);
    Task* child(int row);
    int childCount() const;
    int row() const;
    Task* parentItem();
    //
    QModelIndex        index;
    static QAtomicInt  break_loop;
    // methods & variables to control start from a selected item
    bool checkSkipping();
    static QModelIndex start_index;
    static QAtomicInt  skip_tasks;
    
protected:
    Task* rootItem();
    void  connectPropertiesToRoot(const QList<const TaskProperty*> & properties);
    void  connectToRoot();
    float propertyToFloat(const StringProperty<ToggleBox<QDoubleSpinBox>>& property);
    int   propertyToInt(const StringProperty<ToggleBox<QSpinBox>>& property);
    //
    std::list<TaskProperty*> properties;
private:
    QVector<Task*> child_tasks;
    Task *         parent_task;
signals:
    void changed(const QModelIndex & index);
};

/// @brief class TaskFabric
/// фабрика классов
class TaskFabric
{
protected:
    static QMap<QString, Task* (*)(Task* parent)> tasks;

public:
    /// @brief create
    /// создает объект задачи
    /// @param task_type - название задачи
    /// @return 
    static Task* create(const QString& task_type, Task* parent = nullptr);

    /// @brief listTaskName
    /// returns task list
    /// @return 
    static QStringList listTaskName();
};

/// @brief class TaskCreator
/// @tparam  
template<typename TaskType>
class TaskCreator : public TaskFabric
{
protected:
    static Task* create(Task* parent = nullptr)
    {
        return new TaskType(parent);
    }

    static QString addTaskName(const QString& name)
    {
        tasks.insert(name, create);
        return name;
    }

    static const QString name;
};

class BlockItem : public Task
{
    Q_OBJECT
public:
    BlockItem(Task* parent = nullptr);

    virtual void write(QJsonObject& json) override;
    virtual void read(const QJsonObject& json) override;
    virtual int type() override;
    void sendStartItem(const QModelIndex& index);
    virtual QString information() override;

signals:
    void itemStarted(const QModelIndex & index);
};

class QTasksModel;

/// @brief class RootBlock
/// @param  
class RootBlock : public BlockItem
{
    Q_OBJECT
public:
    RootBlock(QTasksModel * model); // TODO remove model from here
    virtual void createForm(QFormLayout* layout) override;
    virtual const QString& taskname() override;
    virtual QStringList getLocalVariables() override;
    virtual bool getValue(const QString& variable, QString& value) override;

    MtlController mtl_controller;
protected:
    // execute task
    virtual void run() override;
    QTasksModel * model;
};

/// @brief class ParallelBlock
class ParallelBlock : public BlockItem, public TaskCreator<ParallelBlock>
{
    KeyValueModel   keyvalue;
    QTableView    * table;
public:
    ParallelBlock(Task* parent = nullptr);
    virtual void createForm(QFormLayout* layout) override;
    virtual const QString& taskname() override;
    virtual QStringList getLocalVariables() override;
    virtual bool getValue(const QString& variable, QString& value) override;
    // read/write
    virtual void write(QJsonObject& json) override;
    virtual void read(const QJsonObject& json) override;
protected:
    // execute task
    virtual void run() override;
};

/// @brief class ForLoopBlock 
class ForLoopBlock : public BlockItem, public TaskCreator<ForLoopBlock>
{
public:
    ForLoopBlock(Task* parent = nullptr);
    virtual void createForm(QFormLayout* layout) override;
    virtual const QString& taskname() override;
    virtual QString information() override;
    virtual QStringList getLocalVariables() override;
    virtual bool getValue(const QString& variable, QString& value) override;
protected:
    // execute task
    virtual void run() override;

    StringProperty<QLineEdit>  variable_name;
    NumProperty<int>           from;
    NumProperty<int>           to;
    NumProperty<int>           step;
    int                        loop_counter;
};

/// @brief class ForListBlock
class ForListBlock : public BlockItem, public TaskCreator<ForListBlock>
{
public:
    ForListBlock(Task* parent = nullptr);
    virtual void createForm(QFormLayout* layout) override;
    virtual const QString& taskname() override;
    virtual QString information() override;
    virtual QStringList getLocalVariables() override;
    virtual bool getValue(const QString& variable, QString& value) override;

protected:
    // execute task
    virtual void run() override;
    void addPoint();

    StringProperty<QLineEdit>  variable_name;
    StringListProperty         values_list;
    QDoubleSpinBox*            point_sb;
    int                        loop_counter;
};

/// @brief class InfiniteLoopBlock
class InfiniteLoopBlock : public BlockItem, public TaskCreator<InfiniteLoopBlock>
{
public:
    InfiniteLoopBlock(Task* parent = nullptr);
    virtual void createForm(QFormLayout* layout) override;
    virtual const QString& taskname() override;

protected:
    // execute task
    virtual void run() override;
};

class QTasksModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    enum ItemType { kBlock = 1, kTask };

    explicit QTasksModel(QObject* parent = nullptr);
    ~QTasksModel();

    QVariant data(const QModelIndex& index, int role) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    Task* itemFromIndex(const QModelIndex& index) const;
    QModelIndex createIndex(int row, int column, Task* task) const;

    Qt::DropActions supportedDropActions() const override;
    virtual QStringList mimeTypes() const override;
    virtual QMimeData* mimeData(const QModelIndexList& indexes) const override;
    virtual bool canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const override;
    virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) override;

    void addTask(const QModelIndex& index, const QString& task_name);
    void remove(const QModelIndexList& index_list);

    void highlightItem(const QModelIndex& index = QModelIndex());

    RootBlock& rootBlock();

    void save(const QString& filename);
    void load(const QString& filename);

private:
    RootBlock* rootItem;
    QModelIndex highlighted_index = QModelIndex();
};

#endif // TASK_H
