#include <QString>
#include <QLabel>
#include <QMimeData>
#include <QFile>
#include <QJsonDocument>
#include <QPushButton>
#include "task.h"

/// @brief class TaskFabric
QMap<QString, Task* (*)(Task* parent)> TaskFabric::tasks;

// class fabric
Task* TaskFabric::create(const QString& task_name, Task* parent)
{
    if (tasks.contains(task_name))
        return tasks[task_name](parent);
    else
        return nullptr;
}

QStringList TaskFabric::listTaskName()
{
    return tasks.keys();
}

/// @brief class Task
QAtomicInt  Task::break_loop  = false;
QModelIndex Task::start_index = QModelIndex();
QAtomicInt  Task::skip_tasks  = false;

Task::Task(Task* parent) :
    parent_task(parent)
{
}

Task::~Task()
{
    qDeleteAll(child_tasks);
}

void Task::initialize()
{
}

void Task::finalize()
{
}

void Task::write(QJsonObject& json)
{
    json["tasktype"] = QJsonValue(taskname());
    for (const TaskProperty* p : properties)
        p->toJson(json);
}

void Task::read(const QJsonObject& json)
{
    for (TaskProperty* p : properties)
        p->fromJson(json);
}

int Task::type()
{
    return QTasksModel::kTask;
}

QStringList Task::getLocalVariables()
{
    return QStringList();
}

QStringList Task::getVariables()
{
    QStringList variables;
    for (Task* task = this; task != nullptr; task = task->parentItem())
    {
        variables << task->getLocalVariables();
    }
    return variables;
}

QString Task::replaceLocalNamesWithValues(const QString& template_string)
{
    QString output;
    QString var_name;
    bool is_brace_opened = false;
    for (const QChar& symbol : template_string)
    {
        if (symbol == '{')
        {
            is_brace_opened = true;
            var_name.clear();
        }
        else if (is_brace_opened && symbol == '}')
        {
            QString value;
            if (getValue(var_name, value))
                output += value;
            else
                output += '{' + var_name + '}';
            is_brace_opened = false;
            var_name.clear();
        }
        else if (is_brace_opened)
        {
            var_name += symbol;
        }
        else if (!is_brace_opened)
        {
            output += symbol;
        }
    }

    return output;
}

QString Task::replaceNamesWithValues(const QString& line)
{
    QString output_line = line;
    for (Task* task = this; task != nullptr; task = task->parentItem())
    {
        output_line = task->replaceLocalNamesWithValues(output_line);
    }
    return output_line;
}

bool Task::getValue(const QString & variable, QString & value)
{
    value = "";
    return false;
}

Task* Task::duplicate()
{
    Task* task = TaskFabric::create(this->taskname(), this->parent_task);
    QJsonObject json;
    this->write(json);
    task->read(json);
    return task;
}

void Task::appendChild(Task* child)
{
    child_tasks.append(child);
    child->parent_task = this;
}

void Task::insertChild(int row, Task* child)
{
    if (row < 0)
        row = 0;
    if (row > child_tasks.count())
        row = child_tasks.count();
    child_tasks.insert(row, child);
    child->parent_task = this;
}

void Task::removeChild(int row)
{
    child_tasks.remove(row);
}

void Task::eraseChild(int row)
{
    child(row)->disconnect();
    delete child(row);
    removeChild(row);
}

void Task::eraseChilds()
{
    int count = childCount();
    for (int i = 0; i < count; i++)
        eraseChild(0);
}

void Task::move(Task* src_parent, Task* dst_parent, int row)
{
    int old_row = this->row();
    if (old_row < row)
    {
        dst_parent->insertChild(row, this);
        src_parent->removeChild(old_row);
    }
    else
    {
        src_parent->removeChild(old_row);
        dst_parent->insertChild(row, this);
    }
}

Task* Task::child(int row)
{
    if (row < 0 || row >= child_tasks.size())
        return nullptr;
    return child_tasks.at(row);
}

int Task::childCount() const
{
    return child_tasks.count();
}

int Task::row() const
{
    if (parent_task)
        return parent_task->child_tasks.indexOf(const_cast<Task*>(this));

    return 0;
}

Task* Task::parentItem()
{
    return parent_task;
}

