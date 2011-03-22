#include "printdialog.h"
#include "ui_printdialog.h"

PrintDialog::PrintDialog(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::PrintDialog)
{
  ui->setupUi(this);
}

PrintDialog::~PrintDialog()
{
  delete ui;
}

#include <QTextTable>
#include <QTextTableFormat>
#include <QTextFrameFormat>

void PrintDialog::on_actionInsert_Cell_Table_triggered()
{
  QTextCursor cursor(ui->textEdit->textCursor());
  cursor.beginEditBlock();
  QTextTableFormat format;
  format.setPadding(0.0);
  format.setCellPadding(2.0);
  format.setCellSpacing(0.0);
  format.setBorder(1.0);
  format.setBorderStyle(QTextFrameFormat::BorderStyle_Solid);
  format.setAlignment(Qt::AlignCenter);
  QTextTable* table = cursor.insertTable(2,3, format);
  table->cellAt(0, 0).firstCursorPosition().insertText("a");
  table->cellAt(0, 1).firstCursorPosition().insertText("b");
  table->cellAt(0, 2).firstCursorPosition().insertText("c");
  table->cellAt(1, 0).firstCursorPosition().insertText("alpha");
  table->cellAt(1, 1).firstCursorPosition().insertText("beta");
  table->cellAt(1, 2).firstCursorPosition().insertText("gamma");
  cursor.insertHtml("<br>");
  cursor.endEditBlock();
}
