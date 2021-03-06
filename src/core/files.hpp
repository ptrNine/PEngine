#pragma once

#include "types.hpp"
#include "container_extensions.hpp"

namespace core
{
/**
 * @brief Reads file line by line
 *
 * @param path - a path to the file
 *
 * @return lines of the file or nullopt if the file cannot be opened
 */
[[nodiscard]] inline optional<vector<string>> read_lines(const string& path) {
    vector<string> result;

    std::ifstream ifs(path, std::ios::in);

    if (!ifs.is_open())
        return nullopt;

    while (!ifs.eof()) {
        string line;
        std::getline(ifs, line);
        result.emplace_back(std::move(line));
    }

    return result;
}

/**
 * @brief Reads file into string
 *
 * @param file_path - a path to the file
 *
 * @return the file content or nullopt if the file cannot be opened
 */
[[nodiscard]] inline optional<string> try_read_file(const string& file_path) {
    std::ifstream ifs(file_path, std::ios_base::in);

    if (!ifs.is_open())
        return nullopt;

    auto stat = platform_dependent::get_file_stat(file_path);
    if (!stat)
        return nullopt;

    if (stat->size == 0)
        return core::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());

    std::string str;
    str.resize(stat->size);
    auto bs = ifs.readsome(str.data(), static_cast<std::streamsize>(stat->size));
    if (bs < 0)
        return nullopt;
    str.resize(static_cast<size_t>(bs));

    return str;
}

/**
 * @brief Reads file into byte vector
 *
 * @param file_path - a path to the file
 *
 * @return the file content or nullopt if the file cannot be opened
 */