Task* Task::rootItem()
{
    Task* item = this;
    while (item->parent_task)
        item = item->parent_task;
    return item;
}

void Task::connectPropertiesToRoot(const QList<const TaskProperty*>& properties)
{
    Task* root_task = rootItem();
    if (root_task)
    {
        for (const TaskProperty* property : properties)
            connect(property, &TaskProperty::changed, root_task,
                [=]()
                {
                    root_task->changed(this->index);
                });
    }
}

void  Task::connectToRoot()
{
    Task* root_task = rootItem();
    if (root_task)
        connect(this, &Task::changed, root_task, &Task::changed);
}

bool Task::checkSkipping()
{
    if (Task::skip_tasks && index == start_index)
        Task::skip_tasks = false;
    return Task::skip_tasks;
}

float Task::propertyToFloat(const StringProperty<ToggleBox<QDoubleSpinBox>> & property)
{
    QString v = property.value;
    if (property.is_variable)
        v = replaceNamesWithValues("{" + v + "}");
    return v.toFloat();
}

int Task::propertyToInt(const StringProperty<ToggleBox<QSpinBox>>& property)
{
    QString v = property.value;
    if (property.is_variable)
        v = replaceNamesWithValues("{" + v + "}");
    return int(v.toDouble());
}

/// @brief class BlockItem
//
BlockItem::BlockItem(Task* parent) :
    Task(parent)
{
}

int BlockItem::type()
{
    return QTasksModel::kBlock;
}

void BlockItem::write(QJsonObject& json)
{
    int digit_number = log10(childCount()) + 1;
    json["tasktype"] = QJsonValue(taskname());
    for (int i = 0; i < childCount(); i++)
    {
        Task* task = child(i);
        QString task_name = task->taskname(); // TODO: set to user text
        QJsonObject itemObject;
        task->write(itemObject);
        json[QString("%1").arg(i, digit_number, 10, QChar('0'))] = itemObject;
    }
    Task::write(json);
}

void BlockItem::read(const QJsonObject& json)
{
    for (QJsonObject::const_iterator it = json.begin(); it != json.end(); ++it)
    {
        QJsonObject item_json = it.value().toObject();
        Task* task = TaskFabric::create(item_json["tasktype"].toString(), this);
        if (task)
        {
            task->read(item_json);
            appendChild(task);
        }
    }
    Task::read(json);
}

void BlockItem::sendStartItem(const QModelIndex& index)
{
    BlockItem* root = dynamic_cast<BlockItem*>(parentItem());
    while (root->parentItem()) 
        root = dynamic_cast<BlockItem*>(root->parentItem());
    emit root->itemStarted(index);
}

QString BlockItem::information()
{
    return taskname();
}

/// @brief class RootBlock
RootBlock::RootBlock(QTasksModel* model) 
    : BlockItem(nullptr)
    , model(model)
{
}

void RootBlock::createForm(QFormLayout* layout)
{
}

const QString& RootBlock::taskname()
{
    static QString name("RootBlock");
    return name;
}

QStringList RootBlock::getLocalVariables()
{
    return mtl_controller.model()->getVariables().toList();
}

bool RootBlock::getValue(const QString& variable, QString& value)
{
    return mtl_controller.model()->getValue(variable, value);
}

void RootBlock::run()
{
    for (int i = 0; i < childCount() && !break_loop; i++)
    {
        Task* task = child(i);
        emit itemStarted(task->index);
        task->start();
        task->wait();
    }
}

/// @brief class ParallelBlock
const QString TaskCreator<ParallelBlock>::name = TaskCreator<ParallelBlock>::addTaskName("ParallelBlock");

ParallelBlock::ParallelBlock(Task* parent) :
    BlockItem(parent), 
    table(nullptr)
{
}

void ParallelBlock::createForm(QFormLayout* layout)
{
    table = new QTableView;
    table->setModel(&keyvalue);
    layout->addWidget(table);
}

void ParallelBlock::run()
{
    if (checkSkipping())
        return;
    // run subtasks
    for (int i = 0; i < childCount() && !break_loop; i++)
    {
        Task* task = child(i);
        task->start();
    }

    for (int i = 0; i < childCount() && !break_loop; i++)
    {
        Task* task = child(i);
        task->wait();
    }
}

const QString& ParallelBlock::taskname()
{
    return name;
}

