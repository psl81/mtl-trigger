#include "linefilebrowse.h"
#include <QToolButton>
#include <QStyle>
#include <QFileDialog>

LineFileBrowse::LineFileBrowse(QWidget *parent, bool _dir, bool _save)
    : QLineEdit(parent), open_dir(_dir), save(_save)
{
    browseButton = new QToolButton(this);
    browseButton->setText("...");
    browseButton->setCursor(Qt::ArrowCursor);
    connect(browseButton, SIGNAL(clicked()), SLOT(browse()));
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    setStyleSheet(QString("QLineEdit { padding-right: %1px; } ").arg(browseButton->sizeHint().width() + frameWidth + 1));
    QSize msz = minimumSizeHint();
    setMinimumSize(qMax(msz.width(), browseButton->sizeHint().height() + frameWidth * 2 + 2),
                   qMax(msz.height(), browseButton->sizeHint().height() + frameWidth * 2 + 2));
}

LineFileBrowse::~LineFileBrowse()
{
    if (browseButton) delete browseButton;
}

void LineFileBrowse::resizeEvent(QResizeEvent *)
{
    QSize sz = browseButton->sizeHint();
    int frameWidth = style()->pixelMetric(QStyle::PM_DefaultFrameWidth);
    browseButton->move(rect().right() - frameWidth - sz.width(),
                      (rect().bottom() + 1 - sz.height())/2);
}

void LineFileBrowse::browse()
{
  QString filename;
  if (open_dir)
    filename = QFileDialog::getExistingDirectory();
  else
    filename = save ? QFileDialog::getSaveFileName() : QFileDialog::getOpenFileName();
  if (!filename.isEmpty())
    setText(filename);
}
