#ifndef LINEFILEBROWSE_H
#define LINEFILEBROWSE_H

#include <QLineEdit>

class QToolButton;

class LineFileBrowse : public QLineEdit
{
    Q_OBJECT

public:
    LineFileBrowse(QWidget *parent = 0, bool _dir = false, bool _save = false);
    ~LineFileBrowse();

protected:
    void resizeEvent(QResizeEvent *);

private slots:
    void browse();

private:
    QToolButton *browseButton;
    bool open_dir;
    bool save;
};

#endif // LINEFILEBROWSE_H