QStringList ParallelBlock::getLocalVariables()
{
    return keyvalue.getVariables();
}

bool ParallelBlock::getValue(const QString& variable, QString& value)
{
    return keyvalue.getValue(variable, value);
}

void ParallelBlock::write(QJsonObject& json)
{
    BlockItem::write(json);
    json["name_value"] = keyvalue.toJson();
}

void ParallelBlock::read(const QJsonObject& json)
{
    keyvalue.fromJson(json["name_value"].toObject());
    BlockItem::read(json);
}

/// @brief class ForLoopBlock
const QString TaskCreator<ForLoopBlock>::name = TaskCreator<ForLoopBlock>::addTaskName("LoopBlock");

ForLoopBlock::ForLoopBlock(Task* parent) :
    BlockItem(parent),
    loop_counter(0),
    variable_name(properties, "variable_name", "i"),
    from(properties, "from", 0, 0, INT_MAX),
    to(properties, "to", 1, 0, INT_MAX),
    step(properties, "step", 1, 0, INT_MAX)
{
    connectPropertiesToRoot({ &variable_name, &from, &to, &step });
    connectToRoot();
}

void ForLoopBlock::createForm(QFormLayout* layout)
{
    layout->addRow(new QLabel("Variable: "), variable_name.createControl(layout->parentWidget()));
    layout->addRow(new QLabel("From ({Variable}=): "), from.createControl("", ""));
    layout->addRow(new QLabel("To ({Variable}<): "), to.createControl("", ""));
    layout->addRow(new QLabel("Step ({Variable}+=): "), step.createControl("", ""));
}

void ForLoopBlock::run()
{
    checkSkipping();
    for (loop_counter = from; loop_counter < to && !break_loop && MtlController::checkPower(); loop_counter += step)
    {
        emit changed(index);
        for (int i = 0; i < childCount() && !break_loop && MtlController::checkPower(); i++)
        {
            Task* task = child(i);
            sendStartItem(task->index);
            task->start();
            task->wait();
        }
    }
}

const QString& ForLoopBlock::taskname()
{
    return name;
}

QString ForLoopBlock::information()
{
    if (isRunning())
        return QString("%1 {%2}=%3").arg(taskname()).arg(variable_name.value).arg(loop_counter);
    else
        return QString("%1 {%2}=%3..%4,%5").arg(taskname()).arg(variable_name.value).arg(from).arg(to).arg(step);
}

QStringList ForLoopBlock::getLocalVariables()
{
    return { variable_name.value };
}

bool ForLoopBlock::getValue(const QString& variable, QString& value)
{
    bool exist_variable = false;
    if (variable == variable_name.value)
    {
        exist_variable = true;
        value = QString("%1").arg(loop_counter);
    }
    return exist_variable;
}

/// @brief class ForListBlock
const QString TaskCreator<ForListBlock>::name = TaskCreator<ForListBlock>::addTaskName("ListBlock");

ForListBlock::ForListBlock(Task* parent) :
    BlockItem(parent),
    variable_name(properties, "variable_name", "i"),
    values_list(properties, "list", QStringList()),
    loop_counter(0),
    point_sb(nullptr)
{
    connectPropertiesToRoot({ &variable_name });
    connectToRoot();
}

void ForListBlock::createForm(QFormLayout* layout)
{
    layout->addRow(new QLabel("Variable: "), variable_name.createControl(layout->parentWidget()));
    //
    QPushButton* addbtn = new QPushButton("Add Point: ");
    point_sb = new QDoubleSpinBox(layout->parentWidget());
    point_sb->setRange(-DBL_MAX, DBL_MAX);
    point_sb->setDecimals(3);
    connect(addbtn, &QPushButton::clicked, this, &ForListBlock::addPoint);
    layout->addRow(addbtn, point_sb);
    // values list
    layout->addRow(new QLabel("List: "), values_list.createControl());
}

const QString& ForListBlock::taskname()
{
    return name;
}

QString ForListBlock::information()
{
    if (isRunning() && 0 <= loop_counter && loop_counter < values_list.list_model.rowCount())
        return QString("%1 {%2}=%3").arg(taskname()).arg(variable_name.value).arg(values_list.list_model.stringList()[loop_counter]);
    else
        return QString("%1 {%2}").arg(taskname()).arg(variable_name.value);
}

