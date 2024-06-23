#include "document.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>

#include <fstream>
#include <sstream>

Document::Document(QObject *parent)
    : QObject{ parent }
{}

auto Document::newDocument()
    -> void
{
    auto fg = FractalGeneratorObject{}.fractalGenerator();
    fractalGenerator_.setFractalGenerator(fg);
    fileName_.clear();
}

auto Document::fractalGenerator()
    -> FractalGeneratorObject*
{ return &fractalGenerator_; }

auto Document::fileName() const
    -> QString
{ return fileName_; }

auto Document::open(const QString& fileName) -> void
{
    auto critical = [&](auto... args) -> void
    {
        auto s = std::ostringstream{};
        ((s << args), ...);

        QMessageBox::critical(
            qobject_cast<QWidget*>(parent()),
            {},
            QString::fromUtf8(s.str()));
    };

    auto s = std::ifstream{fileName.toStdString()};
    if (!s.is_open())
        return critical("Failed to open input file ", fileName.toStdString());

    size_t lineNumber = 0;

    auto parseFailure =
        [&](const std::string& line) -> void
    {
        critical(
            "Failed to parse input file\n",
            fileName.toStdString(), ":", lineNumber, ": ", line);
    };

    FractalGenerator fg;
    static const auto cellSep = QRegularExpression{"[ \t,]+"};
    while (true)
    {
        ++lineNumber;
        auto line = std::string{};
        std::getline(s, line);
        if (line.empty() || s.fail())
            break;

        auto cells = QString::fromStdString(line).split(cellSep);
        if (cells.size() != 2)
            return parseFailure(line);
        auto ok_x = false;
        auto x = cells[0].toDouble(&ok_x);
        auto ok_y = false;
        auto y = cells[1].toDouble(&ok_y);
        if (!(ok_x && ok_y))
        {
            if (lineNumber == 1)
                continue;
            return parseFailure(line);
        }
        fg.emplace_back(x, y);
    }
    if (fg.size() < 2)
        return critical(
            "Too few points in generator read from file ",
            fileName.toStdString());
    fractalGenerator_.setFractalGenerator(fg);
    fileName_ = fileName;
}

auto Document::saveAs(const QString& fileName) -> void
{
    auto s = std::ofstream{fileName.toStdString()};
    if (!s.is_open())
    {
        QMessageBox::critical(
            qobject_cast<QWidget*>(parent()),
            {},
            "Failed to open output file");
        return;
    }
    s << "x\ty\n";
    for (auto& v : fractalGenerator_.fractalGenerator())
        s << v[0] << '\t' << v[1] << std::endl;
    fileName_ = fileName;
}

auto Document::open() -> void
{
    auto fileName =
        QFileDialog::getOpenFileName(
            qobject_cast<QWidget*>(parent()),
            tr("Open fractal generator file"),
            QString(),
            tr("Text files (*.txt);;All files (*)"));
    if (fileName.isEmpty())
        return;

    open(fileName);
}

auto Document::save() -> void
{
    if (fileName_.isEmpty())
        saveAs();
    else
        saveAs(fileName_);
}

auto Document::saveAs() -> void
{
    auto fileName =
        QFileDialog::getSaveFileName(
            qobject_cast<QWidget*>(parent()),
            tr("Save fractal generator file"),
            QString(),
            tr("Text files (*.txt);;All files (*)"));
    if (fileName.isEmpty())
        return;

    saveAs(fileName);
}
