
#pragma once

class material_handle
{
public:
    material_handle() {}
    material_handle(const material_handle& that)
        : handle (that.handle)
        , format (that.format)
        , callback ( that.callback)
        , graph (that.graph)
    {
    }
    std::string     handle;
    std::string     format;
    std::function< std::shared_ptr<std::istream>() > callback;

    lx0::lxvar      graph;
};