QStringList ForListBlock::getLocalVariables()
{
    return { variable_name.value };
}

bool ForListBlock::getValue(const QString& variable, QString& value)
{
    bool exist_variable = false;
    if (variable == variable_name.value)
    {
        exist_variable = true;
        value = values_list.list_model.stringList()[loop_counter];
    }
    return exist_variable;
}

void ForListBlock::run()
{
    checkSkipping();
    QStringList points_list = values_list.list_model.stringList();
    for (loop_counter = 0; loop_counter < points_list.count() && !break_loop && MtlController::checkPower(); loop_counter++)
    {
        emit changed(index);
        for (int i = 0; i < childCount() && !break_loop && MtlController::checkPower(); i++)
        {
            Task* task = child(i);
            sendStartItem(task->index);
            task->start();
            task->wait();
        }
    }
}

void ForListBlock::addPoint()
{
    values_list.insertValue(point_sb->value());
}

/// @brief class InfiniteLoopBlock
const QString TaskCreator<InfiniteLoopBlock>::name = TaskCreator<InfiniteLoopBlock>::addTaskName("InfiniteLoopBlock");

InfiniteLoopBlock::InfiniteLoopBlock(Task* parent) :
    BlockItem(parent)
{
}

void InfiniteLoopBlock::createForm(QFormLayout* layout)
{
}

const QString& InfiniteLoopBlock::taskname()
{
    return name;
}

void InfiniteLoopBlock::run()
{
    checkSkipping();
    while (!break_loop && MtlController::checkPower())
        for (int i = 0; i < childCount() && !break_loop && MtlController::checkPower(); i++)
        {
            Task* task = child(i);
            sendStartItem(task->index);
            task->start();
            task->wait();
        }
}

/// @brief class QTasksModel
QTasksModel::QTasksModel(QObject* parent)
    : QAbstractItemModel(parent)
{
    rootItem = new RootBlock(this);
    connect(rootItem, &Task::changed, this, 
        [=](const QModelIndex & index) 
        { 
            emit QTasksModel::dataChanged(index, index, { Qt::DisplayRole }); 
        });
}

QTasksModel::~QTasksModel()
{
    delete rootItem;
}

RootBlock& QTasksModel::rootBlock()
{
    return *rootItem;
}

QModelIndex QTasksModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    Task* parentItem = rootItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<Task*>(parent.internalPointer());

    Task* childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex QTasksModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();

    Task* childItem = static_cast<Task*>(index.internalPointer());
    Task* parentItem = childItem->parentItem();

    if (parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int QTasksModel::rowCount(const QModelIndex& parent) const
{
    Task* parentItem;
    if (parent.column() > 0)
        return 0;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<Task*>(parent.internalPointer());

    return parentItem->childCount();
}

int QTasksModel::columnCount(const QModelIndex& parent) const
{
    return 1;
}

QVariant QTasksModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if (role == Qt::BackgroundRole)
    {
        if (index == highlighted_index)
            return QBrush(Qt::red);
        else
            return QBrush(Qt::white);
    }

    if (role == Qt::DisplayRole)
    {
        Task* item = static_cast<Task*>(index.internalPointer());
        return item->information();
    }

    return QVariant();
}

Qt::ItemFlags QTasksModel::flags(const QModelIndex& index) const
{
    if (index.isValid())
        return (QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);

    return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled;
}

Qt::DropActions QTasksModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction;
}

//
void QTasksModel::addTask(const QModelIndex& index, const QString& task_name)
{
    Task* parent_item = rootItem;
    if (index.isValid())
        parent_item = static_cast<Task*>(index.internalPointer());

    if (parent_item->type() != ItemType::kBlock)
        return /*nullptr*/;

    beginInsertRows(index, parent_item->childCount(), parent_item->childCount());
    Task* task = TaskFabric::create(task_name, parent_item);
    parent_item->appendChild(task);
    endInsertRows();
}

