#pragma once

#include <algorithm>
#include <sstream>
#include <limits>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <cassert>

namespace argh
{
   class stringstream_proxy
   {
   public:
      stringstream_proxy() = default;

      // Construct with a value.
      stringstream_proxy(std::string const& value);

      // Copy constructor.
      stringstream_proxy(const stringstream_proxy& other);

      stringstream_proxy& operator=(const stringstream_proxy& other);

      void setstate(std::ios_base::iostate state);

      stringstream_proxy& operator>>(bool& value);
      stringstream_proxy& operator>>(double& value);
      stringstream_proxy& operator>>(char*& value);
      stringstream_proxy& operator>>(std::string& value);
      stringstream_proxy& operator>>(float& value);
      stringstream_proxy& operator>>(char& value);
      stringstream_proxy& operator>>(short& value);
      stringstream_proxy& operator>>(int& value);
      stringstream_proxy& operator>>(long& value);
      stringstream_proxy& operator>>(long long& value);
      stringstream_proxy& operator>>(unsigned char& value);
      stringstream_proxy& operator>>(unsigned short& value);
      stringstream_proxy& operator>>(unsigned int& value);
      stringstream_proxy& operator>>(unsigned long& value);
      stringstream_proxy& operator>>(unsigned long long& value);

      // Get the string value.
      std::string str() const;

      // Check the state of the stream.
      // False when the most recent stream operation failed
      explicit operator bool() const;

      ~stringstream_proxy() = default;

   private:
      std::istringstream stream_;
   };

   using string_stream = stringstream_proxy;

   enum Mode { PREFER_FLAG_FOR_UNREG_OPTION = 1 << 0,
               PREFER_PARAM_FOR_UNREG_OPTION = 1 << 1,
               NO_SPLIT_ON_EQUALSIGN = 1 << 2,
               SINGLE_DASH_IS_MULTIFLAG = 1 << 3,
    };

   class parser
   {
   public:
      parser() = default;

      parser(const std::vector<std::string>& pre_reg_names)
      {  add_params(pre_reg_names); }

      parser(const char* const argv[], int mode = PREFER_FLAG_FOR_UNREG_OPTION)
      {  parse(argv, mode); }

      parser(int argc, const char* const argv[], int mode = PREFER_FLAG_FOR_UNREG_OPTION)
      {  parse(argc, argv, mode); }

      void add_param(std::string const& name);
      void add_params(std::string const& name);

      void add_param(const std::vector<std::string>& init_list);
      void add_params(const std::vector<std::string>& init_list);

      void parse(const char* const argv[], int mode = PREFER_FLAG_FOR_UNREG_OPTION);
      void parse(int argc, const char* const argv[], int mode = PREFER_FLAG_FOR_UNREG_OPTION);

      size_t size()                                    const { return pos_args_.size();   }

      //////////////////////////////////////////////////////////////////////////
      // Accessors

      // flag (boolean) accessors: return true if the flag appeared, otherwise false.
      bool operator[](std::string const& name) const;

      // multiple flag (boolean) accessors: return true if at least one of the flag appeared, otherwise false.
      bool operator[](const std::vector<std::string>& init_list) const;

      // returns positional arg string by order. Like argv[] but without the options
      std::string const& operator[](size_t ind) const;

      // returns a std::istream that can be used to convert a positional arg to a typed value.
      string_stream operator()(size_t ind) const;

      // parameter accessors, give a name get an std::istream that can be used to convert to a typed value.
      // call .str() on result to get as string
      string_stream operator()(std::string const& name) const;

      // accessor for a parameter with multiple names, give a list of names, get an std::istream that can be used to convert to a typed value.
      // call .str() on result to get as string
      // returns the first value in the list to be found.
      string_stream operator()(const std::vector<std::string>& init_list) const;

   private:
      string_stream bad_stream() const;
      std::string trim_leading_dashes(std::string const& name) const;
      bool is_number(std::string const& arg) const;
      bool is_option(std::string const& arg) const;
      bool got_flag(std::string const& name) const;
      bool is_param(std::string const& name) const;

   private:
      std::vector<std::string> args_;
      std::multimap<std::string, std::string> params_;
      std::vector<std::string> pos_args_;
      std::multiset<std::string> flags_;
      std::set<std::string> registeredParams_;
      std::string empty_;
   };

}
