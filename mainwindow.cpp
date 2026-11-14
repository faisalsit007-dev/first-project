// mainwindow.cpp - implementation of the Student Record Management System
#include "mainwindow.h"

#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QStatusBar>
#include <QTableWidget>
#include <QHeaderView>
#include <QDoubleValidator>
#include <QtGlobal>
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
#include <QStringConverter>
#endif

// Make the text file always UTF-8 (works in Qt 5 and Qt 6)
static void useUtf8(QTextStream &s)
{
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    s.setEncoding(QStringConverter::Utf8);
#else
    s.setCodec("UTF-8");
#endif
}

// ======================================================================
//  Construction
// ======================================================================
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // The data file lives next to the executable
    filePath = QCoreApplication::applicationDirPath() + "/students.txt";

    buildUi();
    applyStyle();

    if (!loadFromFile())
        statusBar()->showMessage("Could not read students.txt");
    else
        statusBar()->showMessage(QString("Data file: %1").arg(filePath));

    refreshTable();
    setAddMode();      // application starts in Add mode
}

void MainWindow::buildUi()
{
    setWindowTitle("Student Record");
    setMinimumSize(760, 640);

    auto *central = new QWidget(this);
    central->setObjectName("central");
    setCentralWidget(central);

    // ---- Widgets ----
    auto *title = new QLabel("Student Record Management System");
    title->setObjectName("title");

    txtName     = new QLineEdit; txtRoll     = new QLineEdit;
    txtFather   = new QLineEdit; txtCaste    = new QLineEdit;
    txtDistrict = new QLineEdit; txtGpa      = new QLineEdit;
    txtSearch   = new QLineEdit;

    txtName->setPlaceholderText("e.g. Ali Khan");
    txtRoll->setPlaceholderText("e.g. K26SW001");
    txtFather->setPlaceholderText("Father's name");
    txtCaste->setPlaceholderText("Caste");
    txtDistrict->setPlaceholderText("District");
    txtGpa->setPlaceholderText("0.00 - 4.00");
    txtSearch->setPlaceholderText("Search by Roll No and press Enter");

    // GPA: only digits and a dot, 0 to 4 with up to 2 decimals
    auto *gpaCheck = new QDoubleValidator(0.0, 4.0, 2, this);
    gpaCheck->setNotation(QDoubleValidator::StandardNotation);
    gpaCheck->setLocale(QLocale::c());
    txtGpa->setValidator(gpaCheck);

    btnAdd    = new QPushButton("Add");
    btnUpdate = new QPushButton("Update");
    btnDelete = new QPushButton("Delete");
    btnNext   = new QPushButton("Next  \u25B6");
    btnPrev   = new QPushButton("\u25C0  Previous");
    btnNew    = new QPushButton("New / Clear");
    btnSearch = new QPushButton("Search");
    btnDelete->setObjectName("del");
    btnNew->setObjectName("secondary");
    btnPrev->setObjectName("secondary");
    btnNext->setObjectName("secondary");
    for (auto *b : {btnAdd, btnUpdate, btnDelete, btnNext, btnPrev, btnNew, btnSearch})
        b->setCursor(Qt::PointingHandCursor);

    lblCount = new QLabel("0 records");
    lblCount->setObjectName("counter");
    lblCount->setAlignment(Qt::AlignCenter);

    // ---- Form grid ----
    auto makeLabel = [](const QString &t) { return new QLabel(t); };
    auto *grid = new QGridLayout;
    grid->setHorizontalSpacing(20);
    grid->setVerticalSpacing(12);
    grid->addWidget(makeLabel("Student Name"),  0, 0); grid->addWidget(txtName,     0, 1);
    grid->addWidget(makeLabel("Roll No"),       1, 0); grid->addWidget(txtRoll,     1, 1);
    grid->addWidget(makeLabel("Father Name"),   2, 0); grid->addWidget(txtFather,   2, 1);
    grid->addWidget(makeLabel("Student Caste"), 3, 0); grid->addWidget(txtCaste,    3, 1);
    grid->addWidget(makeLabel("District"),      4, 0); grid->addWidget(txtDistrict, 4, 1);
    grid->addWidget(makeLabel("Student GPA"),   5, 0); grid->addWidget(txtGpa,      5, 1);
    grid->setColumnStretch(1, 1);

    // ---- Right column: navigation ----
    auto *side = new QVBoxLayout;
    side->setSpacing(10);
    side->addWidget(btnNext);
    side->addWidget(btnPrev);
    side->addWidget(btnNew);
    side->addSpacing(8);
    side->addWidget(lblCount);
    side->addStretch();

    auto *top = new QHBoxLayout;
    top->addLayout(grid, 1);
    top->addSpacing(24);
    top->addLayout(side);

    // ---- Action buttons ----
    auto *actions = new QHBoxLayout;
    actions->setSpacing(12);
    actions->addWidget(btnAdd);
    actions->addWidget(btnUpdate);
    actions->addWidget(btnDelete);
    actions->addStretch();

    // ---- Search row ----
    auto *searchRow = new QHBoxLayout;
    searchRow->addWidget(txtSearch, 1);
    searchRow->addWidget(btnSearch);

    // ---- Table of all records ----
    table = new QTableWidget(0, 6);
    table->setHorizontalHeaderLabels({"Name", "Roll No", "Father Name", "Caste", "District", "GPA"});
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setMinimumHeight(170);

    auto *root = new QVBoxLayout(central);
    root->setContentsMargins(24, 16, 24, 8);
    root->setSpacing(16);
    root->addWidget(title);
    root->addLayout(searchRow);
    root->addLayout(top);
    root->addLayout(actions);
    root->addWidget(table, 1);

    // ---- Signals and slots ----
    connect(btnAdd,    &QPushButton::clicked, this, &MainWindow::onAdd);
    connect(btnUpdate, &QPushButton::clicked, this, &MainWindow::onUpdate);
    connect(btnDelete, &QPushButton::clicked, this, &MainWindow::onDelete);
    connect(btnNext,   &QPushButton::clicked, this, &MainWindow::onNext);
    connect(btnPrev,   &QPushButton::clicked, this, &MainWindow::onPrevious);
    connect(btnNew,    &QPushButton::clicked, this, &MainWindow::onNew);
    connect(btnSearch, &QPushButton::clicked, this, &MainWindow::onSearch);
    connect(txtSearch, &QLineEdit::returnPressed, this, &MainWindow::onSearch);
    connect(table, &QTableWidget::cellClicked, this, &MainWindow::onTableClicked);
}

