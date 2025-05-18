#include "argh.h"

namespace argh
{
    // Construct with a value.
    stringstream_proxy::stringstream_proxy(std::string const& value) :
        stream_(value)
    {}

    // Copy constructor.
    stringstream_proxy::stringstream_proxy(const stringstream_proxy& other) :
        stream_(other.stream_.str())
    {
        stream_.setstate(other.stream_.rdstate());
    }

    stringstream_proxy& stringstream_proxy::operator=(const stringstream_proxy& other) {
        if (this != &other) {
            stream_.str(other.stream_.str());
            stream_.clear(other.stream_.rdstate());
        }
        return *this;
    }

    void stringstream_proxy::setstate(std::ios_base::iostate state) { stream_.setstate(state); }

    // Get the string value.
    std::string stringstream_proxy::str() const { return stream_.str(); }

    // Check the state of the stream.
    // False when the most recent stream operation failed
    stringstream_proxy::operator bool() const { return !!stream_; }

    stringstream_proxy& stringstream_proxy::operator>>(bool& value) { stream_ >> value; return *this;}
    stringstream_proxy& stringstream_proxy::operator>>(double& value) { stream_ >> value; return *this;}
    stringstream_proxy& stringstream_proxy::operator>>(char*& value) { stream_ >> value; return *this;}
    stringstream_proxy& stringstream_proxy::operator>>(std::string& value) { stream_ >> value; return *this;}
    stringstream_proxy& stringstream_proxy::operator>>(float& value) { stream_ >> value; return *this;}
    stringstream_proxy& stringstream_proxy::operator>>(char& value) { stream_ >> value; return *this;}
    stringstream_proxy& stringstream_proxy::operator>>(short& value) { stream_ >> value; return *this;}
    stringstream_proxy& stringstream_proxy::operator>>(int& value) { stream_ >> value; return *this;}
    stringstream_proxy& stringstream_proxy::operator>>(long& value) { stream_ >> value; return *this;}
    stringstream_proxy& stringstream_proxy::operator>>(long long& value) { stream_ >> value; return *this;}
    stringstream_proxy& stringstream_proxy::operator>>(unsigned char& value) { stream_ >> value; return *this;}
    stringstream_proxy& stringstream_proxy::operator>>(unsigned short& value) { stream_ >> value; return *this;}
    stringstream_proxy& stringstream_proxy::operator>>(unsigned int& value) { stream_ >> value; return *this;}
    stringstream_proxy& stringstream_proxy::operator>>(unsigned long& value) { stream_ >> value; return *this;}
    stringstream_proxy& stringstream_proxy::operator>>(unsigned long long& value) { stream_ >> value; return *this;}


//////////////////////////////////////////////////////////////////////////

void parser::parse(const char * const argv[], int mode)
{
    int argc = 0;
    for (auto argvp = argv; *argvp; ++argc, ++argvp);
    parse(argc, argv, mode);
}

//////////////////////////////////////////////////////////////////////////

void parser::parse(int argc, const char* const argv[], int mode /*= PREFER_FLAG_FOR_UNREG_OPTION*/)
{
    // clear out possible previous parsing remnants
    flags_.clear();
    params_.clear();
    pos_args_.clear();

    if(argv != nullptr && argc > 1 && argv[argc - 1] == nullptr)
        argc--;

    // convert to strings
    args_.resize(static_cast<decltype(args_)::size_type>(argc));
    std::transform(argv, argv + argc, args_.begin(), [](const char* const arg) { return arg;  });

    // parse line
    for (auto i = 0u; i < args_.size(); ++i)
    {
        if (!is_option(args_[i]))
        {
            pos_args_.emplace_back(args_[i]);
            continue;
        }

        auto name = trim_leading_dashes(args_[i]);

        if (!(mode & NO_SPLIT_ON_EQUALSIGN))
        {
            auto equalPos = name.find('=');
            if (equalPos != std::string::npos)
            {
                params_.insert({ name.substr(0, equalPos), name.substr(equalPos + 1) });
                continue;
            }
        }

        // if the option is unregistered and should be a multi-flag
        if (1 == (args_[i].size() - name.size()) &&         // single dash
            argh::SINGLE_DASH_IS_MULTIFLAG & mode && // multi-flag mode
            !is_param(name))                                  // unregistered
        {
            std::string keep_param;

            if (!name.empty() && is_param(std::string(1ul, name.back()))) // last char is param
            {
                keep_param += name.back();
                name.resize(name.size() - 1);
            }

            for (auto const& c : name)
            {
                flags_.emplace(std::string{ c });
            }

            if (!keep_param.empty())
            {
                name = keep_param;
            }
            else
            {
                continue; // do not consider other options for this arg
            }
        }

        // any potential option will get as its value the next arg, unless that arg is an option too
        // in that case it will be determined a flag.
        if (i == args_.size() - 1 || is_option(args_[i + 1]))
        {
            flags_.emplace(name);
            continue;
        }

        // if 'name' is a pre-registered option, then the next arg cannot be a free parameter to it is skipped
        // otherwise we have 2 modes:
        // PREFER_FLAG_FOR_UNREG_OPTION: a non-registered 'name' is determined a flag.
        //                               The following value (the next arg) will be a free parameter.
        //
        // PREFER_PARAM_FOR_UNREG_OPTION: a non-registered 'name' is determined a parameter, the next arg
        //                                will be the value of that option.

        assert(!(mode & argh::PREFER_FLAG_FOR_UNREG_OPTION)
               || !(mode & argh::PREFER_PARAM_FOR_UNREG_OPTION));

        bool preferParam = mode & argh::PREFER_PARAM_FOR_UNREG_OPTION;

        if (is_param(name) || preferParam)
        {
            params_.insert({ name, args_[i + 1] });
            ++i; // skip next value, it is not a free parameter
            continue;
        }
        else
        {
            flags_.emplace(name);
        }
    }
}

//////////////////////////////////////////////////////////////////////////

string_stream parser::bad_stream() const
{
    string_stream bad;
    bad.setstate(std::ios_base::failbit);
    return bad;
}

//////////////////////////////////////////////////////////////////////////

bool parser::is_number(std::string const& arg) const
{
    // inefficient but simple way to determine if a string is a number (which can start with a '-')
    std::istringstream istr(arg);
    double number;
    istr >> number;
    return !(istr.fail() || istr.bad());
}

//////////////////////////////////////////////////////////////////////////

bool parser::is_option(std::string const& arg) const
{
    assert(0 != arg.size());
    if (is_number(arg))
        return false;
    return '-' == arg[0];
}

//////////////////////////////////////////////////////////////////////////

std::string parser::trim_leading_dashes(std::string const& name) const
{
    auto pos = name.find_first_not_of('-');
    return std::string::npos != pos ? name.substr(pos) : name;
}

//////////////////////////////////////////////////////////////////////////

bool argh::parser::got_flag(std::string const& name) const
{
    return flags_.end() != flags_.find(trim_leading_dashes(name));
}

//////////////////////////////////////////////////////////////////////////

bool argh::parser::is_param(std::string const& name) const
{
    return registeredParams_.count(name);
}

//////////////////////////////////////////////////////////////////////////

bool parser::operator[](std::string const& name) const
{
    return got_flag(name);
}

//////////////////////////////////////////////////////////////////////////

bool parser::operator[](const std::vector<std::string>& init_list) const
{
    return std::any_of(init_list.begin(), init_list.end(), [&](const std::string& name) { return got_flag(name); });
}

//////////////////////////////////////////////////////////////////////////

std::string const& parser::operator[](size_t ind) const
{
    if (ind < pos_args_.size())
        return pos_args_[ind];
    return empty_;
}

//////////////////////////////////////////////////////////////////////////

string_stream parser::operator()(std::string const& name) const
{
    auto optIt = params_.find(trim_leading_dashes(name));
    if (params_.end() != optIt)
        return string_stream(optIt->second);
    return bad_stream();
}

//////////////////////////////////////////////////////////////////////////

string_stream parser::operator()(const std::vector<std::string>& init_list) const
{
    for (auto& name : init_list)
    {
        auto optIt = params_.find(trim_leading_dashes(name));
        if (params_.end() != optIt)
            return string_stream(optIt->second);
    }
    return bad_stream();
}

//////////////////////////////////////////////////////////////////////////

string_stream parser::operator()(size_t ind) const
{
    if (pos_args_.size() <= ind)
        return bad_stream();

    return string_stream(pos_args_[ind]);
}

//////////////////////////////////////////////////////////////////////////

void parser::add_param(std::string const& name)
{
    registeredParams_.insert(trim_leading_dashes(name));
}

//////////////////////////////////////////////////////////////////////////

void parser::add_param(const std::vector<std::string>& init_list)
{
    parser::add_params(init_list);
}

//////////////////////////////////////////////////////////////////////////

void parser::add_params(const std::vector<std::string>& init_list)
{
    for (auto& name : init_list)
        registeredParams_.insert(trim_leading_dashes(name));
}

//////////////////////////////////////////////////////////////////////////

void parser::add_params(const std::string &name)
{
    parser::add_param(name);
}

//////////////////////////////////////////////////////////////////////////


}
