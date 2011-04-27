/**********************************************************************
  Copyright (C) 2010 Olaf J. Schumann

  This file is part of the Cologne Laue Indexation Program.
  For more information, see <http://clip.berlios.de>

  Clip is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 3 of the License, or
  (at your option) any later version.

  Clip is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see http://www.gnu.org/licenses/
  or write to the Free Software Foundation, Inc., 51 Franklin Street,
  Fifth Floor, Boston, MA 02110-1301, USA.
 **********************************************************************/

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