void MainWindow::applyStyle()
{
    // Every colour is set explicitly (text AND background) so nothing can turn white-on-white.
    setStyleSheet(R"(
        QMainWindow, QWidget#central { background: #eef1f6; }
        QLabel { color: #1f2a44; font-size: 14px; font-weight: 600; background: transparent; }
        QLabel#title { font-size: 22px; font-weight: 700; color: #1f3a7a; padding-bottom: 4px; }
        QLabel#counter { color: #3b4a6b; font-weight: 600; background: #dfe6f3;
                         border-radius: 6px; padding: 6px; }

        QLineEdit { background: #ffffff; color: #1f2a44; border: 1px solid #b8c2d6;
                    border-radius: 6px; padding: 7px; font-size: 14px;
                    selection-background-color: #3b6fd4; selection-color: #ffffff; }
        QLineEdit:focus { border: 2px solid #3b6fd4; }
        QLineEdit:read-only { background: #e1e6f0; color: #3b4a6b; }

        QPushButton { background: #3b6fd4; color: #ffffff; border: none; border-radius: 6px;
                      padding: 9px 20px; font-size: 14px; font-weight: 600; min-width: 80px; }
        QPushButton:hover { background: #2f5cb5; }
        QPushButton:pressed { background: #264b95; }
        QPushButton:disabled { background: #c3cbdc; color: #f4f6fb; }
        QPushButton#del { background: #d9534f; }
        QPushButton#del:hover { background: #c04541; }
        QPushButton#del:disabled { background: #e5c3c1; color: #f9f2f2; }
        QPushButton#secondary { background: #5a6b8f; }
        QPushButton#secondary:hover { background: #48587a; }
        QPushButton#secondary:disabled { background: #c3cbdc; color: #f4f6fb; }

        QTableWidget { background: #ffffff; alternate-background-color: #f4f6fb; color: #1f2a44;
                       gridline-color: #d5dbe8; border: 1px solid #b8c2d6; border-radius: 6px;
                       font-size: 13px; }
        QTableWidget::item:selected { background: #3b6fd4; color: #ffffff; }
        QHeaderView::section { background: #dfe6f3; color: #1f2a44; font-weight: 700;
                               border: none; border-right: 1px solid #c4cde0; padding: 6px; }
        QStatusBar { background: #dfe6f3; color: #3b4a6b; }

        QMessageBox { background: #f4f6fb; }
        QMessageBox QLabel { color: #1f2a44; font-weight: 500; }
    )");
}

// ======================================================================
//  File handling  (format: name,roll,father,caste,district,gpa)
// ======================================================================
bool MainWindow::loadFromFile()
{
    students.clear();

    QFile file(filePath);
    if (!file.exists())
        return true;                         // first run: no file yet is fine
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QTextStream in(&file);
    useUtf8(in);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty())
            continue;
        const QStringList f = line.split(',');
        if (f.size() != 6)
            continue;                        // skip corrupted lines
        Student s;
        s.name = f[0].trimmed(); s.roll = f[1].trimmed(); s.father = f[2].trimmed();
        s.caste = f[3].trimmed(); s.district = f[4].trimmed();
        s.gpa = f[5].trimmed().toDouble();
        students.push_back(s);
    }
    return true;
}

bool MainWindow::saveToFile()
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        QMessageBox::critical(this, "Error",
            "Cannot write to:\n" + filePath + "\n\nCheck that the folder is writable.");
        return false;
    }
    QTextStream out(&file);
    useUtf8(out);
    for (const Student &s : students) {
        out << s.name << ',' << s.roll << ',' << s.father << ','
            << s.caste << ',' << s.district << ','
            << QString::number(s.gpa, 'f', 2) << '\n';
    }
    out.flush();
    file.close();
    return true;
}

// ======================================================================
//  Helpers
// ======================================================================
bool MainWindow::readForm(Student &out, QString &error) const
{
    out.name     = txtName->text().trimmed();
    out.roll     = txtRoll->text().trimmed();
    out.father   = txtFather->text().trimmed();
    out.caste    = txtCaste->text().trimmed();
    out.district = txtDistrict->text().trimmed();
    const QString gpaText = txtGpa->text().trimmed();

    if (out.name.isEmpty() || out.roll.isEmpty() || out.father.isEmpty() ||
        out.caste.isEmpty() || out.district.isEmpty() || gpaText.isEmpty()) {
        error = "All fields are required.";
        return false;
    }
    // A comma would break the comma-separated file format
    const QStringList texts = {out.name, out.roll, out.father, out.caste, out.district};
    for (const QString &s : texts) {
        if (s.contains(',')) {
            error = "Commas are not allowed in the fields.";
            return false;
        }
    }
    bool ok = false;
    out.gpa = gpaText.toDouble(&ok);
    if (!ok || out.gpa < 0.0 || out.gpa > 4.0) {
        error = "GPA must be a number between 0 and 4.";
        return false;
    }
    return true;
}

int MainWindow::findByRoll(const QString &roll) const
{
    for (size_t i = 0; i < students.size(); ++i)
        if (students[i].roll.compare(roll, Qt::CaseInsensitive) == 0)
            return static_cast<int>(i);
    return -1;
}

void MainWindow::showRecord(int index)
{
    if (index < 0 || index >= static_cast<int>(students.size()))
        return;
    currentIndex = index;
    const Student &s = students[index];
    txtName->setText(s.name);
    txtRoll->setText(s.roll);
    txtFather->setText(s.father);
    txtCaste->setText(s.caste);
    txtDistrict->setText(s.district);
    txtGpa->setText(QString::number(s.gpa, 'f', 2));
    table->selectRow(index);
    setBrowseMode();
}

void MainWindow::clearForm()
{
    txtName->clear(); txtRoll->clear(); txtFather->clear();
    txtCaste->clear(); txtDistrict->clear(); txtGpa->clear();
}

void MainWindow::setAddMode()
{
    currentIndex = -1;
    btnAdd->setEnabled(true);
    btnUpdate->setEnabled(false);
    btnDelete->setEnabled(false);
    txtRoll->setReadOnly(false);
    table->clearSelection();
    updateCounter();
    txtName->setFocus();
}

void MainWindow::setBrowseMode()
{
    btnAdd->setEnabled(false);
    btnUpdate->setEnabled(true);
    btnDelete->setEnabled(true);
    txtRoll->setReadOnly(true);   // Roll No is the unique key, so it can't be edited here
    updateCounter();
}

void MainWindow::updateCounter()
{
    const int total = static_cast<int>(students.size());
    if (currentIndex >= 0)
        lblCount->setText(QString("Record %1 of %2").arg(currentIndex + 1).arg(total));
    else
        lblCount->setText(QString("%1 record%2").arg(total).arg(total == 1 ? "" : "s"));
}

void MainWindow::refreshTable()
{
    table->setRowCount(static_cast<int>(students.size()));
    for (int r = 0; r < static_cast<int>(students.size()); ++r) {
        const Student &s = students[r];
        const QStringList cols = {s.name, s.roll, s.father, s.caste, s.district,
                                  QString::number(s.gpa, 'f', 2)};
        for (int c = 0; c < cols.size(); ++c)
            table->setItem(r, c, new QTableWidgetItem(cols[c]));
    }
}

// ======================================================================
//  Button slots
// ======================================================================
void MainWindow::onAdd()
{
    Student s;
    QString err;
    if (!readForm(s, err)) {
        QMessageBox::warning(this, "Invalid input", err);
        return;
    }
    if (findByRoll(s.roll) != -1) {
        QMessageBox::warning(this, "Duplicate", "A student with this Roll No already exists.");
        return;
    }
    students.push_back(s);
    if (!saveToFile()) {
        students.pop_back();
        return;
    }
    refreshTable();
    clearForm();
    setAddMode();
    QMessageBox::information(this, "Success", "Record added successfully.");
}

void MainWindow::onNext()
{
    if (students.empty()) {
        QMessageBox::information(this, "No records", "There are no records yet. Add one first.");
        return;
    }
    int next = currentIndex + 1;
    if (next >= static_cast<int>(students.size()))
        next = 0;                              // wrap around to the first record
    showRecord(next);
}

void MainWindow::onPrevious()
{
    if (students.empty()) {
        QMessageBox::information(this, "No records", "There are no records yet. Add one first.");
        return;
    }
    int prev = currentIndex - 1;
    if (prev < 0)
        prev = static_cast<int>(students.size()) - 1;   // wrap to the last record
    showRecord(prev);
}

void MainWindow::onUpdate()
{
    if (currentIndex < 0)
        return;
    Student s;
    QString err;
    if (!readForm(s, err)) {
        QMessageBox::warning(this, "Invalid input", err);
        return;
    }
    // Roll No is the identifier: locate the record by it
    const int idx = findByRoll(s.roll);
    if (idx == -1) {
        QMessageBox::warning(this, "Not found", "No record with this Roll No.");
        return;
    }
    students[idx] = s;
    if (saveToFile()) {
        refreshTable();
        showRecord(idx);
        QMessageBox::information(this, "Success", "Record updated successfully.");
    }
}

void MainWindow::onDelete()
{
    if (currentIndex < 0)
        return;
    const auto answer = QMessageBox::question(this, "Confirm delete",
                            "Delete this student record?",
                            QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (answer != QMessageBox::Yes)
        return;

    const int removed = currentIndex;
    const Student backup = students[removed];
    students.erase(students.begin() + removed);
    if (!saveToFile()) {                       // saving failed: undo the delete
        students.insert(students.begin() + removed, backup);
        return;
    }
    refreshTable();

    if (students.empty()) {
        clearForm();
        setAddMode();
    } else {
        showRecord(removed >= static_cast<int>(students.size()) ? 0 : removed);
    }
    QMessageBox::information(this, "Success", "Record deleted.");
}

void MainWindow::onNew()
{
    clearForm();
    setAddMode();
}

void MainWindow::onSearch()
{
    const QString roll = txtSearch->text().trimmed();
    if (roll.isEmpty())
        return;
    const int idx = findByRoll(roll);
    if (idx == -1) {
        QMessageBox::information(this, "Search", "No student found with that Roll No.");
        return;
    }
    showRecord(idx);
}

void MainWindow::onTableClicked(int row, int /*column*/)
{
    showRecord(row);
}
