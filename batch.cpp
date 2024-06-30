#include "batch.hpp"

#include "anim_param.hpp"
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

struct BatchLine
{
    std::vector<Vec2d> base;
    std::vector<Vec2d> gen;
    FractalViewParam viewParam;
    QSize size;
    AnimParam animParam;
};

inline auto field_names_of(TypeTag<QSize>)
    -> std::array<std::string_view, 2>
{ return { "width", "height" }; }

inline auto fields_of(QSize& p)
    -> std::tuple<int&, int&>
{ return std::tie(p.rwidth(), p.rheight()); }

inline auto fields_of(const QSize& p)
    -> std::tuple<int, int>
{ return {p.width(), p.height()}; }

auto lerp(double x0, double x1, double p)
    -> double
{ return x0*(1-p) + x1*p; }

auto lerp(size_t x0, size_t x1, double p)
    -> size_t
{ return static_cast<size_t>(x0*(1-p) + x1*p); }

auto lerp(int x0, int x1, double p)
    -> int
{ return static_cast<int>(x0*(1-p) + x1*p); }

auto lerp(bool x0, bool x1, double p)
    -> bool
{ return lerp(x0? 1.: 0., x1? 1.:0, p) >= 0.5; }

auto lerp(const Vec2d& x0, const Vec2d& x1, double p)
    -> Vec2d
{ return { lerp(x0[0], x1[0], p), lerp(x0[1], x1[1], p) }; }

auto lerp2dLine(const std::vector<Vec2d>& l0,
                const std::vector<Vec2d>& l1,
                const AnimParam& a0,
                const AnimParam& a1,
                double p)
    -> std::vector<Vec2d>
{
    size_t n0 = l0.size();
    size_t n1 = l1.size();
    auto n = std::max(n0, n1);
    auto result = std::vector<Vec2d>{};
    result.reserve(n);
    constexpr auto prep = +[](const Vec2d& v, const AnimParam& a)
        -> Vec2d
    {
        auto result = v;
        if (a.reflectX)
            result[0] = -result[0];
        if (a.reflectY)
            result[1] = -result[1];
        return result;
    };
    for (size_t i=0; i<n; ++i)
    {
        auto i0 = i * n0 / n;
        auto i1 = i * n1 / n;
        auto v0 = prep(l0[i0], a0);
        auto v1 = prep(l1[i1], a1);
        result.push_back(lerp(v0, v1, p));
    }
    return result;
}

template <typename T>
auto lerpStruct(const T& x0, const T& x1, double p)
    -> T
{
    auto result = T{};
    auto f0 = fields_of(x0);
    auto f1 = fields_of(x1);
    auto f = fields_of(result);
    constexpr auto fieldCount = std::tuple_size_v<decltype(f)>;
    [&]<size_t... I>(std::index_sequence<I...>)
    {
        ((std::get<I>(f) = lerp(std::get<I>(f0), std::get<I>(f1), p)), ...);
    }(std::make_index_sequence<fieldCount>());
    return result;
}


auto interpolateBatchLine(const BatchLine& bl0,
                          const BatchLine& bl1,
                          double param)
    -> BatchLine
{
    auto p = param*param*(3 - 2*param);

    return {
        .base = lerp2dLine(
            bl0.base, bl1.base,
            {}, {},
            p),
        .gen = lerp2dLine(
            bl0.gen, bl1.gen,
            bl0.animParam, bl1.animParam,
            p),
        .viewParam = lerpStruct(bl0.viewParam, bl1.viewParam, p),
        .size = lerpStruct(bl0.size, bl1.size, p),
        .animParam = {}
    };
}

} // anonymous namespace

auto batch(const QString& batchFileName)
    -> int
{
    namespace fs = std::filesystem;

    try
    {
        const auto outputDirName = "gen_fractal.out";

        auto outputFileName = [&](size_t number)
            -> QString
        {
            auto s = std::ostringstream{};
            s << "img_"
              << std::setw(6) << std::setfill('0') << number
              << ".png";
            return QString::fromStdString(fs::path(outputDirName) / s.str());
        };

        auto in = std::ifstream(batchFileName.toStdString());
        if (!in.is_open())
            throw_("Failed to open input file '",
                   batchFileName.toStdString(), "'");

        size_t lineNumber = 0;
        auto batchLines = std::vector<BatchLine>{};
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

            auto read2dLine = [&]() -> std::vector<Vec2d>
            {
                auto result = std::vector<Vec2d>{};
                while (true)
                {
                    double x;
                    double y;
                    if (!nextValue(x))
                        break;
                    if (!nextValue(y))
                        throw_("Odd number of 2d line coordinates, line ",
                               lineNumber);
                    result.emplace_back(x, y);
                }
                return result;
            };

            auto readStruct = [&]<typename T>(TypeTag<T> tag) -> T
            {
                T result;

                auto fields = fields_of(result);
                constexpr auto fieldCount = std::tuple_size_v<decltype(fields)>;
                [&]<size_t... I>(std::index_sequence<I...>)
                {
                    (ensureNextValue(std::get<I>(fields)), ...);
                }(std::make_index_sequence<fieldCount>());

                return result;
            };

            batchLines.push_back({
                .base         = read2dLine(),
                .gen          = read2dLine(),
                .viewParam    = readStruct(Type<FractalViewParam>),
                .size         = readStruct(Type<QSize>),
                .animParam    = readStruct(Type<AnimParam>)});

            if (nextToken())
                std::cout << "NOTE: Ignoring extra elements in line "
                          << lineNumber << std::endl;
        }

        if (fs::exists(outputDirName))
            throw_("Output directory '", outputDirName, "' already exists");

        if (!fs::create_directory(outputDirName))
            throw_("Failed to create output directory '", outputDirName, "'");

        std::cout << "Generating images in directory '" << outputDirName << "'"
                  << std::endl;

        size_t frameCount = 0;
        for (size_t iLine=0, nLines=batchLines.size(); iLine<nLines; ++iLine)
        {
            const auto& bl = batchLines[iLine];
            if (iLine > 0)
            {
                auto blPrev = batchLines[iLine-1];
                auto midFrames = blPrev.animParam.frameCountAfter;
                for (size_t iFrame=0; iFrame<midFrames; ++iFrame)
                {
                    auto param = static_cast<double>(iFrame+1) / (midFrames+1);
                    auto blInter = interpolateBatchLine(blPrev, bl, param);
                    renderFractalImage(blInter.base,
                                       blInter.gen,
                                       blInter.viewParam,
                                       blInter.size)
                        .save(outputFileName(++frameCount));
                }
            }
            renderFractalImage(bl.base, bl.gen, bl.viewParam, bl.size)
                .save(outputFileName(++frameCount));
        }

        return EXIT_SUCCESS;
    }

    catch (const std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