// remove items from index_list, for example selected items
void QTasksModel::remove(const QModelIndexList& index_list)
{
    QList< QPersistentModelIndex > persistentIndexList;
    for (QModelIndex const& index : index_list)
        persistentIndexList.append(index);
    for (QPersistentModelIndex const& persistentIndex : persistentIndexList)
        if (persistentIndex.isValid())
        {
            beginRemoveRows(persistentIndex.parent(), persistentIndex.row(), persistentIndex.row());
            Task* task = itemFromIndex(persistentIndex);
            task->eraseChilds();
            if (task->parentItem())
                task->parentItem()->eraseChild(task->row());
            endRemoveRows();
        }
}

void QTasksModel::highlightItem(const QModelIndex& index)
{
    QModelIndex last_index = highlighted_index;
    highlighted_index = index;
    dataChanged(last_index, last_index, { Qt::BackgroundRole });
    dataChanged(index, index, { Qt::BackgroundRole });
}

void QTasksModel::save(const QString& filename)
{
    QFile saveFile(filename);

    if (!saveFile.open(QIODevice::WriteOnly))
    {
        qWarning("Couldn't open save file.");
        return/* false*/;
    }

    QJsonObject rootObject;
    //
    rootItem->write(rootObject);
    //
    QJsonDocument saveDoc(rootObject);
    saveFile.write(saveDoc.toJson());
}

void QTasksModel::load(const QString& filename)
{
    QFile loadFile(filename);

    if (!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return /*false*/;
    }

    beginResetModel();
    rootItem->eraseChilds();

    QByteArray loadData = loadFile.readAll();
    QJsonDocument loadDoc(QJsonDocument::fromJson(loadData));
    rootItem->read(loadDoc.object());
    endResetModel();
}

Task* QTasksModel::itemFromIndex(const QModelIndex& index) const
{
    if ((index.row() < 0) || (index.column() < 0) || (index.model() != this))
        return nullptr;
    Task* item = static_cast<Task*>(index.internalPointer());
    return item;
}

QModelIndex QTasksModel::createIndex(int row, int column, Task* task) const
{
    if (task == nullptr)
        return QModelIndex();
    
    task->index = QAbstractItemModel::createIndex(row, column, task);
    return task->index;
}

QStringList QTasksModel::mimeTypes() const
{
    QStringList types;
    types << "application/taskitem";
    return types;
}

QMimeData* QTasksModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* mimeData = new QMimeData;
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);

    for (const QModelIndex& index : indexes) 
        if (index.isValid())
        {
            stream << uintptr_t(index.internalPointer());
        }

    mimeData->setData("application/taskitem", encodedData);
    return mimeData;
}


bool QTasksModel::canDropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) const
{
    if (row > rowCount(parent))
        row = rowCount(parent);
    if (row == -1)
        row = rowCount(parent);
    if (column == -1)
        column = 0;

    Task* parent_item = rootItem;
    if (parent.isValid())
        parent_item = static_cast<Task*>(parent.internalPointer());

    if (parent_item == nullptr)
        return true;
    if (parent_item && parent_item->type() == ItemType::kBlock && data->hasFormat("application/taskitem"))
        return true;
    else
        return false;
}

bool QTasksModel::dropMimeData(const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent)
{
    if (row > rowCount(parent))
        row = rowCount(parent);
    if (row == -1)
        row = rowCount(parent);
    if (column == -1)
        column = 0;

    Task* dst_parent = rootItem;
    if (parent.isValid())
        dst_parent = static_cast<Task*>(parent.internalPointer());

    if (dst_parent->type() != ItemType::kBlock)
        return false;

    QByteArray encoded = data->data("application/taskitem");
    QDataStream stream(&encoded, QIODevice::ReadOnly);

    // read data of droped item from MIME data
    while (!stream.atEnd())
    {
        uintptr_t ptr;
        stream >> ptr;
        Task* task = (Task*)ptr;
        Task* src_parent = task->parentItem();
        // check if source task and destination task the same
        if (src_parent != dst_parent ||
            (src_parent == dst_parent &&
                row != task->row() &&
                row != task->row() + 1))
        {
            if (action == Qt::MoveAction)
            {
                beginMoveRows(src_parent->index, task->row(), task->row(), dst_parent->index, row);
                task->move(src_parent, dst_parent, row);
                endMoveRows();
            }
        }
        if (action == Qt::CopyAction)
        {
            beginInsertRows(dst_parent->index, row, row);
            dst_parent->insertChild(row, task->duplicate());
            endInsertRows();
        }
    }

    return true;
}