[[nodiscard]] inline optional<vector<byte>> try_read_binary_file(const string& file_path) {
    std::ifstream ifs(file_path, std::ios_base::binary | std::ios_base::in);

    if (!ifs.is_open())
        return nullopt;

    auto stat = platform_dependent::get_file_stat(file_path);
    if (!stat)
        return nullopt;

    if (stat->size == 0) {
        auto res = vector<char>(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
        vector<byte> r(res.size());
        ::memcpy(r.data(), res.data(), r.size());
        return r;
    }

    vector<byte> result(stat->size);
    auto         bs = ifs.readsome(reinterpret_cast<char*>(result.data()), // NOLINT
                           static_cast<std::streamsize>(stat->size));
    if (bs < 0)
        return nullopt;
    result.resize(static_cast<size_t>(bs));

    return result;
}

/**
 * @brief Reads file into string
 *
 * @throw runtime_error if the file cannot be opened
 *
 * @param file_path - a path to the file
 *
 * @return the file content
 */
[[nodiscard]] inline string read_file(const string& file_path) {
    if (auto file = try_read_file(file_path))
        return *file;
    else
        throw std::runtime_error("Can't open file '" + file_path + "'");
}

/**
 * @brief Reads file into byte vector
 *
 * @throw runtime_error if the file cannot be opened
 *
 * @param file_path - a path to the file
 *
 * @return the file content
 */
[[nodiscard]] inline vector<byte> read_binary_file(const string& file_path) {
    if (auto file = try_read_binary_file(file_path))
        return *file;
    else
        throw std::runtime_error("Can't open file '" + file_path + "'");
}

/**
 * @brief Writes a file from string
 *
 * @param file_path - path to new file
 * @param data - the file content
 * @param append - if true then appends data to the end of the file
 *
 * @return true if write successful, false otherwise
 */
[[nodiscard]] inline bool
try_write_file(const string& file_path, string_view data, bool append = false) {
    auto flags = std::ios_base::out;
    if (append)
        flags |= std::ios_base::app;

    std::ofstream ofs(file_path, flags);

    if (!ofs.is_open())
        return false;

    ofs.write(data.data(), static_cast<std::streamsize>(data.size()));

    return !ofs.bad();
}

/**
 * @brief Writes a file from byte span
 *
 * @param file_path - path to new file
 * @param data - the file content
 * @param append - if true then appends data to the end of the file
 *
 * @return true if write successful, false otherwise
 */
[[nodiscard]] inline bool
try_write_file(const string& file_path, span<const byte> data, bool append = false) {
    auto flags = std::ios_base::out;
    if (append)
        flags |= std::ios_base::app;

    std::ofstream ofs(file_path, flags);

    if (!ofs.is_open())
        return false;

    ofs.write(reinterpret_cast<const char*>(data.data()),
              static_cast<std::streamsize>(data.size())); // NOLINT

    return !ofs.bad();
}

/**
 * @brief Writes a file from string
 *
 * @throw runtime_error if write failed
 *
 * @param file_path - path to new file
 * @param data - the file content
 * @param append - if true then appends data to the end of the file
 */
inline void write_file(const string& file_path, string_view data, bool append = false) {
    if (!try_write_file(file_path, data, append))
        throw std::runtime_error("Can't write file '" + file_path + "'");
}

/**
 * @brief Writes a file from byte span
 *
 * @throw runtime_error if write failed
 *
 * @param file_path - path to new file
 * @param data - the file content
 * @param append - if true then appends data to the end of the file
 */
inline void write_file(const string& file_path, span<const byte> data, bool append = false) {
    if (!try_write_file(file_path, data, append))
        throw std::runtime_error("Can't write file '" + file_path + "'");
}

/**
 * @brief Get path for new file with collision resolving
 *
 * If the file already exists, this function append indices in the end of
 * the file name.
 * If the directory not exists then the file path is available and
 * will be returned without any indices
 *
 * @throws runtime_error if the dir is not a directory
 *
 * @param dir - the directory, where file is should be located
 * @param filename - the name of a file
 * @param extension - the extension of the file without a dot prefix
 * @param append_mode - append to file with greater index
 *
 * @return - absolute available path
 */
inline string get_available_file_path_with_index(const string& dir,
                                                 const string& filename,
                                                 const string& extension,
                                                 bool append_mode = false) {
    auto type = get_file_type(dir);

    /* Directory not exists - file path available */
    if (!type)
        return dir / filename + "." + extension;

    if (*type != file_type::directory)
        throw std::runtime_error("Not a directory!");

    size_t index = 0;
    auto full_path = dir / filename + "." + extension;

    optional<string> prev_full_path;
    if (append_mode)
        prev_full_path = full_path;


    while ((type = get_file_type(full_path))) {
        if (append_mode)
            prev_full_path = full_path;

        full_path = dir / filename + std::to_string(index) + "." + extension;
        ++index;
    }

    return append_mode ? *prev_full_path : full_path;
}

/**
 * @brief Cleverly opens files
 *
 * - creates directories recursively for the destination dir
 * - append indices if the file with the same name already exists
 * - guarantees that the file openned successfully if no exception were thrown
 *
 * @throws runtime_error if some component of "dir" is not a directory
 * @throws runtime_error if file can't be openned for an unknown reason
 *
 * @param dir - the directory, where file is should be located
 * @param filename - the name of the file
 * @param extension - the extension of the file without a dot prefix
 * @param mode - open mode (same as for std::ofstream)
 * @param path - file path
 *
 * @return - openned file
 */
inline std::ofstream open_cleverly(const string&           dir,
                                   const string&           filename,
                                   const string&           extension,
                                   std::ios_base::openmode mode = std::ios_base::out,
                                   string*                 file_path = nullptr) {
    auto dir_type = get_file_type(dir);
    if (!dir_type)
        platform_dependent::recursive_create_directory(dir);
    else if (*dir_type != file_type::directory)
        throw std::runtime_error(dir + " not a directory!");

    auto path = get_available_file_path_with_index(
        dir, filename, extension, (mode & std::ios_base::app) ? true : false);
    auto ofs  = std::ofstream(path, mode);

    if (!ofs.is_open())
        throw std::runtime_error("Can't open file " + path);

    if (file_path)
        *file_path = path;

    return ofs;
}

inline std::fstream open_pipe(const string& path, std::ios_base::openmode mode) {
    auto type = get_file_type(path);
    if (type) {
        if (*type != file_type::pipe)
            throw std::runtime_error("File " + path + " already exists and it's not a pipe");
    } else {
        if (!(mode & std::ios_base::out))
            throw std::runtime_error("File " + path + " does not exist");
        else if (!platform_dependent::create_pipe(path))
            throw std::runtime_error("Can't create pipe at " + path);
    }

    auto pipe = std::fstream(path, mode);
    if (!pipe.is_open())
        throw std::runtime_error("Can't open pipe at " + path);

    return pipe;
}

[[nodiscard]]
inline bool has_extension(const string& file_path, string_view extension) {
    return file_path.size() >= extension.size() &&
        case_insensitive_match(string_view(file_path).substr(file_path.size() - extension.size()), extension);
}

} // namespace core
