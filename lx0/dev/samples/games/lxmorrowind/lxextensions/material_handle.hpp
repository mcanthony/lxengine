
#pragma once

class material_handle
{
public:
    std::string     handle;
    std::string     format;
    std::function< std::shared_ptr<std::istream>() > callback;
};
