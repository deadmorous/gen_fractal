#pragma once

class ScopedTrue
{
public:
    ScopedTrue(bool& b) : b_{b}
    { b = true; }

    ~ScopedTrue() { b_ = false; }
private:
    bool& b_;
};
