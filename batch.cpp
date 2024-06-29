#include "batch.hpp"

#include "render_fractal.hpp"
#include "throw.hpp"

#include <QImage>
#include <QPainter>

#include <iomanip>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>


using namespace std::string_view_literals;

namespace {

auto renderFractalImage(std::span<const Vec2d> base,
                        std::span<const Vec2d> gen,
                        const FractalViewParam& param,
                        const QSize& size)
    -> QImage
{
    auto img = QImage{ size, QImage::Format_ARGB32 };
    auto p = QPainter{ &img };
    renderFractal(p, {QPoint{}, size}, base, gen, param);
    return img;
}

} // anonymous namespace



auto batch(const QString& batchFileName)
    -> int
{
    namespace fs = std::filesystem;

    try
    {
        const auto outputDirName = "gen_fractal.out";
        if (fs::exists(outputDirName))
            throw_("Output directory '", outputDirName, "' already exists");

        if (!fs::create_directory(outputDirName))
            throw_("Failed to create output directory '", outputDirName, "'");

        std::cout << "Generating images in directory '" << outputDirName << "'"
                  << std::endl;

        auto outputFileName = [&](size_t number)
            -> std::string
        {
            auto s = std::ostringstream{};
            s << "img_"
              << std::setw(6) << std::setfill('0') << number
              << ".png";
            return fs::path(outputDirName) / s.str();
        };

        auto in = std::ifstream(batchFileName.toStdString());
        if (!in.is_open())
            throw_("Failed to open input file '",
                   batchFileName.toStdString(), "'");

        size_t lineNumber = 0;
        while (true)
        {
            ++lineNumber;

            std::string line;
            std::getline(in, line);
            if (in.fail())
                break;
            if (line.empty())
                continue;

            if (lineNumber == 1)
                continue;   // Skip header

            auto base = std::vector<Vec2d>{};
            auto gen = std::vector<Vec2d>{};
            auto param = FractalViewParam{};
            auto size = QSize{};

            size_t pos = 0;
            auto nextToken = [&]()-> std::optional<std::string>
            {
                if (pos == std::string::npos || pos == line.size())
                    return std::nullopt;

                auto pos2 = line.find_first_of(',', pos);
                auto n = (pos2 == std::string::npos ? line.size() : pos2) - pos;
                auto token = line.substr(pos, n);
                pos = pos2 == std::string::npos? std::string::npos: pos2 + 1;
                return token;
            };

            auto nextValue =
                [&]<typename T>(T& value)
                -> bool
            {
                auto tok = nextToken();
                if (!tok || *tok == "*"sv)
                    return false;
                std::istringstream s{ *tok };
                s >> value;
                if (s.fail())
                    throw_("Failed to parse value '", *tok,
                           "', line ", lineNumber);
                return true;
            };

            auto ensureNextValue =
                [&]<typename T>(T& value)
            {
                if (!nextValue(value))
                    throw_("Too few values in line ", lineNumber);
            };

            auto read2dLine = [&](std::vector<Vec2d>& v)
            {
                while (true)
                {
                    double x;
                    double y;
                    if (!nextValue(x))
                        break;
                    if (!nextValue(y))
                        throw_("Odd number of 2d line coordinates, line ",
                               lineNumber);
                    v.emplace_back(x, y);
                }
            };

            read2dLine(base);
            read2dLine(gen);

            auto paramTuple = fields_of(param);
            constexpr auto paramCount = std::tuple_size_v<decltype(paramTuple)>;
            [&]<size_t... I>(std::index_sequence<I...>)
            {
                (ensureNextValue(std::get<I>(paramTuple)), ...);
            }(std::make_index_sequence<paramCount>());

            ensureNextValue(size.rwidth());
            ensureNextValue(size.rheight());

            if (nextToken())
                std::cout << "NOTE: Ignoring extra elements in line "
                          << lineNumber << std::endl;

            auto img = renderFractalImage(base, gen, param, size);
            img.save(QString::fromStdString(outputFileName(lineNumber-1)));
        }
        return EXIT_SUCCESS;
    }

    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
