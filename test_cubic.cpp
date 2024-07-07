#include "interp_curves.hpp"

#include <iostream>

struct KF
{
    std::vector<double> curves;
    double slopeFactor = 1;
    size_t subdivAfter = 20;
};

struct KFcurves
{
    auto operator()(const KF& kf)
        -> std::span<const double>
    { return kf.curves; }

};

struct KFsubdiv
{
    auto operator()(const KF& kf)
        -> size_t
    { return kf.subdivAfter; }

};

struct KFslopeF
{
    auto operator()(const KF& kf)
        -> double
    { return kf.slopeFactor; }

};

void run()
{

    // auto keyframes = std::vector<KF> {
    //     { .curves = {0, 100} },
    //     { .curves = {0, 100, 0},
    //       .slopeFactor = 0 },
    //     { .curves = {0, 100, 100, 0} }
    // };

    auto keyframes = std::vector<KF> {
        { .curves = {0, 100} },
        { .curves = {30} },
        { .curves = {70} },
        { .curves = {0, 100} }
    };

    auto subdiv = 20;
    auto curves =
        interpCurves<KF>(keyframes, KFcurves{}, KFsubdiv{}, KFslopeF{});
    for (size_t row=0, nrows=curves.front().size(); row<nrows; ++row)
    {
        auto xi = static_cast<double>(row) / subdiv;
        std::cout << xi;
        for (const auto& curve : curves)
            std::cout << '\t' << curve[row];
        std::cout << std::endl;
    }
}

// TODO: Make it a google test
auto testMapIndex()
    -> void
{
    for (size_t nglobal=1; nglobal<=20; ++nglobal)
    {
        for (size_t nlocal=1; nlocal<=nglobal; ++nlocal)
        {
            std::cout << nlocal << " / " << nglobal << ":";
            std::vector<size_t> ilocal;
            for (size_t iglobal=0; iglobal<nglobal; ++iglobal)
                ilocal.push_back(localIndex(iglobal, nglobal, nlocal));

            size_t ndelta = 0;
            for (size_t iglobal=1; iglobal<nglobal; ++iglobal)
                if (ilocal[iglobal] - ilocal[iglobal-1] > 1)
                    ++ndelta;

            size_t nasym = 0;
            for (size_t iglobal=1; 2*iglobal+1<nglobal; ++iglobal)
            {
                auto iglobal2 = nglobal - 1 - iglobal;
                auto d1 = ilocal[iglobal] - ilocal[iglobal-1];
                auto d2 = ilocal[iglobal2+1] - ilocal[iglobal2];
                if (d1 != d2)
                    ++nasym;
            }

            size_t nrange = 0;
            if (ilocal.front() != 0)
                ++nrange;
            if (ilocal.back() + 1 != nlocal)
                ++nrange;

            for (size_t iglobal=0; iglobal<nglobal; ++iglobal)
                std::cout << '\t' << ilocal[iglobal];
            if (ndelta > 0)
                std::cout << "\tDELTA: " << ndelta;
            if (nasym > 0)
                std::cout << "\tASYM: " << nasym;
            if (nrange > 0)
                std::cout << "\tRANGE: " << nrange;
            std::cout << std::endl;
        }
        std::cout << "----\n";
    }
}

int main()
{
    try
    {
        run();
        // testMapIndex();
        return EXIT_SUCCESS;
    }
    catch (std::exception& e)
    {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
