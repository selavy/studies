#include "tarray_util.h"
#include <iostream>
#include <fstream>
#include <string_view>
#include <cassert>
#include <zlib.h>


static bool ends_with(const std::string& s, std::string_view sv)
{
    return s.size() >= sv.size() && s.compare(s.size() - sv.size(), std::string::npos, sv) == 0;
}


static std::vector<char> read_gz_dict_file(const std::string& filename)
{
    gzFile file = gzopen(filename.c_str(), "rb");
    if (!file) {
        gzclose(file);
        throw std::runtime_error("unable to open input file");
    }
    constexpr std::size_t chunk = 1024;
    std::size_t len = 0;
    std::vector<char> buf(chunk, '\0');
    int rc;
    while ((rc = gzread(file, &buf[len], chunk)) > 0) {
        len += static_cast<std::size_t>(rc);
        buf.insert(buf.end(), chunk, '\0');
    }
    if (!gzeof(file)) {
        // TEMP TEMP
        int errnum = 0;
        std::cerr << "error: unable to read GZIP file [" << rc << "]: " << gzerror(file, &errnum) << std::endl;
        gzclose(file);
        throw std::runtime_error("unable to read input file -- not GZIP format?");
    }
    gzclose(file);
    assert(gzeof(file));
    buf.erase(std::next(buf.cbegin(), len), buf.cend());
    return buf;
}


std::vector<char> read_dict_file(const std::string& filename)
{
    if (ends_with(filename, ".gz")) {
        return read_gz_dict_file(filename);
    }
    std::ifstream infile;
    infile.open(filename, std::ios::binary);
    infile.seekg(0, std::ios::end);
    int length = infile.tellg();
    infile.seekg(0, std::ios::beg);
    std::vector<char> data(length);
    infile.read(data.data(), length);
    infile.close();
    return data;
}
